// 
// AYA version 5
//
// 関数を扱うクラス　CFunction
// - 主処理部
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
 *  関数名  ：  CFunction::CompleteSetting
 *  機能概要：  関数の構築が完了した（≒辞書の読み込みが完了した）際に呼ばれます
 *  　　　　　  実行の際に必要な最後の初期化処理を行ないます
 * -----------------------------------------------------------------------
 */
void	CFunction::CompleteSetting(void)
{
	statelenm1 = statement.size() - 1;

	vector<CStatement>(statement).swap(statement);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::Execute
 *  機能概要：  関数を実行します
 *
 *  引数CValue argは必ず配列型です。arrayが空であれば引数の無いコールとなります
 * -----------------------------------------------------------------------
 */
CValue	CFunction::Execute(CValue &arg, CLocalVariable &lvar, int &exitcode)
{
	CValue	result;

	exitcode = ST_NOP;

	// _argvを作成
	lvar.SetValue(L"_argv", arg);
	// _argcを作成
	CValue	t_argc((int)arg.array.size());
	lvar.SetValue(L"_argc", t_argc);

	// 実行
	if (!calldepth.Add())
		return CValue(F_TAG_NOP, 0/*dmy*/);
	ExecuteInBrace(0, result, lvar, BRACE_DEFAULT, exitcode);
	calldepth.Del();

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::ExecuteInBrace
 *  機能概要：  {}を実行し、結果をひとつ返します
 *  引数　　　  type     この{}の種別。ただし0～の場合はswitch構文の際の候補抽出位置
 *  　　　　　  exitcode 終了コード。ST_NOP/ST_BREAK/ST_RETURN/ST_CONTINUE=通常/break/return/continue
 *
 *  "{}"内の各ステートメントを実行します。引数lineで指定される位置から実行を開始し、"}"に突き当たるまで
 *  順次実行していきます。
 *  返値は実行を終了した"}"の位置です。
 * -----------------------------------------------------------------------
 */
int	CFunction::ExecuteInBrace(int line, CValue &result, CLocalVariable &lvar, int type, int &exitcode)
{
	// 開始時の処理
	lvar.AddDepth();

	// 実行
	CSelecter	output((lvar.GetDepth() == 1) ? &dupl : NULL, type);
	char		exec_end     = 0;	// この{}の実行を終了するためのフラグ 1で終了
	char		ifflg        = 0;	// if-elseif-else制御用。1でそのブロックを処理したことを示す
	int			foreachcount;		// foreach制御用

	CValue		t_value;

	int	t_statelenm1 = statelenm1;
	int i;
	for(i = line; i < t_statelenm1; i++) {
		switch(statement[i].type) {
		case ST_OPEN:					// "{"
			i = ExecuteInBrace(i + 1, t_value, lvar, BRACE_DEFAULT, exitcode);
			output.Append(t_value);
			break;
		case ST_CLOSE:					// "}"　注　関数終端の"}"はここを通らない
			exec_end = 1;
			break;
		case ST_COMBINE:				// "--"
			output.AddArea();
			break;
		case ST_FORMULA_OUT_FORMULA:	// 出力（数式。配列、引数つき関数も含まれる）
			{
		        CValue val = GetFormulaAnswer(lvar, statement[i]);
				output.Append(val);
			}
			break;
		case ST_FORMULA_SUBST:			// 代入
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
		default:						// 未知のステートメント
			logger.Error(E_E, 82, dicfilename, statement[i].linecount);
			break;
		};
		if (exec_end)
			break;

		if (exitcode != ST_NOP)
			FeedLineToTail(i);
	}

	// 終了時の処理
	lvar.DelDepth();

	// 候補から出力を選び出す　入れ子の深さが0なら重複回避が働く
	result = output.Output();

	return i;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::Foreach
 *  機能概要：  foreach処理を行います
 *  返値　　：  0/1=ループ脱出/ループ実行
 *
 *  実際に送るのは"}"の1つ手前の行の位置です
 * -----------------------------------------------------------------------
 */
char	CFunction::Foreach(CStatement &st0, CStatement &st1, CLocalVariable &lvar, int &foreachcount)
{
	// 代入値を求める
	CValue	value = GetFormulaAnswer(lvar, st0);
	if (value.GetType() == F_TAG_NOP)
		value = L"";

	// 代入値の要素数を求める
	// 簡易配列かつ変数からの取得の場合、その変数に設定されているデリミタを取得する
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

	// 序数が要素数を超えていたらループ脱出
	if (foreachcount >= sz)
		return 0;

	// 代入する要素値を取得
	CValue	t_value;
	if (value.GetType() == F_TAG_STRING)
		t_value = s_array[foreachcount];
	else	// F_TAG_ARRAY
		t_value = value.array[foreachcount];
	// 代入
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

	// 序数を進める
	foreachcount++;

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::GetFormulaAnswer
 *  機能概要：  数式を演算して結果を返します
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
 *  関数名  ：  CFunction::GetValuePtrForCalc
 *  機能概要：  与えられた項に対応する値へのポインタを取得します
 * -----------------------------------------------------------------------
 */
CValue	*CFunction::GetValuePtrForCalc(CCell &cell, CStatement &st, CLocalVariable &lvar)
{
	// 即値はv、関数/変数/演算子項ならansvから取得　関数/変数の場合その値や実行結果が取得される

	// %[n]処理
	if (cell.v.GetType() == F_TAG_SYSFUNCPARAM)
		ExecHistoryP2(cell, st);

	// 演算が完了している（はずの）項ならそれを返す
	if (cell.v.GetType() < F_TAG_ORIGIN_VALUE)
		return &(cell.ansv);

	// 即値ならそれをそのまま返す
	if (cell.v.GetType() <= F_TAG_STRING)
		return &(cell.v);

	// 関数なら実行して結果を、変数ならその内容を返す
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
 *  関数名  ：  CFunction::SolveEmbedCell
 *  機能概要：  %埋め込み付文字列項の値を求めます
 * -----------------------------------------------------------------------
 */
void	CFunction::SolveEmbedCell(CCell &cell, CStatement &st, CLocalVariable &lvar)
{
	// 分割する位置を求める
	int	solve_src;	// 種別 0/1/2/3=ローカル変数/変数/関数/システム関数
	int	max_len = 0;	// 最長一致検索用

	if (cell.v.s_value[0] == L'_') {
		// ローカル変数
		solve_src = 0;
		max_len   = lvar.GetMacthedLongestNameLength(cell.v.s_value);
	}
	else {
		// 変数
		solve_src = 1;
		max_len   = variable.GetMacthedLongestNameLength(cell.v.s_value);
		// 関数
		int	t_len = 0;
		for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
			if (!it->name.compare(cell.v.s_value.substr(0, it->namelen)))
				if (t_len < it->namelen)
					t_len = it->namelen;
		if (t_len > max_len) {
			solve_src = 2;
			max_len   = t_len;
		}
		// システム関数
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
	// 存在しなければ全体が文字列ということになる
	if (!max_len) {
		cell.ansv     = L"%" + cell.v.s_value;
		cell.emb_ansv = L"%" + cell.v.s_value;
		return;
	}

	// 関数/システム関数の場合は引数部分を探す
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

	// 配列部分を探す
	int	depth = 1;
	int i;
	for(i = max_len + 1; i < len; i++) {
		if (!depth && cell.v.s_value[i] != L'[')
			break;
		depth += ((cell.v.s_value[i] == L'[') - (cell.v.s_value[i] == L']'));
	}
	if (!depth)
		max_len = i;

	// 埋め込まれた要素とそれ以降の文字列に分割する
	wstring	s_value(cell.v.s_value.substr(0, max_len));
	wstring	d_value(cell.v.s_value.substr(max_len, len - max_len));
	// 埋め込まれた要素を数式に変換する　失敗なら全体が文字列
	CStatement	t_state(ST_FORMULA, st.linecount);
	if (CParser0::ParseEmbedString(s_value, t_state, dicfilename, st.linecount)) {
		cell.ansv     = L"%" + cell.v.s_value;
		cell.emb_ansv = L"%" + cell.v.s_value;
		return;
	}

	// 埋め込み要素の値を取得して応答文字列を作成
	wstring	result = GetFormulaAnswer(lvar, t_state).GetValueString();
	cell.emb_ansv  = result;
	cell.ansv      = result + d_value;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::Cumma
 *  機能概要：  ,演算子を処理します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
char	CFunction::Cumma(CValue &answer, vector<int> &sid, CStatement &st, CLocalVariable &lvar)
{
	// 結合して配列値を作成
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
 *  関数名  ：  CFunction::Subst
 *  機能概要：  代入演算子を処理します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
char	CFunction::Subst(int type, CValue &answer, vector<int> &sid, CStatement &st, CLocalVariable &lvar)
{
	CCell	*s_cell = &(st.cell[sid[0]]);
	CCell	*d_cell = &(st.cell[sid[1]]);

	// 代入元の値を取得　演算子つきなら演算も行う
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

	// 代入を実行
	// 配列要素への代入は操作が複雑なので、さらに他の関数へ処理を渡す
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
 *  関数名  ：  CFunction::SubstToArray
 *  機能概要：  配列要素への代入を処理します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
char	CFunction::SubstToArray(CCell &vcell, CCell &ocell, CValue &answer, CStatement &st, CLocalVariable &lvar)
{
	// 序数を取得
	CValue	t_order = EncodeArrayOrder(vcell, ocell.order, lvar);
	if (t_order.GetType() == F_TAG_UNKNOWN)
		return 1;

	// 値を取得
	CValue	value = *(GetValuePtrForCalc(vcell, st, lvar)->SafeThis());

	// 更新
	value.SetArrayValue(t_order, answer);

	// 代入
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
 *  関数名  ：  CFunction::Array
 *  機能概要：  配列[]演算子を処理します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
char	CFunction::Array(CCell &anscell, vector<int> &sid, CStatement &st, CLocalVariable &lvar)
{
	CCell	*v_cell = &(st.cell[sid[0]]);
	CCell	*n_cell = &(st.cell[sid[1]]);

	// 序数を取得
	anscell.order = *(GetValuePtrForCalc(*n_cell, st, lvar)->SafeThis());

	CValue	t_order = EncodeArrayOrder(*v_cell, anscell.order, lvar);
	if (t_order.GetType() == F_TAG_UNKNOWN) {
		anscell.ansv = L"";
		return 1;
	}

	// 値を取得
	anscell.ansv = (*(GetValuePtrForCalc(*v_cell, st, lvar)->SafeThis()))[t_order];

	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::_in_
 *  機能概要：  _in_演算子を処理します
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
 *  関数名  ：  CFunction::not_in_
 *  機能概要：  !_in_演算子を処理します
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
 *  関数名  ：  CFunction::ExecFunctionWithArgs
 *  機能概要：  引数付きの関数を実行します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
char	CFunction::ExecFunctionWithArgs(CValue &answer, vector<int> &sid, CStatement &st, CLocalVariable &lvar)
{
	// 関数の格納位置を取得
	vector<int>::iterator it = sid.begin();
	int	index = st.cell[*it].index;
	it++;

	// 引数作成
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

	// 実行
	CLocalVariable	t_lvar;
	int	exitcode;
	answer = function[index].Execute(arg, t_lvar, exitcode);

	// フィードバック
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
 *  関数名  ：  CFunction::ExecSystemFunctionWithArgs
 *  機能概要：  引数付きのシステム関数を実行します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
char	CFunction::ExecSystemFunctionWithArgs(CCell& cell, vector<int> &sid, CStatement &st, CLocalVariable &lvar)
{
	// 関数の格納位置を取得
	vector<int>::iterator it = sid.begin();
	int	func_index = *it;
	int	index = st.cell[func_index].index;
	it++;

	// 引数作成
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

	// 実行　%[n]処理関数のみ特例扱い
	if (index == SYSFUNC_HIS)
		ExecHistoryP1(func_index - 2, cell, arg, st);
	else
		cell.ansv = sysfunction.Execute(index, arg, pcellarg, lvar, st.linecount, this);

	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::ExecHistoryP1
 *  機能概要：  %[n]（置換済の値の再利用）を処理します（前処理）
 *
 *  処理は二段階で行われます。前処理では本処理のための値を演算子の項へセットします。
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
 *  関数名  ：  CFunction::ExecHistoryP2
 *  機能概要：  %[n]（置換済の値の再利用）を処理します（本処理）
 *
 *  処理は二段階で行われます。本処理では前処理で埋め込んだ値を参照して値を取得します。
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
 *  関数名  ：  CFunction::Feedback
 *  機能概要：  &演算子を処理します
 *
 *  返値　　：  0/1=成功/エラー
 * -----------------------------------------------------------------------
 */
char	CFunction::Feedback(CCell &anscell, vector<int> &sid, CStatement &st, CLocalVariable &lvar)
{
	CCell	*v_cell = &(st.cell[sid[1]]);

	// 値は右辺をそのままコピー
	anscell.ansv = *(GetValuePtrForCalc(*v_cell, st, lvar)->SafeThis());

	// 右辺が配列序数を指定する演算子だった場合はそこから序数をコピー
	// 配列でなかった場合は序数を格納する変数の型をNOPにしてフラグとする
	if (v_cell->v.GetType() == F_TAG_ARRAYORDER)
		anscell.order = v_cell->order;
	else
		anscell.order.SetType(F_TAG_NOP);

	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::Exc
 *  機能概要：  !演算子を処理します
 * -----------------------------------------------------------------------
 */
CValue	CFunction::Exc(CValue &value)
{
	return CValue(1 - value.GetTruth());
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFunction::EncodeArrayOrder
 *  機能概要：  配列の序数を作成して返します
 *
 *  CValue operator [] は引数として要素数2以上の配列型のCValueを要求します。
 *  （第一引数が序数、第二引数がデリミタ）
 *  この関数はそれを作成します。
 *
 *  エラーが発生した場合は型のない（F_TAG_UNKNOWN）値を返します。（呼び出し側はこれを見てエラー処理します）
 * -----------------------------------------------------------------------
 */
CValue	CFunction::EncodeArrayOrder(CCell &vcell, CValue &order, CLocalVariable &lvar)
{
	CValue	result(F_TAG_ARRAY, 0/*dmy*/);

	// 序数
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

	// デリミタ
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
 *  関数名  ：  CFunction::FeedLineToTail
 *  機能概要：  現在の{}の終端"}"まで実行行カウンタを送ります
 *
 *  実際に送るのは"}"の1つ手前の行の位置です
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

