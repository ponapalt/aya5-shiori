// 
// AYA version 5
//
// �֐��������N���X�@CFunction
// - �又����
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#else
#  include "messages.h"
#  include "posix_utils.h"
#  include <iostream>
#endif
#include "function.h"
#include "sysfunc.h"
#include "value.h"
#include "parser0.h"
#include "parser1.h"
#include "misc.h"
#include "log.h"
#include "wsex.h"

extern vector<CFunction>	function;
extern CCallDepth			calldepth;
extern CSystemFunction		sysfunction;
extern CGlobalVariable		variable;
extern CLog					logger;

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::CompleteSetting
 *  �@�\�T�v�F  �֐��̍\�z�����������i�������̓ǂݍ��݂����������j�ۂɌĂ΂�܂�
 *  �@�@�@�@�@  ���s�̍ۂɕK�v�ȍŌ�̏������������s�Ȃ��܂�
 * -----------------------------------------------------------------------
 */
void	CFunction::CompleteSetting(void)
{
	statelenm1 = statement.size() - 1;

	vector<CStatement>(statement).swap(statement);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::Execute
 *  �@�\�T�v�F  �֐������s���܂�
 *
 *  ����CValue arg�͕K���z��^�ł��Barray����ł���Έ����̖����R�[���ƂȂ�܂�
 * -----------------------------------------------------------------------
 */
CValue	CFunction::Execute(CValue &arg, CLocalVariable &lvar, int &exitcode)
{
	CValue	result;

	exitcode = ST_NOP;

	// _argv���쐬
	lvar.SetValue(L"_argv", arg);
	// _argc���쐬
	CValue	t_argc((int)arg.array.size());
	lvar.SetValue(L"_argc", t_argc);

	// ���s
	if (!calldepth.Add())
		return CValue(F_TAG_NOP, 0/*dmy*/);
	ExecuteInBrace(0, result, lvar, BRACE_DEFAULT, exitcode);
	calldepth.Del();

	return result;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::ExecuteInBrace
 *  �@�\�T�v�F  {}�����s���A���ʂ��ЂƂԂ��܂�
 *  �����@�@�@  type     ����{}�̎�ʁB������0�`�̏ꍇ��switch�\���̍ۂ̌�⒊�o�ʒu
 *  �@�@�@�@�@  exitcode �I���R�[�h�BST_NOP/ST_BREAK/ST_RETURN/ST_CONTINUE=�ʏ�/break/return/continue
 *
 *  "{}"���̊e�X�e�[�g�����g�����s���܂��B����line�Ŏw�肳���ʒu������s���J�n���A"}"�ɓ˂�������܂�
 *  �������s���Ă����܂��B
 *  �Ԓl�͎��s���I������"}"�̈ʒu�ł��B
 * -----------------------------------------------------------------------
 */
int	CFunction::ExecuteInBrace(int line, CValue &result, CLocalVariable &lvar, int type, int &exitcode)
{
	// �J�n���̏���
	lvar.AddDepth();

	// ���s
	CSelecter	output((lvar.GetDepth() == 1) ? &dupl : NULL, type);
	char		exec_end     = 0;	// ����{}�̎��s���I�����邽�߂̃t���O 1�ŏI��
	char		ifflg        = 0;	// if-elseif-else����p�B1�ł��̃u���b�N�������������Ƃ�����
	int			foreachcount;		// foreach����p

	CValue		t_value;

	int	t_statelenm1 = statelenm1;
	int i;
	for(i = line; i < t_statelenm1; i++) {
		switch(statement[i].type) {
		case ST_OPEN:					// "{"
			i = ExecuteInBrace(i + 1, t_value, lvar, BRACE_DEFAULT, exitcode);
			output.Append(t_value);
			break;
		case ST_CLOSE:					// "}"�@���@�֐��I�[��"}"�͂�����ʂ�Ȃ�
			exec_end = 1;
			break;
		case ST_COMBINE:				// "--"
			output.AddArea();
			break;
		case ST_FORMULA_OUT_FORMULA:	// �o�́i�����B�z��A�������֐����܂܂��j
			{
		        CValue val = GetFormulaAnswer(lvar, statement[i]);
				output.Append(val);
			}
			break;
		case ST_FORMULA_SUBST:			// ���
			GetFormulaAnswer(lvar, statement[i]);
			break;
		case ST_IF:						// if
			ifflg = 0;
			if (GetFormulaAnswer(lvar, statement[i]).GetTruth()) {
				i = ExecuteInBrace(i + 2, t_value, lvar, BRACE_DEFAULT, exitcode);
				output.Append(t_value);
				ifflg = 1;
			}
			else
				i = statement[i].jumpto;
			break;
		case ST_ELSEIF:					// elseif
			if (ifflg)
				i = statement[i].jumpto;
			else if (GetFormulaAnswer(lvar, statement[i]).GetTruth()) {
				i = ExecuteInBrace(i + 2, t_value, lvar, BRACE_DEFAULT, exitcode);
				output.Append(t_value);
				ifflg = 1;
			}
			else
				i = statement[i].jumpto;
			break;
		case ST_ELSE:					// else
			if (ifflg)
				i = statement[i].jumpto;
			else {
				i = ExecuteInBrace(i + 2, t_value, lvar, BRACE_DEFAULT, exitcode);
				output.Append(t_value);
			}
			break;
		case ST_PARALLEL:				// parallel
			{
		        CValue val = GetFormulaAnswer(lvar, statement[i]);
				if (val.GetType() == F_TAG_ARRAY) {
					for(int j = 0; j < val.array.size(); j++)
						output.Append(CValue(val.array[j]));
				}
				else
					output.Append(val);
			}
			break;
		case ST_WHILE:					// while
			for( ; ; ) {
				if (!GetFormulaAnswer(lvar, statement[i]).GetTruth())
					break;
				ExecuteInBrace(i + 2, t_value, lvar, BRACE_LOOP, exitcode);
				output.Append(t_value);

				if (exitcode == ST_BREAK) {
					exitcode = ST_NOP;
					break;
				}
				else if (exitcode == ST_RETURN)
					break;
				else if (exitcode == ST_CONTINUE)
					exitcode = ST_NOP;
			}
			i = statement[i].jumpto;
			break;
		case ST_FOR:					// for
			GetFormulaAnswer(lvar, statement[i]);
			for( ; ; ) {
				if (!GetFormulaAnswer(lvar, statement[i + 1]).GetTruth())
					break;
				ExecuteInBrace(i + 4, t_value, lvar, BRACE_LOOP, exitcode);
				output.Append(t_value);

				if (exitcode == ST_BREAK) {
					exitcode = ST_NOP;
					break;
				}
				else if (exitcode == ST_RETURN)
					break;
				else if (exitcode == ST_CONTINUE)
					exitcode = ST_NOP;

				GetFormulaAnswer(lvar, statement[i + 2]);
			}
			i = statement[i].jumpto;
			break;
		case ST_SWITCH: {				// switch
				int	sw_index = GetFormulaAnswer(lvar, statement[i]).GetValueInt();
				if (sw_index < 0)
					sw_index = BRACE_SWITCH_OUT_OF_RANGE;
				i = ExecuteInBrace(i + 2, t_value, lvar, sw_index, exitcode);
				output.Append(t_value);
			}
			break;
		case ST_FOREACH:				// foreach
			for(foreachcount = 0; ; ) {
				if (!Foreach(statement[i], statement[i + 1], lvar, foreachcount))
					break;
				ExecuteInBrace(i + 3, t_value, lvar, BRACE_LOOP, exitcode);
				output.Append(t_value);

				if (exitcode == ST_BREAK) {
					exitcode = ST_NOP;
					break;
				}
				else if (exitcode == ST_RETURN)
					break;
				else if (exitcode == ST_CONTINUE)
					exitcode = ST_NOP;
			}
			i  = statement[i].jumpto;
			break;
		case ST_BREAK:					// break
			exitcode = ST_BREAK;
			break;
		case ST_CONTINUE:				// continue
			exitcode = ST_CONTINUE;
			break;
		case ST_RETURN:					// return
			exitcode = ST_RETURN;
			break;
		default:						// ���m�̃X�e�[�g�����g
			logger.Error(E_E, 82, dicfilename, statement[i].linecount);
			break;
		};
		if (exec_end)
			break;

		if (exitcode != ST_NOP)
			FeedLineToTail(i);
	}

	// �I�����̏���
	lvar.DelDepth();

	// ��₩��o�͂�I�яo���@����q�̐[����0�Ȃ�d�����������
	result = output.Output();

	return i;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::Foreach
 *  �@�\�T�v�F  foreach�������s���܂�
 *  �Ԓl�@�@�F  0/1=���[�v�E�o/���[�v���s
 *
 *  ���ۂɑ���̂�"}"��1��O�̍s�̈ʒu�ł�
 * -----------------------------------------------------------------------
 */
char	CFunction::Foreach(CStatement &st0, CStatement &st1, CLocalVariable &lvar, int &foreachcount)
{
	// ����l�����߂�
	CValue	value = GetFormulaAnswer(lvar, st0);
	if (value.GetType() == F_TAG_NOP)
		value = L"";

	// ����l�̗v�f�������߂�
	// �ȈՔz�񂩂ϐ�����̎擾�̏ꍇ�A���̕ϐ��ɐݒ肳��Ă���f���~�^���擾����
	int	sz;
	vector<wstring>	s_array;
	if (value.GetType() == F_TAG_STRING) {
		wstring delimiter = VAR_DELIMITER;
		if (st0.cell.size() == 1) {
			if (st0.cell[0].v.GetType() == F_TAG_VARIABLE) {
				delimiter = variable.GetDelimiter(st0.cell[0].index);
			}
			else if (st0.cell[0].v.GetType() == F_TAG_LOCALVARIABLE)
				delimiter = lvar.GetDelimiter(st0.cell[0].name);
		}
		else {
			CCell *l_cell = &(st0.cell[st0.serial[st0.serial.size() - 1].tindex]);
			if (l_cell->v.GetType() == F_TAG_VARIABLE)
				delimiter = variable.GetDelimiter(l_cell->index);
			else if (l_cell->v.GetType() == F_TAG_LOCALVARIABLE)
				delimiter = lvar.GetDelimiter(l_cell->name);
			else if (l_cell->v.GetType() == F_TAG_ARRAYORDER) {
				l_cell = &(st0.cell[st0.serial[st0.serial.size() - 1].tindex - 1]);
				if (l_cell->v.GetType() == F_TAG_VARIABLE)
					delimiter = variable.GetDelimiter(l_cell->index);
				else if (l_cell->v.GetType() == F_TAG_LOCALVARIABLE)
					delimiter = lvar.GetDelimiter(l_cell->name);
			}
		}
		sz = SplitToMultiString(value.s_value, s_array, delimiter);
	}
	else if (value.GetType() == F_TAG_ARRAY)
		sz = value.array.size();
	else
		sz = -1;

	// �������v�f���𒴂��Ă����烋�[�v�E�o
	if (foreachcount >= sz)
		return 0;

	// �������v�f�l���擾
	CValue	t_value;
	if (value.GetType() == F_TAG_STRING)
		t_value = s_array[foreachcount];
	else	// F_TAG_ARRAY
		t_value = value.array[foreachcount];
	// ���
	switch(st1.cell[0].v.GetType()) {
	case F_TAG_VARIABLE:
		variable.SetValue(st1.cell[0].index, t_value);
		break;
	case F_TAG_LOCALVARIABLE:
		lvar.SetValue(st1.cell[0].name, t_value);
		break;
	default:
		logger.Error(E_E, 28, dicfilename, st1.linecount);
		return 0;
	};

	// ������i�߂�
	foreachcount++;

	return 1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::GetFormulaAnswer
 *  �@�\�T�v�F  ���������Z���Č��ʂ�Ԃ��܂�
 * -----------------------------------------------------------------------
 */
CValue	CFunction::GetFormulaAnswer(CLocalVariable &lvar, CStatement &st)
{
	int		o_index;

	for(vector<CSerial>::iterator it = st.serial.begin(); it != st.serial.end(); it++) {
		o_index = it->tindex;
		CCell	&o_cell = st.cell[o_index];
		if (o_cell.v.GetType() >= F_TAG_ORIGIN_VALUE) {
			o_cell.ansv = *GetValuePtrForCalc(o_cell, st, lvar);
			break;
		}

		CCell	*s_cell = &(st.cell[it->index[0]]);
		CCell	*d_cell = &(st.cell[it->index[1]]);
		switch(o_cell.v.GetType()) {
		case F_TAG_CUMMA:
			if (Cumma(o_cell.ansv, it->index, st, lvar))
				logger.Error(E_E, 33, L",", dicfilename, st.linecount);
			break;
		case F_TAG_EQUAL:
		case F_TAG_EQUAL_D:
		case F_TAG_PLUSEQUAL:
		case F_TAG_PLUSEQUAL_D:
		case F_TAG_MINUSEQUAL:
		case F_TAG_MINUSEQUAL_D:
		case F_TAG_MULEQUAL:
		case F_TAG_MULEQUAL_D:
		case F_TAG_DIVEQUAL:
		case F_TAG_DIVEQUAL_D:
		case F_TAG_SURPEQUAL:
		case F_TAG_SURPEQUAL_D:
		case F_TAG_CUMMAEQUAL:
			if (Subst(o_cell.v.GetType(), o_cell.ansv, it->index, st, lvar))
				logger.Error(E_E, 33, L"=", dicfilename, st.linecount);
			break;
		case F_TAG_PLUS:
			o_cell.ansv = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) +
				*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
			break;
		case F_TAG_MINUS:
			o_cell.ansv = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) -
				*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
			break;
		case F_TAG_MUL:
			o_cell.ansv = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) *
				*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
			break;
		case F_TAG_DIV:
			o_cell.ansv = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) /
				*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
			break;
		case F_TAG_SURP:
			o_cell.ansv = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) %
				*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
			break;
		case F_TAG_IFEQUAL:
			o_cell.ansv = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) ==
				*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
			break;
		case F_TAG_IFDIFFER:
			o_cell.ansv = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) !=
				*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
			break;
		case F_TAG_IFGTEQUAL:
			o_cell.ansv = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) >=
				*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
			break;
		case F_TAG_IFLTEQUAL:
			o_cell.ansv = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) <=
				*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
			break;
		case F_TAG_IFGT:
			o_cell.ansv = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) >
				*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
			break;
		case F_TAG_IFLT:
			o_cell.ansv = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) <
				*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
			break;
		case F_TAG_IFIN:
			o_cell.ansv = _in_(*(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()),
				*(GetValuePtrForCalc(*d_cell, st, lvar))->SafeThis());
			break;
		case F_TAG_IFNOTIN:
			o_cell.ansv = not_in_(*(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()),
				*(GetValuePtrForCalc(*d_cell, st, lvar))->SafeThis());
			break;
		case F_TAG_OR:
			o_cell.ansv = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) ||
				*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
			break;
		case F_TAG_AND:
			o_cell.ansv = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) &&
				*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
			break;
		case F_TAG_FUNCPARAM:
			if (ExecFunctionWithArgs(o_cell.ansv, it->index, st, lvar))
				logger.Error(E_E, 33, function[st.cell[it->index[0]].index].name, dicfilename, st.linecount);
			break;
		case F_TAG_SYSFUNCPARAM:
			if (ExecSystemFunctionWithArgs(o_cell, it->index, st, lvar))
				logger.Error(E_E, 33, (wchar_t *)sysfunc[st.cell[it->index[0]].index], dicfilename, st.linecount);
			break;
		case F_TAG_ARRAYORDER:
			if (Array(o_cell, it->index, st, lvar))
				logger.Error(E_E, 33, L",", dicfilename, st.linecount);
			break;
		case F_TAG_FEEDBACK:
			if (Feedback(o_cell, it->index, st, lvar))
				logger.Error(E_E, 33, L"&", dicfilename, st.linecount);
			break;
		case F_TAG_EXC:
			o_cell.ansv = Exc(*GetValuePtrForCalc(*d_cell, st, lvar));
			break;
		default:
			logger.Error(E_E, 34, dicfilename, st.linecount);
			return CValue(F_TAG_NOP, 0/*dmy*/);
		};
	}

	return st.cell[o_index].ansv;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::GetValuePtrForCalc
 *  �@�\�T�v�F  �^����ꂽ���ɑΉ�����l�ւ̃|�C���^���擾���܂�
 * -----------------------------------------------------------------------
 */
CValue	*CFunction::GetValuePtrForCalc(CCell &cell, CStatement &st, CLocalVariable &lvar)
{
	// ���l��v�A�֐�/�ϐ�/���Z�q���Ȃ�ansv����擾�@�֐�/�ϐ��̏ꍇ���̒l����s���ʂ��擾�����

	// %[n]����
	if (cell.v.GetType() == F_TAG_SYSFUNCPARAM)
		ExecHistoryP2(cell, st);

	// ���Z���������Ă���i�͂��́j���Ȃ炻���Ԃ�
	if (cell.v.GetType() < F_TAG_ORIGIN_VALUE)
		return &(cell.ansv);

	// ���l�Ȃ炻������̂܂ܕԂ�
	if (cell.v.GetType() <= F_TAG_STRING)
		return &(cell.v);

	// �֐��Ȃ���s���Č��ʂ��A�ϐ��Ȃ炻�̓��e��Ԃ�
	switch(cell.v.GetType()) {
	case F_TAG_STRING_EMBED:
		SolveEmbedCell(cell, st, lvar);
		return &(cell.ansv);
	case F_TAG_SYSFUNC: {
			CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
			vector<CCell *>	pcellarg;
			cell.ansv =  sysfunction.Execute(cell.index, arg, pcellarg, lvar, st.linecount, this);
			return &(cell.ansv);
		}
	case F_TAG_USERFUNC: {
		CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
		CLocalVariable	t_lvar;
		int	exitcode;
		cell.ansv = function[cell.index].Execute(arg, t_lvar, exitcode);
		return &(cell.ansv);
	}
	case F_TAG_VARIABLE:
		cell.ansv = variable.GetValue(cell.index);
		return &(cell.ansv);
	case F_TAG_LOCALVARIABLE:
		cell.ansv = lvar.GetValue(cell.name);
		return &(cell.ansv);
	default:
		logger.Error(E_E, 16, dicfilename, st.linecount);
		cell.ansv = L"";
		return &(cell.ansv);
	};
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::SolveEmbedCell
 *  �@�\�T�v�F  %���ߍ��ݕt�����񍀂̒l�����߂܂�
 * -----------------------------------------------------------------------
 */
void	CFunction::SolveEmbedCell(CCell &cell, CStatement &st, CLocalVariable &lvar)
{
	// ��������ʒu�����߂�
	int	solve_src;	// ��� 0/1/2/3=���[�J���ϐ�/�ϐ�/�֐�/�V�X�e���֐�
	int	max_len = 0;	// �Œ���v�����p

	if (cell.v.s_value[0] == L'_') {
		// ���[�J���ϐ�
		solve_src = 0;
		max_len   = lvar.GetMacthedLongestNameLength(cell.v.s_value);
	}
	else {
		// �ϐ�
		solve_src = 1;
		max_len   = variable.GetMacthedLongestNameLength(cell.v.s_value);
		// �֐�
		int	t_len = 0;
		for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
			if (!it->name.compare(cell.v.s_value.substr(0, it->namelen)))
				if (t_len < it->namelen)
					t_len = it->namelen;
		if (t_len > max_len) {
			solve_src = 2;
			max_len   = t_len;
		}
		// �V�X�e���֐�
		t_len = 0;
		for(int i = 0; i < SYSFUNC_NUM; i++) {
			int	sysfunc_len = sysfunction.GetNameLen(i);
			if (!::wcscmp(cell.v.s_value.substr(0, sysfunc_len).c_str(), sysfunc[i]))
				if (t_len < sysfunc_len)
					t_len = sysfunc_len;
		}
		if (t_len > max_len) {
			solve_src = 3;
			max_len   = t_len;
		}
	}
	// ���݂��Ȃ���ΑS�̂�������Ƃ������ƂɂȂ�
	if (!max_len) {
		cell.ansv     = L"%" + cell.v.s_value;
		cell.emb_ansv = L"%" + cell.v.s_value;
		return;
	}

	// �֐�/�V�X�e���֐��̏ꍇ�͈���������T��
	int	len = cell.v.s_value.size();
	if (solve_src >= 2) {
		int	depth = 1;
		int i;
		for(i = max_len + 1; i < len; i++) {
			depth += ((cell.v.s_value[i] == L'(') - (cell.v.s_value[i] == L')'));
			if (!depth)
				break;
		}
		if (!depth)
			max_len = i + 1;
	}

	// �z�񕔕���T��
	int	depth = 1;
	int i;
	for(i = max_len + 1; i < len; i++) {
		if (!depth && cell.v.s_value[i] != L'[')
			break;
		depth += ((cell.v.s_value[i] == L'[') - (cell.v.s_value[i] == L']'));
	}
	if (!depth)
		max_len = i;

	// ���ߍ��܂ꂽ�v�f�Ƃ���ȍ~�̕�����ɕ�������
	wstring	s_value(cell.v.s_value.substr(0, max_len));
	wstring	d_value(cell.v.s_value.substr(max_len, len - max_len));
	// ���ߍ��܂ꂽ�v�f�𐔎��ɕϊ�����@���s�Ȃ�S�̂�������
	CStatement	t_state(ST_FORMULA, st.linecount);
	if (CParser0::ParseEmbedString(s_value, t_state, dicfilename, st.linecount)) {
		cell.ansv     = L"%" + cell.v.s_value;
		cell.emb_ansv = L"%" + cell.v.s_value;
		return;
	}

	// ���ߍ��ݗv�f�̒l���擾���ĉ�����������쐬
	wstring	result = GetFormulaAnswer(lvar, t_state).GetValueString();
	cell.emb_ansv  = result;
	cell.ansv      = result + d_value;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::Cumma
 *  �@�\�T�v�F  ,���Z�q���������܂�
 *
 *  �Ԓl�@�@�F  0/1=����/�G���[
 * -----------------------------------------------------------------------
 */
char	CFunction::Cumma(CValue &answer, vector<int> &sid, CStatement &st, CLocalVariable &lvar)
{
	// �������Ĕz��l���쐬
	vector<CValueSub>	t_array;
	for(vector<int>::iterator it = sid.begin(); it != sid.end(); it++) {
		CValue	*addv = GetValuePtrForCalc(st.cell[*it], st, lvar)->SafeThis();
		if (addv->GetType() == F_TAG_INT)
			t_array.push_back(CValueSub(addv->i_value));
		else if (addv->GetType() == F_TAG_DOUBLE)
			t_array.push_back(CValueSub(addv->d_value));
		else if (addv->GetType() == F_TAG_STRING)
			t_array.push_back(CValueSub(addv->s_value));
		else if (addv->GetType() == F_TAG_ARRAY)
			t_array.insert(t_array.end(), addv->array.begin(), addv->array.end());
		else
			return 1;
	}

	answer = t_array;
	return 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::Subst
 *  �@�\�T�v�F  ������Z�q���������܂�
 *
 *  �Ԓl�@�@�F  0/1=����/�G���[
 * -----------------------------------------------------------------------
 */
char	CFunction::Subst(int type, CValue &answer, vector<int> &sid, CStatement &st, CLocalVariable &lvar)
{
	CCell	*s_cell = &(st.cell[sid[0]]);
	CCell	*d_cell = &(st.cell[sid[1]]);

	// ������̒l���擾�@���Z�q���Ȃ牉�Z���s��
	switch(type) {
	case F_TAG_EQUAL:
	case F_TAG_EQUAL_D:
		answer = *(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
		break;
	case F_TAG_PLUSEQUAL:
	case F_TAG_PLUSEQUAL_D:
		answer = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) +
			*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
		break;
	case F_TAG_MINUSEQUAL:
	case F_TAG_MINUSEQUAL_D:
		answer = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) -
			*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
		break;
	case F_TAG_MULEQUAL:
	case F_TAG_MULEQUAL_D:
		answer = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) *
			*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
		break;
	case F_TAG_DIVEQUAL:
	case F_TAG_DIVEQUAL_D:
		answer = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) /
			*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
		break;
	case F_TAG_SURPEQUAL:
	case F_TAG_SURPEQUAL_D:
		answer = *(GetValuePtrForCalc(*s_cell, st, lvar)->SafeThis()) %
			*(GetValuePtrForCalc(*d_cell, st, lvar)->SafeThis());
		break;
	case F_TAG_CUMMAEQUAL:
		if (Cumma(answer, sid, st, lvar))
			return 1;
		break;
	default:
		return 1;
	};

	// ��������s
	// �z��v�f�ւ̑���͑��삪���G�Ȃ̂ŁA����ɑ��̊֐��֏�����n��
	switch(s_cell->v.GetType()) {
	case F_TAG_VARIABLE:
		variable.SetValue(s_cell->index, answer);
		return 0;
	case F_TAG_LOCALVARIABLE:
		lvar.SetValue(s_cell->name, answer);
		return 0;
	case F_TAG_ARRAYORDER: {
			if (sid[0] > 0)
				return SubstToArray(st.cell[sid[0] - 1], *s_cell, answer, st, lvar);
			else
				return 1;
		}
	default:
		return 1;
	};
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::SubstToArray
 *  �@�\�T�v�F  �z��v�f�ւ̑�����������܂�
 *
 *  �Ԓl�@�@�F  0/1=����/�G���[
 * -----------------------------------------------------------------------
 */
char	CFunction::SubstToArray(CCell &vcell, CCell &ocell, CValue &answer, CStatement &st, CLocalVariable &lvar)
{
	// �������擾
	CValue	t_order = EncodeArrayOrder(vcell, ocell.order, lvar);
	if (t_order.GetType() == F_TAG_UNKNOWN)
		return 1;

	// �l���擾
	CValue	value = *(GetValuePtrForCalc(vcell, st, lvar)->SafeThis());

	// �X�V
	value.SetArrayValue(t_order, answer);

	// ���
	switch(vcell.v.GetType()) {
	case F_TAG_VARIABLE:
		variable.SetValue(vcell.index, value);
		return 0;
	case F_TAG_LOCALVARIABLE:
		lvar.SetValue(vcell.name, value);
		return 0;
	default:
		return 1;
	};
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::Array
 *  �@�\�T�v�F  �z��[]���Z�q���������܂�
 *
 *  �Ԓl�@�@�F  0/1=����/�G���[
 * -----------------------------------------------------------------------
 */
char	CFunction::Array(CCell &anscell, vector<int> &sid, CStatement &st, CLocalVariable &lvar)
{
	CCell	*v_cell = &(st.cell[sid[0]]);
	CCell	*n_cell = &(st.cell[sid[1]]);

	// �������擾
	anscell.order = *(GetValuePtrForCalc(*n_cell, st, lvar)->SafeThis());

	CValue	t_order = EncodeArrayOrder(*v_cell, anscell.order, lvar);
	if (t_order.GetType() == F_TAG_UNKNOWN) {
		anscell.ansv = L"";
		return 1;
	}

	// �l���擾
	anscell.ansv = (*(GetValuePtrForCalc(*v_cell, st, lvar)->SafeThis()))[t_order];

	return 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::_in_
 *  �@�\�T�v�F  _in_���Z�q���������܂�
 * -----------------------------------------------------------------------
 */
CValue	CFunction::_in_(CValue &src, CValue &dst)
{
	if (src.GetType() == F_TAG_STRING &&
		dst.GetType() == F_TAG_STRING)
		return CValue((dst.s_value.find(src.s_value) != -1) ? 1 : 0);

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::not_in_
 *  �@�\�T�v�F  !_in_���Z�q���������܂�
 * -----------------------------------------------------------------------
 */
CValue	CFunction::not_in_(CValue &src, CValue &dst)
{
	if (src.GetType() == F_TAG_STRING &&
		dst.GetType() == F_TAG_STRING)
		return CValue((dst.s_value.find(src.s_value) == -1) ? 1 : 0);

	return CValue(1);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::ExecFunctionWithArgs
 *  �@�\�T�v�F  �����t���̊֐������s���܂�
 *
 *  �Ԓl�@�@�F  0/1=����/�G���[
 * -----------------------------------------------------------------------
 */
char	CFunction::ExecFunctionWithArgs(CValue &answer, vector<int> &sid, CStatement &st, CLocalVariable &lvar)
{
	// �֐��̊i�[�ʒu���擾
	vector<int>::iterator it = sid.begin();
	int	index = st.cell[*it].index;
	it++;

	// �����쐬
	CValue	arg(F_TAG_ARRAY, 0/*dmy*/);	
	for( ; it != sid.end(); it++) {
		CValue	*addv = GetValuePtrForCalc(st.cell[*it], st, lvar)->SafeThis();
		if (addv->GetType() == F_TAG_INT)
			arg.array.push_back(CValueSub(addv->i_value));
		else if (addv->GetType() == F_TAG_DOUBLE)
			arg.array.push_back(CValueSub(addv->d_value));
		else if (addv->GetType() == F_TAG_STRING)
			arg.array.push_back(CValueSub(addv->s_value));
		else if (addv->GetType() == F_TAG_ARRAY)
			arg.array.insert(arg.array.end(), addv->array.begin(), addv->array.end());
		else
			return 1;
	}

	// ���s
	CLocalVariable	t_lvar;
	int	exitcode;
	answer = function[index].Execute(arg, t_lvar, exitcode);

	// �t�B�[�h�o�b�N
	CValue	*v_argv = &(t_lvar.GetArgvPtr()->v);
	int	i = 0;
	int	errcount = 0;
	for(it = sid.begin() + 1; it != sid.end(); it++, i++)
		if (st.cell[*it].v.GetType() == F_TAG_FEEDBACK) {
			CValue	v_value;
			v_value = v_argv->array[i];

			if (st.cell[*it].order.GetType() != F_TAG_NOP)
				errcount += SubstToArray(st.cell[(*it) + 1], st.cell[*it], v_value, st, lvar);
			else {
				switch(st.cell[(*it) + 1].v.GetType()) {
				case F_TAG_VARIABLE:
					variable.SetValue(st.cell[(*it) + 1].index, v_value);
					break;
				case F_TAG_LOCALVARIABLE:
					lvar.SetValue(st.cell[(*it) + 1].name, v_value);
					break;
				default:
					break;
				};
			}
		}

	return errcount;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::ExecSystemFunctionWithArgs
 *  �@�\�T�v�F  �����t���̃V�X�e���֐������s���܂�
 *
 *  �Ԓl�@�@�F  0/1=����/�G���[
 * -----------------------------------------------------------------------
 */
char	CFunction::ExecSystemFunctionWithArgs(CCell& cell, vector<int> &sid, CStatement &st, CLocalVariable &lvar)
{
	// �֐��̊i�[�ʒu���擾
	vector<int>::iterator it = sid.begin();
	int	func_index = *it;
	int	index = st.cell[func_index].index;
	it++;

	// �����쐬
	CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
	vector<CCell *>	pcellarg;
	for( ; it != sid.end(); it++) {
		CValue	*addv = GetValuePtrForCalc(st.cell[*it], st, lvar)->SafeThis();
		if (addv->GetType() == F_TAG_INT)
			arg.array.push_back(CValueSub(addv->i_value));
		else if (addv->GetType() == F_TAG_DOUBLE)
			arg.array.push_back(CValueSub(addv->d_value));
		else if (addv->GetType() == F_TAG_STRING)
			arg.array.push_back(CValueSub(addv->s_value));
		else if (addv->GetType() == F_TAG_ARRAY)
			arg.array.insert(arg.array.end(), addv->array.begin(), addv->array.end());
		else
			return 1;

		pcellarg.push_back(&(st.cell[*it]));
	}

	// ���s�@%[n]�����֐��̂ݓ��ስ��
	if (index == SYSFUNC_HIS)
		ExecHistoryP1(func_index - 2, cell, arg, st);
	else
		cell.ansv = sysfunction.Execute(index, arg, pcellarg, lvar, st.linecount, this);

	return 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::ExecHistoryP1
 *  �@�\�T�v�F  %[n]�i�u���ς̒l�̍ė��p�j���������܂��i�O�����j
 *
 *  �����͓�i�K�ōs���܂��B�O�����ł͖{�����̂��߂̒l�����Z�q�̍��փZ�b�g���܂��B
 * -----------------------------------------------------------------------
 */
void	CFunction::ExecHistoryP1(int start_index, CCell& cell, CValue &arg, CStatement &st)
{
	if (arg.array.size()) {
		cell.ansv  = start_index;
		cell.order = arg.array[0];
	}
	else {
		logger.Error(E_E, 90, dicfilename, st.linecount);
		cell.ansv  = L"";
		cell.order = L"";
	}
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::ExecHistoryP2
 *  �@�\�T�v�F  %[n]�i�u���ς̒l�̍ė��p�j���������܂��i�{�����j
 *
 *  �����͓�i�K�ōs���܂��B�{�����ł͑O�����Ŗ��ߍ��񂾒l���Q�Ƃ��Ēl���擾���܂��B
 * -----------------------------------------------------------------------
 */
void	CFunction::ExecHistoryP2(CCell& cell, CStatement &st)
{
	if (cell.order.GetType() != F_TAG_INT &&
		cell.order.GetType() != F_TAG_DOUBLE)
		return;

	cell.ansv = L"";

	int	index = cell.order.GetValueInt();
	if (index < 0)
		return;

	for(int i = cell.ansv.i_value; i >= 0; i--)
		if (st.cell[i].v.GetType() == F_TAG_STRING_EMBED) {
			if (!index) {
				cell.ansv = st.cell[i].emb_ansv;
				return;
			}
			index--;
		}
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::Feedback
 *  �@�\�T�v�F  &���Z�q���������܂�
 *
 *  �Ԓl�@�@�F  0/1=����/�G���[
 * -----------------------------------------------------------------------
 */
char	CFunction::Feedback(CCell &anscell, vector<int> &sid, CStatement &st, CLocalVariable &lvar)
{
	CCell	*v_cell = &(st.cell[sid[1]]);

	// �l�͉E�ӂ����̂܂܃R�s�[
	anscell.ansv = *(GetValuePtrForCalc(*v_cell, st, lvar)->SafeThis());

	// �E�ӂ��z�񏘐����w�肷�鉉�Z�q�������ꍇ�͂������珘�����R�s�[
	// �z��łȂ������ꍇ�͏������i�[����ϐ��̌^��NOP�ɂ��ăt���O�Ƃ���
	if (v_cell->v.GetType() == F_TAG_ARRAYORDER)
		anscell.order = v_cell->order;
	else
		anscell.order.SetType(F_TAG_NOP);

	return 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::Exc
 *  �@�\�T�v�F  !���Z�q���������܂�
 * -----------------------------------------------------------------------
 */
CValue	CFunction::Exc(CValue &value)
{
	return CValue(1 - value.GetTruth());
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::EncodeArrayOrder
 *  �@�\�T�v�F  �z��̏������쐬���ĕԂ��܂�
 *
 *  CValue operator [] �͈����Ƃ��ėv�f��2�ȏ�̔z��^��CValue��v�����܂��B
 *  �i�������������A���������f���~�^�j
 *  ���̊֐��͂�����쐬���܂��B
 *
 *  �G���[�����������ꍇ�͌^�̂Ȃ��iF_TAG_UNKNOWN�j�l��Ԃ��܂��B�i�Ăяo�����͂�������ăG���[�������܂��j
 * -----------------------------------------------------------------------
 */
CValue	CFunction::EncodeArrayOrder(CCell &vcell, CValue &order, CLocalVariable &lvar)
{
	CValue	result(F_TAG_ARRAY, 0/*dmy*/);

	// ����
	switch(order.GetType()) {
	case F_TAG_ARRAY:
		result = order;
		break;
	case F_TAG_INT: {
			CValueSub	addo(order.i_value);
			result.array.push_back(addo);
		}
		break;
	case F_TAG_DOUBLE: {
			CValueSub	addo(order.d_value);
			result.array.push_back(addo);
		}
		break;
	case F_TAG_STRING: {
			CValueSub	addo(order.s_value);
			result.array.push_back(addo);
		}
		break;
	default:
		result.SetType(F_TAG_UNKNOWN);
		return result;
	};

	// �f���~�^
	if (result.array.size() < 2) {
		CValueSub	adddlm(VAR_DELIMITER);
		if (vcell.v.GetType() == F_TAG_VARIABLE)
			adddlm = variable.GetDelimiter(vcell.index);
		else if (vcell.v.GetType() == F_TAG_LOCALVARIABLE)
			adddlm = lvar.GetDelimiter(vcell.name);
		result.array.push_back(adddlm);
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFunction::FeedLineToTail
 *  �@�\�T�v�F  ���݂�{}�̏I�["}"�܂Ŏ��s�s�J�E���^�𑗂�܂�
 *
 *  ���ۂɑ���̂�"}"��1��O�̍s�̈ʒu�ł�
 * -----------------------------------------------------------------------
 */
void	CFunction::FeedLineToTail(int &line)
{
	int	t_statelenm1 = statelenm1;

	int	depth = 1;
	line++;
	for( ; line < t_statelenm1; line++) {
		depth += ((statement[line].type == ST_OPEN) - (statement[line].type == ST_CLOSE));
		if (!depth)
			break;
	}

	line--;
}

