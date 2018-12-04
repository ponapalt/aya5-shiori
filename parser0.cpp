// 
// AYA version 5
//
// �\�����/���ԃR�[�h�̐������s���N���X�@CParser0
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#else
#  include "posix_utils.h"
#  include "messages.h"
#endif
#include "basis.h"
#include "parser0.h"
#include "parser1.h"
#include "sysfunc.h"
#include "comment.h"
#include "misc.h"
#include "log.h"
#include "wsex.h"
#include "ccct.h"
#include "wordmatch.h"

extern CBasis				basis;
extern vector<CFunction>	function;
extern CGlobalVariable		variable;
extern CLog					logger;

extern CWordMatch			formulatag_wm;
extern CWordMatch			function_wm;

#ifndef POSIX
#  define for if(0);else for
#endif

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::Parse
 *  �@�\�T�v�F  �w�肳�ꂽ�����t�@�C���Q��ǂݎ��A���s�\�Ȋ֐��Q���쐬���܂�
 *
 *  �Ԓl�@�@�F  0/1=����/�G���[
 * -----------------------------------------------------------------------
 */
char	CParser0::Parse(int charset, const vector<wstring>& dics, int &lindex, int &ulindex, int &rindex)
{
	// �ǂݎ��A�\����́A���ԃR�[�h�̐���
	logger.Message(3);
	vector<CDefine>	gdefines;
	int	errcount = 0;
	for(vector<wstring>::const_iterator it = dics.begin(); it != dics.end(); it++) {
		logger.Write(L"// ");
		logger.Filename(*it);
		errcount += LoadDictionary1(*it, gdefines, charset);
	}
	logger.Message(8);
	logger.Message(9);

	errcount += AddSimpleIfBrace();

	errcount += SetCellType();
	errcount += MakeCompleteFormula();

	// ���ԃR�[�h�����̌㏈���ƌ���
	errcount += CParser1::CheckExecutionCode();

	// load/unload/request�֐��̈ʒu���擾
	lindex  = GetFunctionIndexFromName(FUNCNAME_LOAD);
//	if (lindex == -1) {
//		logger.Error(E_E, 32, FUNCNAME_LOAD);
//		errcount++;
//	}
	ulindex = GetFunctionIndexFromName(FUNCNAME_UNLOAD);
//	if (ulindex == -1) {
//		logger.Error(E_E, 32, FUNCNAME_UNLOAD);
//		errcount++;
//	}
	rindex  = GetFunctionIndexFromName(FUNCNAME_REQUEST);
//	if (rindex == -1) {
//		logger.Error(E_E, 32, FUNCNAME_REQUEST);
//		errcount++;
//	}
	logger.Message(8);

	return (errcount) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::ParseEmbedString
 *  �@�\�T�v�F  ����������Z�\�Ȑ����ɕϊ����܂�
 *
 *  �Ԓl�@�@�F  0/1=����/�G���[
 * -----------------------------------------------------------------------
 */
char	CParser0::ParseEmbedString(wstring& str, CStatement &st, const wstring& dicfilename, int linecount)
{
	// ������𐔎��ɐ��`
	if (!StructFormula(str, st.cell, dicfilename, linecount))
		return 1;

	// �����̍��̎�ނ𔻒�
	for(vector<CCell>::iterator it = st.cell.begin(); it != st.cell.end(); it++) {
		if (it->v.GetType() != F_TAG_NOP)
			continue;

		if (SetCellType1(*it, 0, dicfilename, linecount))
			return 1;
	}

	// ()�A[]�̐���������
	if (CheckDepth1(st, dicfilename))
		return 1;

	// ���ߍ��ݗv�f�̕���
	if (ParseEmbeddedFactor1(st, dicfilename))
		return 1;

	// �V���O���N�H�[�g�������ʏ핶����֒u��
	ConvertPlainString1(st, dicfilename);

	// ���Z��������
	if (CheckDepthAndSerialize1(st, dicfilename))
		return 1;

	// �㏈���ƌ���
	if (CParser1::CheckExecutionCode1(st, dicfilename))
		return 1;

	return 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::LoadDictionary1
 *  �@�\�T�v�F  ��̎����t�@�C����ǂݎ��A��G�c�ɍ\�������߂��Ē~�ς��Ă����܂�
 *
 *  �Ԓl�@�@�F  1/0=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::LoadDictionary1(const wstring& filename, vector<CDefine>& gdefines, int charset)
{
        wstring file = filename;
#ifdef POSIX
	fix_filepath(file);
#endif
	// �t�@�C�����J��
	FILE	*fp = w_fopen((wchar_t *)file.c_str(), L"r");
	if (fp == NULL) {
		logger.Error(E_E, 5, file);
		return 1;
	}

	// �ǂݎ��
	CComment	comment;
	char	ciphered = IsCipheredDic(file);
	wstring	linebuffer;
	int	depth = 0;
	int	targetfunction = -1;
	vector<CDefine>	defines;
	int	errcount = 0;
	for (int i = 1; ; i++) {
		// 1�s�ǂݍ��݁@�Í����t�@�C���̏ꍇ�͕������s�Ȃ�
		wstring	readline;
		int	ret;
		ret = ws_fgets(readline, fp, charset, ciphered, i);
		if (ret == WS_EOF)
			break;
		// �I�[�̉��s����ѕs�v�ȋ󔒁i�C���f���g���j������
		CutCrLf(readline);
		CutSpace(readline);
		// �R�����g����
		comment.ExecuteSS_Top(readline);
		comment.Execute(readline);
		// ��s�i�������͑S�̂��R�����g�s�������j�Ȃ玟��
		if (readline.size() == 0)
			continue;
		// �ǂݎ��σo�b�t�@�֌���
		linebuffer.append(readline);
		// �I�[��"/"�Ȃ猋���Ȃ̂�"/"�������Ď���ǂ�
		if (readline[readline.size() - 1] == L'/') {
			linebuffer.erase(linebuffer.end() - 1);
			continue;
		}
		// �v���v���Z�b�T�̏ꍇ�͎擾
		int	pp = GetPreProcess(linebuffer, defines, gdefines, file, i);
		// �v���v���Z�b�T�ł������炱�̍s�̏����͏I���A����
		// �ُ�ȃv���v���Z�X�s�̓G���[
		if (pp == 1)
			continue;
		else if (pp == 2) {
			errcount = 1;
			continue;
		}
		// �v���v���Z�b�T#define�A#globaldefine����
		ExecDefinePreProcess(linebuffer, defines);	// #define
		ExecDefinePreProcess(linebuffer, gdefines);	// #globaldefine
		// �R�����g����(2)
		comment.ExecuteSS_Tail(linebuffer);
		// {�A}�A;�ŕ���
		vector<wstring>	factors;
		SeparateFactor(factors, linebuffer);
		// �������ꂽ���������͂��Ċ֐����쐬���A�����̃X�e�[�g�����g��~�ς��Ă���
		if (DefineFunctions(factors, file, i, depth, targetfunction))
			errcount = 1;
	}

	// �t�@�C�������
	::fclose(fp);

	return errcount;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::GetPreProcess
 *  �@�\�T�v�F  ���͂�#define/#globaldefine�v���v���Z�b�T�ł������ꍇ�A�����擾���܂�
 *
 *  �Ԓl�@�@�F  0/1/2=�v���v���Z�X�ł͂Ȃ�/�v���v���Z�X/�G���[
 * -----------------------------------------------------------------------
 */
char	CParser0::GetPreProcess(wstring &str, vector<CDefine>& defines, vector<CDefine>& gdefines, const wstring& dicfilename,
			int linecount)
{
	wstring	base_str = str;

	// �擪1�o�C�g��"#"�ł��邩���m�F
	// �i���̊֐��ɗ���܂łɋ󕶎���͏��O����Ă���̂ŁA�����Ȃ�[0]���Q�Ƃ��Ă����Ȃ��j
	if (base_str[0] != L'#')
		return 0;

	str = L"";

	// �v���v���Z�X���́A�ϊ��O������A�ϊ��㕶������擾
	// �擾�ł��Ȃ���΃G���[
	wstring	pname, bef, aft;

	for(int i = 0; i < 2; i++) {
		// �f���~�^�ł���󔒂������̓^�u��T��
		int	spc_pos = base_str.find(L' ' );
		int	tab_pos = base_str.find(L'\t');
#ifndef POSIX
		int	wsp_pos = base_str.find(L'�@');
#else
		int	wsp_pos = base_str.find(L'\u3000');
#endif

		int	sep_pos = -1;
		if (spc_pos != -1)
			sep_pos = spc_pos;
		if (tab_pos != -1 && (sep_pos == -1 || (sep_pos != -1 && tab_pos < sep_pos)))
			sep_pos = tab_pos;
		if (wsp_pos != -1 && (sep_pos == -1 || (sep_pos != -1 && wsp_pos < sep_pos)))
			sep_pos = wsp_pos;
		if (sep_pos == -1) {
			logger.Error(E_E, 74, dicfilename, linecount);
			return 2;
		}
		// �f���~�^�ŕ������Ċe�v�f���擾
		if (!i) {
			pname.assign(base_str, 0, sep_pos);
			CutSpace(pname);
			base_str.erase(0, sep_pos);
			CutSpace(base_str);
		}
		else {
			bef.assign(base_str, 0, sep_pos);
			CutSpace(bef);
			base_str.erase(0, sep_pos);
			aft = base_str;
			CutSpace(aft);
		}
	}

	if (!pname.size() || !bef.size() || !aft.size()) {
		logger.Error(E_E, 75, dicfilename, linecount);
		return 2;
	}

	// ��ʂ̔���Ə��̕ێ�
	if (!pname.compare(L"#define")) {
		CDefine	adddefine(bef, aft);
		defines.push_back(adddefine);
	}
	else if (!pname.compare(L"#globaldefine")) {
		CDefine	adddefine(bef, aft);
		gdefines.push_back(adddefine);
	}
	else {
		logger.Error(E_E, 76, pname, dicfilename, linecount);
		return 2;
	}

	return 1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::ExecDefinePreProcess
 *  �@�\�T�v�F  #define/#globaldefine�����B�������u�����܂�
 * -----------------------------------------------------------------------
 */
void	CParser0::ExecDefinePreProcess(wstring &str, const vector<CDefine>& defines)
{
	for(vector<CDefine>::const_iterator it = defines.begin(); it != defines.end(); it++)
		ws_replace(str, it->before, it->after);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::IsCipheredDic
 *  �@�\�T�v�F  �t�@�C�����Í����t�@�C�������t�@�C���g���q�����Ĕ��f���܂�
 *
 *  �Ԓl�@�@�F  1/0=�g���q��.ayc�ł���/.ayc�ł͂Ȃ�
 * -----------------------------------------------------------------------
 */
char	CParser0::IsCipheredDic(const wstring& filename)
{
	int	len = filename.size();
	if (len < 5)
		return 0;

	return ((filename[len - 3] == L'a' || filename[len - 3] == L'A') &&
		    (filename[len - 2] == L'y' || filename[len - 2] == L'Y') &&
		    (filename[len - 1] == L'c' || filename[len - 1] == L'C'))
			? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::SeparateFactor
 *  �@�\�T�v�F  �^����ꂽ�������"{"�A"}"�A";"�̈ʒu�ŕ������܂��B";"�͈ȍ~�s�v�Ȃ̂ŏ������݂܂�
 * -----------------------------------------------------------------------
 */
void	CParser0::SeparateFactor(vector<wstring> &s, wstring &line)
{
	int		separatepoint = 0;
	int		apoint        = 0;
	int		len           = line.size();
	char	executeflg    = 0;
	for( ; ; ) {
		// { ����(0)
		int	newseparatepoint = -1;
		int	type = 0;
		int	tmppoint = line.find(L'{', separatepoint);
		if (tmppoint != -1)
			newseparatepoint = tmppoint;
		// } ����(1)
		tmppoint = line.find(L'}', separatepoint);
		if (tmppoint != -1)
			if (newseparatepoint == -1 ||
				(newseparatepoint != -1 && tmppoint < newseparatepoint)) {
				newseparatepoint = tmppoint;
				type = 1;
			}
		// ; ����(2)
		tmppoint = line.find(L';', separatepoint);
		if (tmppoint != -1)
			if (newseparatepoint == -1 ||
				(newseparatepoint != -1 && tmppoint < newseparatepoint)) {
				newseparatepoint = tmppoint;
				type = 2;
			}
		// ����������Ȃ���ΏI���
		if (newseparatepoint == -1)
			break;
		// �����ʒu���_�u���N�H�[�g���Ȃ疳�����Đ�֐i��
		if (IsInDQ(line, 0, newseparatepoint)) {
			separatepoint = newseparatepoint + 1;
			continue;
		}
		// �����ʒu�̎�O�̕�������擾
		if (newseparatepoint > apoint) {
			wstring	tmpstr;
			tmpstr.assign(line, apoint, newseparatepoint - apoint);
			CutSpace(tmpstr);
			s.push_back(tmpstr);
		}
		// ���������̂�"{"��������"}"�Ȃ炱����擾
		if (type == 0)
			s.push_back(L"{");
		else if (type == 1)
			s.push_back(L"}");
		// �����J�n�ʒu���X�V
		apoint = separatepoint = newseparatepoint + 1;
		if (separatepoint >= len) {
			executeflg = 1;
			break;
		}
	}

	// �܂������񂪎c���Ă���Ȃ炻����ǉ�
	if (executeflg == 0) {
		wstring	tmpstr;
		tmpstr.assign(line, apoint, len - apoint);
		CutSpace(tmpstr);
		s.push_back(tmpstr);
	}

	// ���̕�����̓N���A����
	line = L"";
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::DefineFunctions
 *  �@�\�T�v�F  ������Q����͂��Ċ֐��̌��^���`����
 *
 *  �Ԓl�@�@�F  1/0=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::DefineFunctions(vector<wstring> &s, const wstring& dicfilename, int linecount, int &depth, int &targetfunction)
{
	char	retcode = 0;

	for(vector<wstring>::iterator it = s.begin(); it != s.end(); it++) {
		// ��s�̓X�L�b�v
		if (!(it->size()))
			continue;

		// {}����q�̐[�������Ċ֐�������������
		// �[����0�Ȃ�{}�̊O�Ȃ̂Ŋ֐������擾���ׂ��ʒu�ł���
		if (!depth) {
			// �֐��̍쐬
			if (targetfunction == -1) {
				// �֐����Əd������I�v�V�������擾
				wstring	d0, d1;
				if (!Split(*it, d0, d1, L":"))
					d0 = *it;
				// �֐����̐���������
				if (IsLegalFunctionName(d0)) {
					if (!it->compare(L"{"))
						logger.Error(E_E, 1, L"{", dicfilename, linecount);
					else if (!it->compare(L"}"))
						logger.Error(E_E, 2, dicfilename, linecount);
					else
						logger.Error(E_E, 3, d0, dicfilename, linecount);
					return 1;
				}
				// �d������I�v�V�����̔���
				int	chtype = CHOICETYPE_RANDOM;
				if (d1.size()) {
				        int i;
					for(i = 0; i < CHOICETYPE_NUM; i++)
						if (!d1.compare(choicetype[i])) {
							chtype = i;
							break;
						}
					if (i == CHOICETYPE_NUM) {
						logger.Error(E_E, 30, d1, dicfilename, linecount);
						return 1;
					}
				}
				// �쐬
				targetfunction = MakeFunction(d0, chtype, dicfilename);
				if (targetfunction == -1) {
				        logger.Error(E_E, 13, *it, dicfilename, linecount);
					return 1;
				}
				continue;
			}
			// �֐����̎��̃X�e�[�g�����g�@�����"{"�łȂ���΂Ȃ�Ȃ�
			else {
				if (it->compare(L"{")) {
					logger.Error(E_E, 4, dicfilename, linecount);
					return 1;
				}
				// ����q�̐[�������Z
				depth++;
			}
		}
		else {
			// �֐����̃X�e�[�g�����g�̒�`�@{}����q�̌v�Z�������ōs��
			if (!StoreInternalStatement(targetfunction, *it, depth, dicfilename, linecount))
				retcode = 1;
			// ����q�[����0�ɂȂ����炱�̊֐���`����E�o����
			if (!depth)
				targetfunction = -1;
		}
	}

	return retcode;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::MakeFunction
 *  �@�\�T�v�F  ���O���w�肵�Ċ֐����쐬���܂�
 *
 *  �Ԓl�@�@�F  �쐬���ꂽ�ʒu��Ԃ��܂�
 *  �@�@�@�@�@  �w�肳�ꂽ���O�̊֐������ɍ쐬�ς̏ꍇ�̓G���[�ŁA-1��Ԃ��܂�
 * -----------------------------------------------------------------------
 */
int	CParser0::MakeFunction(const wstring& name, int chtype, const wstring& dicfilename)
{
	int	i = GetFunctionIndexFromName(name);
	if(i != -1)
		return -1;
/*	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
		if (!name.compare(it->name))
			return -1;
*/

	CFunction	addfunction(name, chtype, dicfilename);
	function.push_back(addfunction);
	function_wm.addWord(name, function.size() - 1);

	return function.size() - 1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::StoreInternalStatement
 *  �@�\�T�v�F  �֐����̃X�e�[�g�����g����ނ𔻒肵�Ē~�ς��܂�
 *
 *  �Ԓl�@�@�F  0/1=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::StoreInternalStatement(int targetfunc, wstring &str, int& depth, const wstring& dicfilename, int linecount)
{
	// �p�����[�^�̂Ȃ��X�e�[�g�����g

	// {
	if (!str.compare(L"{")) {
		depth++;
		CStatement	addstatement(ST_OPEN, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// }
	else if (!str.compare(L"}")) {
		depth--;
		CStatement	addstatement(ST_CLOSE, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// others�@else�֏��������Ă��܂�
	else if (!str.compare(L"others")) {
		CStatement	addstatement(ST_ELSE, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// else
	else if (!str.compare(L"else")) {
		CStatement	addstatement(ST_ELSE, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// break
	else if (!str.compare(L"break")) {
		CStatement	addstatement(ST_BREAK, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// continue
	else if (!str.compare(L"continue")) {
		CStatement	addstatement(ST_CONTINUE, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// return
	else if (!str.compare(L"return")) {
		CStatement	addstatement(ST_RETURN, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}
	// --
	else if (!str.compare(L"--")) {
		CStatement	addstatement(ST_COMBINE, linecount);
		function[targetfunc].statement.push_back(addstatement);
		return 1;
	}

	// �p�����[�^�̂���X�e�[�g�����g�i���䕶�j
	wstring	st, par;
	if (!Split(str, st, par, L" "))
		st = str;
	wstring	t_st, t_par;
	if (!Split(str, t_st, t_par, L"\t"))
		t_st = str;
	if (st.size() > t_st.size()) {
		st  = t_st;
		par = t_par;
	}
	// if
	if (!st.compare(L"if")) {
		str = par;
		return MakeStatement(ST_IF, targetfunc, str, dicfilename, linecount);
	}
	// elseif
	else if (!st.compare(L"elseif")) {
		str = par;
		return MakeStatement(ST_ELSEIF, targetfunc, str, dicfilename, linecount);
	}
	// while
	else if (!st.compare(L"while")) {
		str = par;
		return MakeStatement(ST_WHILE, targetfunc, str, dicfilename, linecount);
	}
	// switch
	else if (!st.compare(L"switch")) {
		str = par;
		return MakeStatement(ST_SWITCH, targetfunc, str, dicfilename, linecount);
	}
	// for
	else if (!st.compare(L"for")) {
		str = par;
		return MakeStatement(ST_FOR, targetfunc, str, dicfilename, linecount);
	}
	// foreach
	else if (!st.compare(L"foreach")) {
		str = par;
		return MakeStatement(ST_FOREACH, targetfunc, str, dicfilename, linecount);
	}
	// case�@����Ȗ��O�̃��[�J���ϐ��ւ̑���ɏ��������Ă��܂�
	else if (!st.compare(L"case")) {
		str = PREFIX_CASE_VAR + function[targetfunc].name;
		wstring	tmpstr;
		ws_itoa(tmpstr, linecount, 10);
		str += tmpstr;
		str += L"=";
		str += par;
		return MakeStatement(ST_FORMULA, targetfunc, str, dicfilename, linecount);
	}
	// when
	else if (!st.compare(L"when")) {
		str = par;
		return MakeStatement(ST_WHEN, targetfunc, str, dicfilename, linecount);
	}
	// parallel
	else if (!st.compare(L"parallel")) {
		str = par;
		return MakeStatement(ST_PARALLEL, targetfunc, str, dicfilename, linecount);
	}

	// ����܂ł̂��ׂĂɃ}�b�`���Ȃ�������͐����ƔF�������
	return MakeStatement(ST_FORMULA, targetfunc, str, dicfilename, linecount);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::MakeStatement
 *  �@�\�T�v�F  �֐����̃X�e�[�g�����g���쐬�A�~�ς��܂�
 *
 *  �Ԓl�@�@�F  0/1=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::MakeStatement(int type, int targetfunc, wstring &str, const wstring& dicfilename, int linecount)
{
	if (!str.size()) {
		logger.Error(E_E, 27, dicfilename, linecount);
		return 0;
	}

	CStatement	addstatement(type, linecount);
	if (type == ST_WHEN) {
		if (!StructWhen(str, addstatement.cell, dicfilename, linecount))
			return 0;
	}
	else {
		if (!StructFormula(str, addstatement.cell, dicfilename, linecount))
			return 0;
	}

	function[targetfunc].statement.push_back(addstatement);
	return 1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::StructFormula
 *  �@�\�T�v�F  ������𐔎��̍��Ɖ��Z�q�ɕ������A��{�I�Ȑ��`���s���܂�
 *
 *  �Ԓl�@�@�F  0/1=�G���[/����
 *
 *  �n���ꂽ������͂��̊֐��Ŕj�󂳂�܂�
 * -----------------------------------------------------------------------
 */
char	CParser0::StructFormula(wstring &str, vector<CCell> &cells, const wstring& dicfilename, int linecount)
{
	// ���Z�q�ƍ��ɕ����@���̎�ʂ͂��̎��_�ł͒��ׂĂ��Ȃ�
	StructFormulaCell(str, cells);

	// ���`�Ə����G���[�̌��o
	char	bracket = 0;
	for(vector<CCell>::iterator it = cells.begin(); it != cells.end(); ) {
		// ���O��")""]"�̏ꍇ�A���͉��Z�q�����Ȃ���΂Ȃ�Ȃ�
		if (bracket) {
			bracket = 0;
			if (it->v.GetType() == F_TAG_NOP) {
				logger.Error(E_E, 92, dicfilename, linecount);
				return 0;
			}
		}
		// �������i+�̒��O�̍������������邢��")""]"�ȊO�̉��Z�q�@���̍��͏����Ă悢�j
		if (it->v.GetType() == F_TAG_PLUS) {
			if (it == cells.begin()) {
				it = cells.erase(it);
				continue;
			}
			vector<CCell>::iterator	itm = it;
			itm--;
			if (itm->v.GetType() != F_TAG_NOP &&
				itm->v.GetType() != F_TAG_BRACKETOUT &&
				itm->v.GetType() != F_TAG_HOOKBRACKETOUT) {
				it = cells.erase(it);
				continue;
			}
		}
		// �������i-�̒��O�̍������������邢��")"�ȊO�̉��Z�q�@"-1*"�ɐ��`����j
		if (it->v.GetType() == F_TAG_MINUS) {
			if (it == cells.begin()) {
				it->v.SetType(F_TAG_NOP);
				it->v.s_value = L"-1";
				it++;
				CCell	addcell2(F_TAG_MUL);
				it = cells.insert(it, addcell2);
				it++;
				continue;
			}
			vector<CCell>::iterator	itm = it;
			itm--;
			if (itm->v.GetType() != F_TAG_NOP &&
				itm->v.GetType() != F_TAG_BRACKETOUT &&
				itm->v.GetType() != F_TAG_HOOKBRACKETOUT) {
				it->v.SetType(F_TAG_NOP);
				it->v.s_value = L"-1";
				it++;
				CCell	addcell2(F_TAG_MUL);
				it = cells.insert(it, addcell2);
				it++;
				continue;
			}
		}
		// �C���N�������g�i"+=1"�ɐ��`����j
		if (it->v.GetType() == F_TAG_INCREMENT) {
			it->v.SetType(F_TAG_PLUSEQUAL);
			it++;
			CCell	addcell(F_TAG_NOP);
			addcell.v.s_value = L"1";
			it = cells.insert(it, addcell);
			it++;
			continue;
		}
		// �f�N�������g�i"-=1"�ɐ��`����j
		if (it->v.GetType() == F_TAG_DECREMENT) {
			it->v.SetType(F_TAG_MINUSEQUAL);
			it++;
			CCell	addcell(F_TAG_NOP);
			addcell.v.s_value = L"1";
			it = cells.insert(it, addcell);
			it++;
			continue;
		}
		// !�A&���Z�q�i��O�Ƀ_�~�[��0��ǉ��j
		if (it->v.GetType() == F_TAG_FEEDBACK) {
			if (it == cells.begin()) {
				logger.Error(E_E, 87, dicfilename, linecount);
				return 0;
			}
			CCell	addcell(F_TAG_NOP);
			addcell.v.s_value = L"0";
			it = cells.insert(it, addcell);
			it += 2;
			continue;
		}
		if (it->v.GetType() == F_TAG_EXC) {
			CCell	addcell(F_TAG_NOP);
			addcell.v.s_value = L"0";
			it = cells.insert(it, addcell);
			it += 2;
			continue;
		}
		// �ȈՔz�񏘐��A�N�Z�X���Z�q�i"["���O�ɕt�^�j
		if (it->v.GetType() == F_TAG_HOOKBRACKETIN) {
			if (it == cells.begin()) {
				logger.Error(E_E, 6, dicfilename, linecount);
				return 0;
			}
			CCell	addcell(F_TAG_ARRAYORDER);
			it = cells.insert(it, addcell);
			it += 2;
			continue;
		}
		// �֐��v�f�w�艉�Z�q�i"("���O�����Z�q�łȂ���Εt�^�j
		if (it->v.GetType() == F_TAG_BRACKETIN) {
			if (it != cells.begin()) {
				vector<CCell>::iterator	itm = it;
				itm--;
				if (itm->v.GetType() == F_TAG_NOP) {
					CCell	addcell(F_TAG_FUNCPARAM);
					it = cells.insert(it, addcell);
					it += 2;
					continue;
				}
			}
		}
		// ��̃J�b�R�i"()"�@�����@�֐��v�f�w�艉�Z�q���������ꍇ�͂���������j
		// �֐�����2�ȏ�ŋ�̈����i",)"�̏ꍇ�A""��⊮�j
		if (it->v.GetType() == F_TAG_BRACKETOUT) {
			bracket = 1;
			if (it != cells.begin()) {
				vector<CCell>::iterator	itm = it;
				itm--;
				if (itm->v.GetType() == F_TAG_BRACKETIN) {
					it = cells.erase(itm);
					it = cells.erase(it);
					if (it != cells.begin()) {
						itm = it;
						itm--;
						if (itm->v.GetType() == F_TAG_FUNCPARAM)
							it = cells.erase(itm);
					}
					continue;
				}
				else if (itm->v.GetType() == F_TAG_CUMMA) {
					CCell	addcell(F_TAG_NOP);
					addcell.v.s_value = L"\"\"";
					it = cells.insert(it, addcell);
					it += 2;
					continue;
				}
			}
		}
		// �����J�b�R�i"[]"�@�G���[�j
		if (it->v.GetType() == F_TAG_HOOKBRACKETOUT) {
			bracket = 1;
			if (it != cells.begin()) {
				vector<CCell>::iterator	itm = it;
				itm--;
				if (itm->v.GetType() == F_TAG_HOOKBRACKETIN) {
					logger.Error(E_E, 14, dicfilename, linecount);
					return 0;
				}
			}
		}
		// �֐�����2�ȏ�ŋ�̈����i"(,"�����",,"�̏ꍇ�A""��⊮�j
		if (it->v.GetType() == F_TAG_CUMMA) {
			if (it != cells.begin()) {
				vector<CCell>::iterator	itm = it;
				itm--;
				if (itm->v.GetType() == F_TAG_BRACKETIN || itm->v.GetType() == F_TAG_CUMMA) {
					CCell	addcell(F_TAG_NOP);
					addcell.v.s_value = L"\"\"";
					it = cells.insert(it, addcell);
					it += 2;
					continue;
				}
			}
		}
		// ���̍���
		it++;
	}

	return 1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::StructWhen
 *  �@�\�T�v�F  ������𐔎��̍��Ɖ��Z�q�ɕ������A��{�I�Ȑ��`���s���܂��iwhen�\���p�j
 *
 *  �Ԓl�@�@�F  0/1=�G���[/����
 *
 *  �n���ꂽ������͂��̊֐��Ŕj�󂳂�܂�
 * -----------------------------------------------------------------------
 */
char	CParser0::StructWhen(wstring &str, vector<CCell> &cells, const wstring& dicfilename, int linecount)
{
	// ���Z�q�ƍ��ɕ����@���̎�ʂ͂��̎��_�ł͒��ׂĂ��Ȃ�
	StructFormulaCell(str, cells);

	// ���`�Ə����G���[�̌��o
	for(vector<CCell>::iterator it = cells.begin(); it != cells.end(); ) {
		// �������i+�̒��O�̍������������邢��")"�ȊO�̉��Z�q�@���̍��͏����Ă悢�j
		if (it->v.GetType() == F_TAG_PLUS) {
			if (it == cells.begin()) {
				it = cells.erase(it);
				continue;
			}
			vector<CCell>::iterator	itm = it;
			itm--;
			if (itm->v.GetType() != F_TAG_NOP && itm->v.GetType() != F_TAG_BRACKETOUT) {
				it = cells.erase(it);
				continue;
			}
		}
		// �������i-�̒��O�̍��������@���̍��ɒP����-��t������j
		if (it->v.GetType() == F_TAG_MINUS) {
			if (it == cells.begin()) {
				it = cells.erase(it);
				it->v.s_value.insert(0, L"-");
				continue;
			}
			vector<CCell>::iterator	itm = it;
			itm--;
			if (itm->v.GetType() != F_TAG_NOP && itm->v.GetType() != F_TAG_BRACKETOUT) {
				it = cells.erase(it);
				it->v.s_value.insert(0, L"-");
				continue;
			}
		}
		// �f�N�������g �����Ȍ`�󂩂������̏�A- �Ǝ����ւ�-�t�^�Ƃ��ď�������
		if (it->v.GetType() == F_TAG_DECREMENT) {
			if (it == cells.begin()) {
				logger.Error(E_E, 65, dicfilename, linecount);
				return 0;
			}
			it->v.SetType(F_TAG_MINUS);
			it++;
			if (it == cells.end()) {
				logger.Error(E_E, 66, dicfilename, linecount);
				return 0;
			}
			it->v.s_value.insert(0, L"-");
			continue;
		}
		// ���̍���
		it++;
	}
	// ","��"||"�ցA"-"��"&&"�֕ϊ�����@�����ȉ��Z�q�ł̓G���[
	for(vector<CCell>::iterator it = cells.begin(); it != cells.end(); it++) {
		if (it->v.GetType() == F_TAG_CUMMA)
			it->v.SetType(F_TAG_OR);
		else if (it->v.GetType() == F_TAG_MINUS)
			it->v.SetType(F_TAG_AND);
		else if (it->v.GetType() >= F_TAG_ORIGIN && it->v.GetType() < F_TAG_ORIGIN_VALUE) {
			logger.Error(E_E, 50, dicfilename, linecount);
			return 0;
		}
	}

	return 1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::StructFormulaCell
 *  �@�\�T�v�F  ������𐔎��̍��Ɖ��Z�q�ɕ������܂�
 *
 *  �Ԓl�@�@�F  0/1=�G���[/����
 *
 *  �n���ꂽ������͂��̊֐��Ŕj�󂳂�܂�
 * -----------------------------------------------------------------------
 */
//#include <iostream>
void	CParser0::StructFormulaCell(wstring &str, vector<CCell> &cells)
{
	for( ; ; ) {
		// �����ʒu���擾�@�ł���O�ōł����O�������A�N�H�[�g����Ă��Ȃ����Z�q��T��
		int	tagpoint = -1;
		int	tagtype  = 0;
		int	taglen   = 0;
		wstring	bstr;
//		wcout << endl << "str: " << str << endl;
//		wcout << "WordMatch:" << endl;
		int in_dq = 0;
		int	in_sq = 0;
		for(unsigned int i = 0; i < str.size(); ++i) {
			if (str[i] == L'\"') {
				if (!in_sq)
					in_dq = 1 - in_dq;
				continue;
			}
			if (str[i] == L'\'') {
				if (!in_dq)
					in_sq = 1 - in_sq;
				continue;
			}
			if (in_dq || in_sq)
				continue;

			int result = formulatag_wm.search(str, i);
			if(result != -1) {
				tagtype = result;
				taglen  = formulatag_len[tagtype];
				
				wstring	d0, d1;
				d0.assign(str, 0, i);
				d1.assign(str, i + taglen, str.size());
				CutSpace(d0);
				CutSpace(d1);
				tagpoint = d0.size();
				bstr = d1;
//				wcout << "d0: " << d0 << endl;
//				wcout << "d1: " << d1 << endl;
				break;
			}
		}
/*		wcout << "  tagpoint: " << tagpoint << endl;
		wcout << "  tagtype: " << tagtype << endl;
		wcout << "  taglen: " << taglen << endl;
		wcout << "  bstr: " << bstr << endl;

		wcout << "Aya5:" << endl;
		tagpoint = -1;
		tagtype  = 0;
		taglen   = 0;

		for(int i = 0; i < FORMULATAG_NUM; i++) {
			wstring	d0, d1;
			if (!Split_IgnoreDQ(str, d0, d1, (wchar_t *)formulatag[i]))
				continue;
			int	d_point = d0.size();
			if (tagpoint == -1 ||
				(tagpoint != -1 && ((tagpoint > d_point) || tagpoint == d_point && taglen < formulatag_len[i]))) {
				tagpoint = d_point;
				tagtype  = i;
				taglen   = formulatag_len[i];
				bstr = d1;
//				wcout << "d0: " << d0 << endl;
//				wcout << "d1: " << d1 << endl;
			}
		}

		wcout << "  tagpoint: " << tagpoint << endl;
		wcout << "  tagtype: " << tagtype << endl;
		wcout << "  taglen: " << taglen << endl;
		wcout << "  bstr: " << bstr << endl;
*/
		// ������Ȃ��ꍇ�͍Ō�̍���o�^���Ĕ�����
		if (tagpoint == -1) {
			CCell	addcell(F_TAG_NOP);
			addcell.v.s_value = str;
			CutSpace(addcell.v.s_value);
			if (addcell.v.s_value.size())
				cells.push_back(addcell);
			break;
		}
		// ���������̂œo�^����
		else {
			// ���̓o�^�@�󕶎���͓o�^���Ȃ�
			if (tagpoint > 0) {
				CCell	addcell(F_TAG_NOP);
				addcell.v.s_value.assign(str, 0, tagpoint);
				CutSpace(addcell.v.s_value);
				if (addcell.v.s_value.size())
					cells.push_back(addcell);
			}
			// ���Z�q�̓o�^
			CCell	addcell(tagtype + F_TAG_ORIGIN);
			cells.push_back(addcell);
			// ���̕����񂩂���o���ς̗v�f�����
			str = bstr;
		}
	}
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::AddSimpleIfBrace
 *  �@�\�T�v�F  if/elseif/else/when/others�̒����'{'�������ꍇ�A���̍s��{}�ň݂͂܂�
 *
 *  �Ԓl�@�@�F  1/0=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::AddSimpleIfBrace(void)
{
	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++) {
		int	beftype = ST_UNKNOWN;
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++) {
			if (beftype == ST_IF ||
				beftype == ST_ELSEIF ||
				beftype == ST_ELSE ||
				beftype == ST_WHEN) {
				if (it2->type != ST_OPEN) {
					// { �ǉ�
					CStatement	addopen(ST_OPEN, it2->linecount);
					it2 = it->statement.insert(it2, addopen);
					it2 += 2;
					// } �ǉ�
					CStatement	addclose(ST_CLOSE, it2->linecount);
					it2 = it->statement.insert(it2, addclose);
				}
			}
			beftype = it2->type;
		}
	}
			
	return 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::SetCellType
 *  �@�\�T�v�F  �����̍��̎�ʁi���e�����A�֐��A�ϐ�...�Ȃǁj�𒲂ׂĊi�[���Ă����܂�
 *
 *  �Ԓl�@�@�F  1/0=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::SetCellType(void)
{
	int	errorflg = 0;

	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++) {
			// �����ȊO�͔�΂�
			if (it2->type < ST_FORMULA)
				continue;

			for(vector<CCell>::iterator it3 = it2->cell.begin(); it3 != it2->cell.end(); it3++) {
				// ���Z�q�͔�΂�
				if (it3->v.GetType() != F_TAG_NOP)
					continue;

				// ����ʎ擾
				errorflg += SetCellType1(*it3, 0, it->dicfilename, it2->linecount);
				// when�̏ꍇ�A���̓��e�����������蓾�Ȃ�
				if (it2->type == ST_WHEN) {
					if (it3->v.GetType() != F_TAG_INT && 
						it3->v.GetType() != F_TAG_DOUBLE && 
						it3->v.GetType() != F_TAG_STRING && 
						it3->v.GetType() != F_TAG_STRING_PLAIN && 
						it3->v.GetType() != F_TAG_DOUBLE) {
						logger.Error(E_E, 45, it->dicfilename, it2->linecount);
						errorflg++;
					}
				}
			}
		}
	
	return (errorflg) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::SetCellType1
 *  �@�\�T�v�F  �n���ꂽ���̎�ʁi���e�����A�֐��A�ϐ�...�Ȃǁj��ݒ肵�܂�
 *  �����@�@�F  emb 0/1=�ʏ�̐����̍�/������ɖ��ߍ��܂�Ă��������̍�
 *
 *  �Ԓl�@�@�F  1/0=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::SetCellType1(CCell& scell, char emb, const wstring& dicfilename, int linecount)
{
	// �֐�
	int	i = GetFunctionIndexFromName(scell.v.s_value);
	if(i != -1) {
		scell.v.SetType(F_TAG_USERFUNC);
		scell.index     = i;
		scell.v.s_value = L"";
		return 0;
	}

/*
	int i = 0;
	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++, i++)
		if (!scell.v.s_value.compare(it->name)) {
			scell.v.SetType(F_TAG_USERFUNC);
			scell.index     = i;
			scell.v.s_value = L"";
//			wcout << "Aya5:" << endl;
//			wcout << "  result: " << i << endl;
			return 0;
		}
*/

	// �V�X�e���֐�
	for(i = 0; i < SYSFUNC_NUM; i++)
		if (!scell.v.s_value.compare(sysfunc[i])) {
			scell.v.SetType(F_TAG_SYSFUNC);
			scell.index     = i;
			scell.v.s_value = L"";
			return 0;
		}
	// �������e����(DEC)
	if (IsIntString(scell.v.s_value)) {
		scell.v = ws_atoi(scell.v.s_value, 10);
		return 0;
	}
	// �������e����(BIN)
	if (IsIntBinString(scell.v.s_value, 1)) {
		scell.v = ws_atoi(scell.v.s_value, 2);
		return 0;
	}
	// �������e����(HEX)
	if (IsIntHexString(scell.v.s_value, 1)) {
		scell.v = ws_atoi(scell.v.s_value, 16);
		return 0;
	}
	// �������e����
	if (IsDoubleString(scell.v.s_value)) {
		scell.v = ws_atof(scell.v.s_value);
		return 0;
	}
	// �����񃊃e����(�_�u���N�H�[�g)
	i = IsLegalStrLiteral(scell.v.s_value);
	if (!i) {
		CutDoubleQuote(scell.v.s_value);
		if (!emb)
			scell.v.SetType(F_TAG_STRING);
		else {
			if (!scell.v.s_value.size())
				scell.v.SetType(F_TAG_STRING);
			else {
				if (scell.v.s_value[0] == L'%') {
					if (scell.v.s_value.size() == 1)
						scell.v.SetType(F_TAG_STRING);
					else {
						scell.v.s_value.erase(0, 1);
						scell.v.SetType(F_TAG_STRING_EMBED);
					}
				}
				else
					scell.v.SetType(F_TAG_STRING);
			}
		}
		return 0;
	}
	else if (i == 1) {
		scell.v.s_value = L"";
		logger.Error(E_E, 7, scell.v.s_value, dicfilename, linecount);
		return 1;
	}
	else if (i == 2) {
		scell.v.s_value = L"";
		logger.Error(E_E, 8, scell.v.s_value, dicfilename, linecount);
		return 1;
	}
	// �����񃊃e����(�V���O���N�H�[�g)
	i = IsLegalPlainStrLiteral(scell.v.s_value);
	if (!i) {
		CutSingleQuote(scell.v.s_value);
		scell.v.SetType(F_TAG_STRING_PLAIN);
		return 0;
	}
	else if (i == 1) {
		scell.v.s_value = L"";
		logger.Error(E_E, 7, scell.v.s_value, dicfilename, linecount);
		return 1;
	}
	else if (i == 2) {
		scell.v.s_value = L"";
		logger.Error(E_E, 93, scell.v.s_value, dicfilename, linecount);
		return 1;
	}
	// �����܂Ŏc�������͕̂ϐ��̌��
	CVariable	addvariable;
	string	errstr;
	switch(IsLegalVariableName(scell.v.s_value)) {
	case 0:
		// �O���[�o���ϐ�
		scell.v.SetType(F_TAG_VARIABLE);
		scell.index     = variable.Make(scell.v.s_value, 1);
		scell.v.s_value = L"";
		return 0;
	case 16:
		// ���[�J���ϐ�
		scell.v.SetType(F_TAG_LOCALVARIABLE);
		scell.name      = scell.v.s_value;
		scell.v.s_value = L"";
		return 0;
	case 3:
		// �G���[�@�ϐ����̌��
		logger.Error(E_E, 10, scell.v.s_value, dicfilename, linecount);
		scell.v.s_value = L"";
		return 1;
	case 4:
		// �G���[�@�g�p�ł��Ȃ��������܂�
		logger.Error(E_E, 11, scell.v.s_value, dicfilename, linecount);
		scell.v.s_value = L"";
		return 1;
	case 6:
		// �G���[�@�\���ƏՓ�
		logger.Error(E_E, 12, scell.v.s_value, dicfilename, linecount);
		scell.v.s_value = L"";
		return 1;
	default:
		// �����G���[
		logger.Error(E_E, 9, scell.v.s_value, dicfilename, linecount);
		scell.v.s_value = L"";
		return 1;
	};
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::MakeCompleteFormula
 *  �@�\�T�v�F  ���ߍ��ݗv�f�̎��ւ̓W�J�Ɖ��Z�����̌�����s�Ȃ��܂�
 *
 *  �Ԓl�@�@�F  1/0=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::MakeCompleteFormula(void)
{
	int	errcount = 0;

	errcount += ParseEmbeddedFactor();
	ConvertPlainString();
	errcount += CheckDepthAndSerialize();

	return (errcount) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::ParseEmbeddedFactor
 *  �@�\�T�v�F  "%"�Ŗ��ߍ��܂ꂽ�v�f����������𕪉����Đ��������A���̎��ƌ������܂�
 *
 *  �Ԓl�@�@�F  1/0=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::ParseEmbeddedFactor(void)
{
	int	errcount = 0;

	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++)
		    errcount += ParseEmbeddedFactor1(*it2, it->dicfilename);

	return (errcount) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::ParseEmbeddedFactor1
 *  �@�\�T�v�F  "%"�Ŗ��ߍ��܂ꂽ�v�f����������𕪉����Đ��������A���̎��ƌ������܂�
 *
 *  �Ԓl�@�@�F  1/0=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::ParseEmbeddedFactor1(CStatement& st, const wstring& dicfilename)
{
	if (st.type < ST_FORMULA)
		return 0;

	int	errcount = 0;

	// ���Z����������Ȃ��悤�ɂ��邽�߁A%���܂ޗv�f��()�ň͂�
	for(vector<CCell>::iterator it = st.cell.begin(); it != st.cell.end(); ) {
		if (it->v.GetType() == F_TAG_STRING) {
			if (it->v.s_value.find(L'%') != -1) {
				CCell	addcell(F_TAG_BRACKETIN);
				it = st.cell.insert(it, addcell);
				it += 2;
				CCell	addcell2(F_TAG_BRACKETOUT);
				it = st.cell.insert(it, addcell2);
				it++;
				if (st.type == ST_WHEN) {
					logger.Error(E_E, 46, dicfilename, st.linecount);
					errcount++;
				}
				continue;
			}
		}
		// ����
		it++;
	}

	// ���ߍ��ݗv�f�����Z�������ɕ������Č��̎��̊Y���ʒu�֑}��
	for(vector<CCell>::iterator it = st.cell.begin(); it != st.cell.end(); ) {
		if (it->v.GetType() == F_TAG_STRING) {
			if (it->v.s_value.find(L'%') != -1) {
				// ���Z�������֕ϊ�
				int	t_errcount = 0;
				wstring	linedata = it->v.s_value;
				int	res = ConvertEmbedStringToFormula(linedata, dicfilename, st.linecount);
				t_errcount += res;
				if (res) {
					it++;
					continue;
				}
				// �����̍������Ɛ��`
				vector<CCell>	addcells;
				res = 1 - StructFormula(linedata, addcells, dicfilename, st.linecount);
				t_errcount += res;
				if (res) {
					it++;
					continue;
				}
				// ���̎�ʂ�ݒ�
				for(vector<CCell>::iterator it2 = addcells.begin(); it2 != addcells.end(); it2++) {
					if (it2->v.GetType() != F_TAG_NOP)
						continue;

					t_errcount += SetCellType1(*it2, 1, dicfilename, st.linecount);
				}
				// ���̎��̊Y���ʒu�֑}��
				if (!t_errcount) {
					it = st.cell.erase(it);
					int	c_num = addcells.size();
					for(int i = c_num - 1; i >= 0; i--)
						it = st.cell.insert(it, addcells[i]);
					it += c_num;
					continue;
				}
				errcount += t_errcount;
			}
		}
		// ����
		it++;
	}

	return (errcount) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::ConvertPlainString
 *  �@�\�T�v�F  �V���O���N�H�[�g�������ʏ핶����֒u�����܂�
 * -----------------------------------------------------------------------
 */
void	CParser0::ConvertPlainString(void)
{
	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++)
		    ConvertPlainString1(*it2, it->dicfilename);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::ConvertPlainString1
 *  �@�\�T�v�F  �V���O���N�H�[�g�������ʏ핶����֒u�����܂�
 * -----------------------------------------------------------------------
 */
void	CParser0::ConvertPlainString1(CStatement& st, const wstring& dicfilename)
{
	if (st.type < ST_FORMULA)
		return;

	for(vector<CCell>::iterator it = st.cell.begin(); it != st.cell.end(); it++)
		if (it->v.GetType() == F_TAG_STRING_PLAIN)
			it->v.SetType(F_TAG_STRING);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::ConvertEmbedStringToFormula
 *  �@�\�T�v�F  "%"�Ŗ��ߍ��܂ꂽ�v�f��������������Z�������֕ϊ����܂�
 *
 *  �Ԓl�@�@�F  1/0=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::ConvertEmbedStringToFormula(wstring& str, const wstring& dicfilename, int linecount)
{
	wstring	resstr;
	int	nindex = -1;
	for(int nfirst = 0; ; nfirst++) {
		// "%"�̔���
		int	p_pers = str.find(L'%', 0);
		if (p_pers == -1) {
			logger.Error(E_E, 55, dicfilename, linecount);
			return 1;
		}
		// ���Z���Z�q��ǉ�
		if (nfirst)
			resstr += L"+";
		// ��s���镶���񍀂�ǉ�
		if (p_pers > 0) {
			wstring	prestr;
			prestr.assign(str, 0, p_pers);
			AddDoubleQuote(prestr);
			resstr += prestr;
			str.erase(0, p_pers);
			resstr += L"+";
		}
		// "%"�����c��Ȃ������炻��ŏI���
		if (str.size() == 1) {
			resstr += L"\"%\"";
			break;
		}
		// "%"�̎���"("�Ȃ璷���w��t���̖��ߍ��݂Ȃ̂ł���𔲂��o��
		if (str[1] == L'(') {
			// �����o���ʒu����
			int	bdepth = 1;
			int	len = str.size();
			int     spos;
			for(spos = 2; spos < len; spos++) {
				bdepth += ((str[spos] == L'(') - (str[spos] == L')'));
				if (!bdepth)
					break;
			}
			if (spos < len)
				spos++;
			// �G���[����
			if (bdepth != 0) {
				logger.Error(E_E, 60, dicfilename, linecount);
				return 1;
			}
			if (spos == 2) {
				logger.Error(E_E, 61, dicfilename, linecount);
				return 1;
			}
			else if (spos < 2) {
				logger.Error(E_E, 62, dicfilename, linecount);
				return 1;
			}
			// ���ߍ��ݗv�f�����o���Ēǉ�
			wstring	embedstr;
			embedstr.assign(str, 1, spos - 1);
			resstr += L"TOSTR";
			resstr += embedstr;
			str.erase(0, spos);
//			nindex++;
			// ����"%"��T���Ă݂�
			p_pers = str.find(L'%', 0);
			// ������Ȃ���΂��ꂪ�Ō�̕�����萔��
			if (p_pers == -1) {
				embedstr = str;
				if (embedstr.size()) {
					AddDoubleQuote(embedstr);
					resstr += L"+";
					resstr += embedstr;
				}
				break;
			}
			continue;
		}
		// "%"�̎���"["�Ȃ猋�ʂ̍ė��p�i%[n]�j�Ȃ̂ł���𔲂��o��
		if (str[1] == L'[') {
			// �܂���s���鍀�������Ȃ�G���[
			if (nindex == -1) {
				logger.Error(E_E, 81, dicfilename, linecount);
				return 1;
			}
			// �����o���ʒu����
			int	bdepth = 1;
			int	len = str.size();
			int     spos;
			for(spos = 2; spos < len; spos++) {
				bdepth += ((str[spos] == L'[') - (str[spos] == L']'));
				if (!bdepth)
					break;
			}
			if (spos < len)
				spos++;
			// �G���[����
			if (bdepth != 0) {
				logger.Error(E_E, 78, dicfilename, linecount);
				return 1;
			}
			if (spos == 2) {
				logger.Error(E_E, 79, dicfilename, linecount);
				return 1;
			}
			else if (spos < 2) {
				logger.Error(E_E, 80, dicfilename, linecount);
				return 1;
			}
			// ���ߍ��ݗv�f�����o���A"���ʂ̍ė��p�������s���֐�"�Ƃ��Ēǉ�
			resstr += sysfunc[SYSFUNC_HIS];
			resstr += L"(";
			wstring	embedstr;
			ws_itoa(embedstr, nindex, 10);
			resstr += embedstr;
			resstr += L"-(";
			embedstr.assign(str, 2, spos - 3);
			resstr += embedstr;
			resstr += L"))";
			str.erase(0, spos);
			// ����"%"��T���Ă݂�
			p_pers = str.find(L'%', 0);
			// ������Ȃ���΂��ꂪ�Ō�̕�����萔��
			if (p_pers == -1) {
				embedstr = str;
				if (embedstr.size()) {
					AddDoubleQuote(embedstr);
					resstr += L"+";
					resstr += embedstr;
				}
				break;
			}
			continue;
		}
		// �ʏ��"%"
		else {
			// ����"%"�𔭌�
			p_pers = str.find(L'%', 1);
			nindex++;
			// ������Ȃ���΂��ꂪ�Ō�̍�
			if (p_pers == -1) {
				wstring	embedstr = str;
				AddDoubleQuote(embedstr);
				resstr += embedstr;
				break;
			}
			// ���������̂Œǉ�
			wstring	embedstr;
			embedstr.assign(str, 0, p_pers);
			AddDoubleQuote(embedstr);
			resstr += embedstr;
			str.erase(0, p_pers);
		}
	}

	// �Ԃ�
	str = resstr;
	return 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::CheckDepthAndSerialize
 *  �@�\�T�v�F  �����̃J�b�R�����������Ă��邩�����A����ю��̉��Z���������߂܂�
 *
 *  �Ԓl�@�@�F  1/0=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::CheckDepthAndSerialize(void)
{
	int	errcount = 0;

	// �����̃J�b�R����
	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++) {
			if (it2->type < ST_FORMULA)
				continue;

			errcount += CheckDepth1(*it2, it->dicfilename);
		}

	// when��if�ϊ��̍ŏI�����@���̐�����if�ŏ����\�Ȕ��莮�ɐ��`����
	errcount += MakeCompleteConvertionWhenToIf();

	// ���Z�����̌���
	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++) {
			if (it2->type < ST_FORMULA)
				continue;

			errcount += CheckDepthAndSerialize1(*it2, it->dicfilename);
		}

	return (errcount) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::MakeCompleteConvertionWhenToIf
 *  �@�\�T�v�F  when��if�ϊ��̍ŏI�������s���܂��B���̐�����if�ŏ����\�Ȕ��莮�ɐ��`���܂�
 *
 *  �Ԓl�@�@�F  1/0=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::MakeCompleteConvertionWhenToIf(void)
{
	int	errcount = 0;

	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++) {
		vector<wstring>	caseary;
		wstring	dmystr = L"";
		caseary.push_back(dmystr);
		vector<int>		whencnt;
		whencnt.push_back(0);
		int	depth = 0;
		for(vector<CStatement>::iterator it2 = it->statement.begin(); it2 != it->statement.end(); it2++) {
			if (depth == -1) {
				logger.Error(E_E, 52, it->dicfilename, it2->linecount);
				errcount++;
				break;
			}
			// {
			if (it2->type == ST_OPEN) {
				depth++;
				wstring	dmystr = L"";
				caseary.push_back(dmystr);
				whencnt.push_back(0);
				continue;
			}
			// }
			if (it2->type == ST_CLOSE) {
				caseary[depth] = L"";
				whencnt[depth] = 0;
				depth--;
				continue;
			}
			// case
			if (it2->type == ST_FORMULA) {
				if (it2->cell.size() >= 2) {
					if (it2->cell[0].v.GetType() == F_TAG_LOCALVARIABLE &&
						it2->cell[1].v.GetType() == F_TAG_EQUAL) {
						if (!::wcscmp(PREFIX_CASE_VAR,
							it2->cell[0].name.substr(0, PREFIX_CASE_VAR_SIZE).c_str())) {
							caseary[depth] = it2->cell[0].name;
							whencnt[depth] = 0;
							continue;
						}
					}
				}
			}
			// when
			if (it2->type == ST_WHEN) {
				int	depthm1 = depth - 1;
				if (depthm1 < 0) {
					logger.Error(E_E, 64, it->dicfilename, it2->linecount);
					errcount++;
					break;
				}
				if (!caseary[depthm1].size()) {
					logger.Error(E_E, 63, it->dicfilename, it2->linecount);
					errcount++;
					break;
				}
				// if/elseif�֕ϊ�
				if (!whencnt[depthm1])
					it2->type = ST_IF;
				else
					it2->type = ST_ELSEIF;
				(whencnt[depthm1])++;
				// ���̐����𔻒莮�ɏ���������
				int	i = 0;
				for(vector<CCell>::iterator it3 = it2->cell.begin(); it3 != it2->cell.end(); ) {
					// ���x��
					if (!i) {
						if (it3->v.GetType() != F_TAG_INT && 
							it3->v.GetType() != F_TAG_DOUBLE && 
							it3->v.GetType() != F_TAG_STRING && 
							it3->v.GetType() != F_TAG_DOUBLE) {
							logger.Error(E_E, 53, it->dicfilename, it2->linecount);
							errcount++;
							break;
						}
						i = 1;
						it3++;
						// �Ō�̍��@����͕K��==����
						if (it3 == it2->cell.end()) {
							CCell	addcell1(F_TAG_LOCALVARIABLE);
							addcell1.name    = caseary[depthm1];
							addcell1.v.s_value = L"";
							it3 = it2->cell.insert(it3, addcell1);
							CCell	addcell2(F_TAG_IFEQUAL);
							it3 = it2->cell.insert(it3, addcell2);
							break;
						}
						continue;
					}
					// or/and
					i = 0;
					if (it3->v.GetType() == F_TAG_OR) {
						// or
						CCell	addcell1(F_TAG_LOCALVARIABLE);
						addcell1.name      = caseary[depthm1];
						addcell1.v.s_value = L"";
						it3 = it2->cell.insert(it3, addcell1);
						CCell	addcell2(F_TAG_IFEQUAL);
						it3 = it2->cell.insert(it3, addcell2);
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
					}
					else if (it3->v.GetType() == F_TAG_AND) {
						// and
						CCell	addcell1(F_TAG_LOCALVARIABLE);
						addcell1.name      = caseary[depthm1];
						addcell1.v.s_value = L"";
						it3 = it2->cell.insert(it3, addcell1);
						CCell	addcell2(F_TAG_IFLTEQUAL);
						it3 = it2->cell.insert(it3, addcell2);
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						CCell	addcell3(F_TAG_IFLTEQUAL);
						it3 = it2->cell.insert(it3, addcell3);
						CCell	addcell4(F_TAG_LOCALVARIABLE);
						addcell4.name      = caseary[depthm1];
						addcell4.v.s_value = L"";
						it3 = it2->cell.insert(it3, addcell4);
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
						if (it3 == it2->cell.end())
							break;
						it3++;
					}
					else {
						logger.Error(E_E, 54, it->dicfilename, it2->linecount);
						errcount++;
						break;
					}
				}
			}
		}
	}

	return (errcount) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::CheckDepth1
 *  �@�\�T�v�F  �����̃J�b�R�����������Ă��邩�������܂�
 *
 *  �Ԓl�@�@�F  1/0=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::CheckDepth1(CStatement& st, const wstring& dicfilename)
{
	// ()/[]�̑Ή��Â�������
	vector<int>	hb_depth;
	int	depth = 0;
	for(vector<CCell>::iterator it = st.cell.begin(); it != st.cell.end(); it++) {
		if (it->v.GetType() == F_TAG_BRACKETIN)
			depth++;
		else if (it->v.GetType() == F_TAG_BRACKETOUT)
			depth--;
		else if (it->v.GetType() == F_TAG_HOOKBRACKETIN)
			hb_depth.push_back(depth);
		else if (it->v.GetType() == F_TAG_HOOKBRACKETOUT) {
			int	gb_depth_size = hb_depth.size();
			if (!gb_depth_size) {
				logger.Error(E_E, 20, dicfilename, st.linecount);
				return 1;
			}
			else if (hb_depth[gb_depth_size - 1] != depth) {
				logger.Error(E_E, 20, dicfilename, st.linecount);
				return 1;
			}
			hb_depth.erase(hb_depth.end() - 1);
		}
	}
	if (depth) {
		logger.Error(E_E, 19, dicfilename, st.linecount);
		return 1;
	}

	return 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CParser0::CheckDepthAndSerialize1
 *  �@�\�T�v�F  �����̉��Z���������肵�܂�
 *
 *  �Ԓl�@�@�F  1/0=�G���[/����
 * -----------------------------------------------------------------------
 */
char	CParser0::CheckDepthAndSerialize1(CStatement& st, const wstring& dicfilename)
{
	// �ēx()����q�̑Ή����������A���Z�����Z�o�p�̃t���O���쐬����
	// �t���Odepthvec�͑Ή����鍀�̏�����Ԃ������Ă���B
	// -1    �����ς������͏����Ώۂł͂Ȃ���
	// -2    �����҂��̍�
	// 0�ȏ� ���Z�q�A�������傫���قǗD��x������

	int	sz = st.cell.size();
	vector<int>	depthvec;
	int depth = 0;
	int i;
	for(i = 0; i < sz; i++) {
		// ���Z�q
		int	type = st.cell[i].v.GetType();
		if (type >= F_TAG_ORIGIN && type < F_TAG_ORIGIN_VALUE) {
			if (type == F_TAG_BRACKETIN ||
				type == F_TAG_BRACKETOUT ||
				type == F_TAG_HOOKBRACKETIN ||
				type == F_TAG_HOOKBRACKETOUT) {
				depth += formulatag_depth[type];
				depthvec.push_back(-1);
			}
			else
				depthvec.push_back(depth + formulatag_depth[type]);
			continue;
		}
		// ���Z�q�ȊO
		depthvec.push_back(-2);
	}
	if (depth) {
		logger.Error(E_E, 48, dicfilename, st.linecount);
		return 1;
	}
	if (sz != depthvec.size()) {
		logger.Error(E_E, 21, dicfilename, st.linecount);
		return 1;
	}

	// ���Z�����̌���
	for( ; ; ) {
		// ���Z�Ώۂ̉��Z�q�𔭌�
		// ����[���̉��Z�q�̕���͍ŏ��̂��̂��I�΂��B�܂�AYA5�ł͉��Z�q�͏�ɍ�����E�֌��������
		// ���������� i=j=1 �� i=j; j=1 �Ɠ����ł���Bi �� 1 �͑������Ȃ��B�E���猋�������C/C++�Ƃ�
		// �����͈قȂ��Ă���B
		int	t_index = -1;
		int	t_depth = -1;
		for(i = 0; i < sz; i++)
			if (depthvec[i] > t_depth) {
				t_depth = depthvec[i];
				t_index = i;
			}
		// �Ώۂ������Ȃ����甲����
		if (t_depth == -1)
			break;

		// ��`�̊J�n�@���Z�q�̓o�^
		int	t_type = st.cell[t_index].v.GetType();
		CSerial	addserial(t_index);
		depthvec[t_index] = -2;
		// ���ӂ̍����擾
		int	f_depth = 1;
		for(i = t_index - 1; i >= 0; i--) {
			// �J�b�R�[������
			if (st.cell[i].v.GetType() == F_TAG_BRACKETIN ||
				st.cell[i].v.GetType() == F_TAG_HOOKBRACKETIN)
				f_depth--;
			else if (st.cell[i].v.GetType() == F_TAG_BRACKETOUT ||
				st.cell[i].v.GetType() == F_TAG_HOOKBRACKETOUT)
				f_depth++;
			if (!f_depth) {
				i = -1;
				break;
			}
			// �擾
			if (depthvec[i] == -2) {
				addserial.index.push_back(i);
				depthvec[i] = -1;
				break;
			}
		}
		if (i == -1) {
			if (t_type == F_TAG_CUMMA)
				logger.Error(E_E, 23, dicfilename, st.linecount);
			else
				logger.Error(E_E, 22, dicfilename, st.linecount);
			return 1;
		}
		// ���Z�q��","�̏ꍇ�A�����̂���ɍ��֌�����i�߁A�����������֐��ɓn�����߂̉��Z�q�Ɗ֐���
		// ���������ꍇ�͈������֐������ɏ���������B
		// �֐���������Ȃ��ꍇ�͒ʏ�̔z��Ƃ������ƂɂȂ�
		if (t_type == F_TAG_CUMMA) {
			f_depth = 1;
			for( ; i >= 0; i--) {
				// �J�b�R�[������
				if (st.cell[i].v.GetType() == F_TAG_BRACKETIN ||
					st.cell[i].v.GetType() == F_TAG_HOOKBRACKETIN)
					f_depth--;
				else if (st.cell[i].v.GetType() == F_TAG_BRACKETOUT ||
					st.cell[i].v.GetType() == F_TAG_HOOKBRACKETOUT)
					f_depth++;
				if (!f_depth) {
					i--;
					break;
				}
			}
			if (i > 0) {
				if (st.cell[i].v.GetType() == F_TAG_FUNCPARAM) {
					// �֐�
					depthvec[t_index] = -1;
					addserial.tindex = i;
					depthvec[i] = -2;
					i--;
					if (i < 0) {
						logger.Error(E_E, 25, dicfilename, st.linecount);
						return 1;
					}
					if ((st.cell[i].v.GetType() == F_TAG_SYSFUNC ||
						st.cell[i].v.GetType() == F_TAG_USERFUNC) &&
						depthvec[i] == -2) {
						addserial.index.insert(addserial.index.begin(), i);
						depthvec[i] = -1;
					}
				}
			}
		}
		// �E�ӂ̍����擾�@���Z�q��","�̏ꍇ�͗񋓂��ꂽ���ׂĂ��ꊇ���Ď擾����
		if (t_type == F_TAG_CUMMA) {
			// ","
			int	gflg = 0;
			f_depth = 1;
			for(i = t_index + 1; i < sz; i++) {
				// �J�b�R�[������
				if (st.cell[i].v.GetType() == F_TAG_BRACKETIN ||
					st.cell[i].v.GetType() == F_TAG_HOOKBRACKETIN)
					f_depth++;
				else if (st.cell[i].v.GetType() == F_TAG_BRACKETOUT ||
					st.cell[i].v.GetType() == F_TAG_HOOKBRACKETOUT)
					f_depth--;
				if (!f_depth)
					break;
				// �擾
				if (depthvec[i] == -2) {
					addserial.index.push_back(i);
					depthvec[i] = -1;
					gflg = 1;
					continue;
				}
				else if (depthvec[i] == -1)
					continue;
				else if (st.cell[i].v.GetType() == F_TAG_CUMMA) {
					depthvec[i] = -1;
					continue;
				}

				break;
			}
			if (!gflg) {
				logger.Error(E_E, 24, dicfilename, st.linecount);
				return 1;
			}
		}
		else {
			// ","�ȊO
			for(i = t_index + 1; i < sz; i++)
				if (depthvec[i] == -2) {
					addserial.index.push_back(i);
					depthvec[i] = -1;
					break;
				}
			if (i == sz) {
				logger.Error(E_E, 22, dicfilename, st.linecount);
				return 1;
			}
		}
		// ���Z��`��o�^
		st.serial.push_back(addserial);
	}

	// ���Z���������肷��ƁA�����������ЂƂ����c�邱�ƂɂȂ�i���ꂪ���ʁj�B������m�F����
	//
	// �����������������Z�q�łȂ��ꍇ�́A�L���ȍ���1�����Ȃ����߉��Z�������������Ƃ��Ӗ����Ă���B
	// ���̂܂܂ł͌��ʂ������Ȃ��̂ŁA�u�c���������猋�ʂ𓾂�v���Ƃ��w������t���O��ǉ�����
	int	scount = 0;
	for(i = 0; i < sz; i++)
		if (depthvec[i] == -2) {
			scount++;
			if (st.cell[i].v.GetType() >= F_TAG_ORIGIN_VALUE) {
				CSerial	addserial(i);
				addserial.index.push_back(0);	// dmy
				addserial.index.push_back(0);	// dmy
				st.serial.push_back(addserial);
			}
		}
	if (scount != 1) {
		logger.Error(E_E, 86, dicfilename, st.linecount);
		return 1;
	}

	return 0;
}

