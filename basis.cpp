// 
// AYA version 5
//
// ��Ȑ�����s�Ȃ��N���X�@CBasis
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#else
#  include "messages.h"
#  include "posix_utils.h"
#  include <stdlib.h>
#  include <dirent.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <iostream>
using namespace std;
#endif
#include "basis.h"
#include "parser0.h"
#include "logexcode.h"
#include "comment.h"
#include "misc.h"
#include "log.h"
#include "wsex.h"
#include "ccct.h"
#include "file.h"
#include "lib.h"
#include "wordmatch.h"

extern vector<CFunction>	function;
extern CCallDepth			calldepth;
extern CGlobalVariable		variable;
extern CFile				files;
extern CLib					libs;
extern CLog					logger;


extern CWordMatch			formulatag_wm;


/* -----------------------------------------------------------------------
 * CBasis�R���X�g���N�^
 * -----------------------------------------------------------------------
 */
CBasis::CBasis(void)
{
	ResetSurpress();
	CreateWordMatch();

	checkparser = 0;
	iolog       = 1;
	msglang     = MSGLANG_JAPANESE;
	charset     = CHARSET_SJIS;
#ifndef POSIX
	hlogrcvWnd  = NULL;
#endif
	run         = 0;
}

void CBasis::CreateWordMatch(void)
{
	for(int i = 0; i < FORMULATAG_NUM; i++)
		formulatag_wm.addWord(formulatag[i], i);
}


/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::SetModuleHandle
 *  �@�\�T�v�F  ���W���[���n���h�����擾���܂�
 *
 *  ���łɃ��W���[���̎�t�@�C�����擾���s���܂�
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
void	CBasis::SetModuleHandle(HANDLE h)
{
	// ���W���[���n���h�����擾
	hmodule = (HMODULE)h;

	// ���W���[���̎�t�@�C�������擾
	// NT�n�ł͂����Ȃ�UNICODE�Ŏ擾�ł��邪�A9x�n���l������MBCS�Ŏ擾���Ă���UCS-2�֕ϊ�
	char	path[STRMAX];
	GetModuleFileName(hmodule, path, sizeof(path));
	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_splitpath(path, drive, dir, fname, ext);
	string	mbmodulename = fname;

	wchar_t	*wcmodulename = Ccct::MbcsToUcs2(mbmodulename, CHARSET_DEFAULT);
	modulename = wcmodulename;
	free(wcmodulename);
}
#endif

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::SetPath
 *  �@�\�T�v�F  HGLOBAL�Ɋi�[���ꂽ�t�@�C���p�X���擾���AHGLOBAL�͊J�����܂�
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
void	CBasis::SetPath(HGLOBAL h, int len)
{
	// �擾�Ɨ̈�J��
	string	mbpath;
	mbpath.assign((char *)h, 0, len);
	GlobalFree(h);
	// �����R�[�h��UCS-2�֕ϊ��i�����ł̃}���`�o�C�g�����R�[�h��OS�f�t�H���g�j
	wchar_t	*wcpath = Ccct::MbcsToUcs2(mbpath, CHARSET_DEFAULT);
	path = wcpath;
	free(wcpath);
}
#else
void CBasis::SetPath(char* h, long len) {
    // �擾�Ɨ̈�J��
    path = widen(string(h, static_cast<string::size_type>(len)));
    free(h);
    // �X���b�V���ŏI����ĂȂ���Εt����B
    if (path.length() == 0 || path[path.length() - 1] != L'/') {
	path += L'/';
    }
    // ���W���[���n���h���̎擾�͏o���Ȃ��̂ŁA�͋Z�ňʒu��m��B
    // ���̃f�B���N�g���ɂ���S�Ă�*.dll(case insensitive)��T���A
    // ���g��aya5.dll�Ƃ�����������܂�ł�����A�����I�ԁB
    // �������Ή�����*.txt���������dll�̒��g�͌����Ɏ��֍s���B
    modulename = L"aya5";
    DIR* dh = opendir(narrow(path).c_str());
    if (dh == NULL) {
	cerr << narrow(path) << "is not a directory!" << endl;
	exit(1);
    }
    while (true) {
	struct dirent* ent = readdir(dh);
	if (ent == NULL) {
	    break; // ��������
	}
	string fname(ent->d_name, strlen(ent->d_name)/*ent->d_namlen*/);	// by umeici. 2005/1/16 5.6.0.232
	if (lc(get_extension(fname)) == "dll") {
	    string txt_file = narrow(path) + change_extension(fname, "txt");
	    struct stat sb;
	    if (::stat(txt_file.c_str(), &sb) == 0) {
		// txt�t�@�C��������̂ŁA���g�����Ă݂�B
		if (file_content_search(narrow(path) + fname, "aya5.dll") != string::npos) {
		    // ����͕���DLL�ł���B
		    modulename = widen(drop_extension(fname));
		    break;
		}
	    }
	}
    }
    closedir(dh);
}
#endif

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::SetLogRcvWnd
 *  �@�\�T�v�F  �`�F�b�N�c�[������n���ꂽhWnd��ێ����܂�
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
void	CBasis::SetLogRcvWnd(long hwnd)
{
	hlogrcvWnd = (HWND)hwnd;
}
#endif

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::Configure
 *  �@�\�T�v�F  load���ɍs�������ݒ菈��
 * -----------------------------------------------------------------------
 */
void	CBasis::Configure(void)
{
	// ��b�ݒ�t�@�C���i�Ⴆ��aya.txt�j��ǂݎ��
	vector<wstring>	dics;
	LoadBaseConfigureFile(&dics);
	// ��b�ݒ�t�@�C���ǂݎ��ŏd�ĂȃG���[�����������ꍇ�͂����ŏI��
	if (surpress)
		return;

	// ���M���O���J�n
#ifndef POSIX
	logger.Start(logpath, charset, msglang, hlogrcvWnd, iolog);
#else
	logger.Start(logpath, charset, msglang, iolog);
#endif

	// �����ǂݍ��݂ƍ\�����
	if (CParser0::Parse(charset, dics, loadindex, unloadindex, requestindex))
		SetSurpress();

	if (checkparser)
		CLogExCode::OutExecutionCodeForCheck();

	// �O��I�����ɕۑ������ϐ��𕜌�
	RestoreVariable();

	if (checkparser)
		CLogExCode::OutVariableInfoForCheck();

	// �����܂ł̏����ŏd�ĂȃG���[�����������ꍇ�͂����ŏI��
	if (surpress)
		return;

	// �O�����C�u�����ƃt�@�C���̕����R�[�h��������
	libs.SetCharset(charset);
	files.SetCharset(charset);

	// load�֐������s
	ExecuteLoad();

	run = 1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::Termination
 *  �@�\�T�v�F  unload���ɍs���I������
 * -----------------------------------------------------------------------
 */
void	CBasis::Termination(void)
{
	// ����}�~����Ă��Ȃ���ΏI�����̏��������s
	if (!surpress) {
		// unload
		ExecuteUnload();
		// ���[�h���Ă��邷�ׂẴ��C�u������unload
		libs.DeleteAll();
		// �J���Ă��邷�ׂẴt�@�C�������
		files.DeleteAll();
		// �ϐ��̕ۑ�
		SaveVariable();
	}

	// ���M���O���I��
	logger.Termination();
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::IsSurpress
 *  �@�\�T�v�F  ���݂̎����}�~��Ԃ�Ԃ��܂�
 *
 *  �Ԓl�@�@�F  0/1=��}�~/�}�~
 *
 *  ��b�ݒ�t�@�C���̓ǂݎ��⎫���t�@�C���̉�͒��ɁA����p������ȃG���[�����������
 *  SetSurpress()�ɂ���ė}�~�ݒ肳��܂��B�}�~�ݒ肳���ƁAload/request/unload�ł̓��삪
 *  ���ׂă}�X�N����܂��B���̎��Arequest�̕Ԓl�͏�ɋ󕶎���ɂȂ�܂��B�iHGLOBAL=NULL�A
 *  len=0�ŉ������܂��j
 * -----------------------------------------------------------------------
 */
char	CBasis::IsSurpress(void)
{
	return surpress;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::SetSurpress
 *  �@�\�T�v�F  ��������}�~��ݒ肵�܂�
 * -----------------------------------------------------------------------
 */
void	CBasis::SetSurpress(void)
{
	surpress = 1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::ResetSurpress
 *  �@�\�T�v�F  ��������}�~�@�\�����Z�b�g���܂�
 * -----------------------------------------------------------------------
 */
void	CBasis::ResetSurpress(void)
{
	surpress = 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::LoadBaseConfigureFile
 *  �@�\�T�v�F  ��b�ݒ�t�@�C����ǂݎ��A�e��p�����[�^���Z�b�g���܂�
 *
 *  ��b�ݒ�t�@�C����DLL�Ɠ��K�w�ɑ��݂��閼�O��"DLL��t�@�C����.txt"�̃t�@�C���ł��B
 *
 *  �����t�@�C���̕����R�[�h��Shift_JIS�ȊO�ɂ�UTF-8��OS�f�t�H���g�̃R�[�h�ɑΉ��ł��܂����A
 *  ���̊�b�ݒ�t�@�C����OS�f�t�H���g�̃R�[�h�œǂݎ���邱�Ƃɒ��ӂ��Ă��������B
 *  ���ۉ��Ɋւ��čl������ꍇ�́A���̃t�@�C�����̋L�q�Ƀ}���`�o�C�g�������g�p����ׂ��ł�
 *  ����܂���i�����R�[�h0x7F�ȉ���ASCII�����݂̂ŋL�q���ׂ��ł��j�B
 * -----------------------------------------------------------------------
 */
void	CBasis::LoadBaseConfigureFile(vector<wstring> *dics)
{
	// �ݒ�t�@�C��("name".txt)�ǂݎ��

	// �t�@�C�����J��
    	wstring	filename = path + modulename + L".txt";
	FILE	*fp = w_fopen((wchar_t *)filename.c_str(), L"r");
	if (fp == NULL) {
		SetSurpress();
		return;
	}

	// �ǂݎ�菈��
	CComment	comment;
	for (int i = 1; ; i++) {
		// 1�s�ǂݍ���
		wstring	readline;
		if (ws_fgets(readline, fp, CHARSET_DEFAULT, 0, i) == WS_EOF)
			break;
		// ���s�͏���
		CutCrLf(readline);
		// �R�����g�A�E�g����
		comment.ExecuteSS_Top(readline);
		comment.Execute(readline);
		comment.ExecuteSS_Tail(readline);
		// ��s�A�������͑S�̂��R�����g�s�������ꍇ�͎��̍s��
		if (readline.size() == 0)
			continue;
		// �p�����[�^��ݒ�
		wstring	cmd, param;
		if (Split(readline, cmd, param, L",")) {
			SetParameter(cmd, param, dics);
		}
		else {
			logger.Error(E_W, 0, filename, i);
		}
	}

	// �t�@�C�������
	fclose(fp);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::SetParameter
 *  �@�\�T�v�F  LoadBaseConfigureFile����Ă΂�܂��B�e��p�����[�^��ݒ肵�܂�
 * -----------------------------------------------------------------------
 */
void	CBasis::SetParameter(wstring &cmd, wstring &param, vector<wstring> *dics)
{
	// dic
	if (!cmd.compare(L"dic")) {
		wstring	filename = path + param;
		dics->push_back(filename);
	}
	// log
	else if (!cmd.compare(L"log")) {
		logpath = path + param;
	}
	// iolog
	else if (!cmd.compare(L"iolog")) {
		if (!param.compare(L"off"))
			iolog = 0;
	}
	// msglang
	else if (!cmd.compare(L"msglang")) {
		if (!param.compare(L"english"))
			msglang = MSGLANG_ENGLISH;
		else
			msglang = MSGLANG_JAPANESE;
	}
	// charset
	else if (!cmd.compare(L"charset")) {
		if (!param.compare(L"UTF-8") || !param.compare(L"UTF8"))
			charset = CHARSET_UTF8;
		else if (!param.compare(L"default"))
			charset = CHARSET_DEFAULT;
		else if (!param.compare(L"Shift_JIS") || !param.compare(L"ShiftJIS") || !param.compare(L"SJIS"))
			charset = CHARSET_SJIS;
		else {
			logger.Error(E_N, 1, param);
			charset = CHARSET_DEFAULT;
		}
	}
	// fncdepth
	else if (!cmd.compare(L"fncdepth")) {
		int	f_depth = ws_atoi(param, 10);
		calldepth.SetMaxDepth((f_depth < 2) ? 2 : f_depth);
	}
	// checkparser�@������J�@�\
	
	else if (!cmd.compare(L"checkparser")) {
		if (!param.compare(L"on"))
			checkparser = 1;
	}
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::SaveVariable
 *  �@�\�T�v�F  �ϐ��l���t�@�C���ɕۑ����܂�
 *
 *  �t�@�C������"DLL��t�@�C����_variable.cfg"�ł��B
 *  �t�@�C���t�H�[�}�b�g��1�s1�ϐ��A�f���~�^���p�J���}�ŁA
 *
 *  �ϐ���,���e,�f���~�^
 *
 *  �̌`���ŕۑ�����܂��B���e�͐���/�����̏ꍇ�͂��̂܂܁A������ł̓_�u���N�H�[�g����܂��B
 *  �z��̏ꍇ�͊e�v�f�Ԃ��R�����ŕ�������܂��B�ȉ��ɗv�f��3�A�f���~�^"@"�ł̗�������܂��B
 *
 *  var,1:"TEST":0.3,@
 *
 *  �f���~�^�̓_�u���N�H�[�g����܂���B
 *
 *  ��b�ݒ�t�@�C���Őݒ肵�������R�[�h�ŕۑ�����܂��B
 * -----------------------------------------------------------------------
 */
void	CBasis::SaveVariable(void)
{
	// �ϐ��̕ۑ�
	std::string old_locale = setlocale(LC_NUMERIC,NULL);
	setlocale(LC_NUMERIC,"English"); //�����_�����

	// �t�@�C�����J��
	wstring	filename = path + modulename + L"_variable.cfg";
	logger.Message(7);
	logger.Filename(filename);
	FILE	*fp = w_fopen((wchar_t *)filename.c_str(), L"w");
	if (fp == NULL) {
		logger.Error(E_E, 57, filename);
		return;
	}
/*
#ifndef POSIX
	// UTF-8�̏ꍇ�͐擪��BOM��ۑ�
	if (charset == CHARSET_UTF8)
		write_utf8bom(fp);
#endif
	// UTF-8�Ȃ̂�BOM��t����̂͂�߂������X�������Ɓc
	// �g���u���̌����ɂȂ�̂ŁB

	// �����ł��B�O���Ă��܂��܂��B
	// ����
	// UTF-8�ɂ̓o�C�g�I�[�_�[�ɂ��o���G�[�V���������݂��Ȃ��̂�BOM�͕K�v�Ȃ��B
	// �t�^���邱�Ƃ͏o����B�������Ή����Ă��Ȃ��\�t�g�œǂ߂Ȃ��Ȃ�̂ŕt���Ȃ��ق���
	// �ǂ��B
*/

	// �����ۑ�
	wstring	wstr;
	int	c_ch;
	char	*tmpstr;
	int	var_num = variable.GetNumber();
	for(int i = 0; i < var_num; i++) {
		CVariable	*var = variable.GetPtr(i);
		// ���e���󕶎���̕ϐ��͕ۑ����Ȃ�
		if (var->v.GetType() == F_TAG_STRING && !var->v.s_value.size())
			continue;
		// ���e����ėp�z��̕ϐ��͕ۑ����Ȃ�
//		if (var->v.GetType() == F_TAG_ARRAY && !var->v.array.size())
//			continue;
		// �����t���O�������Ă���ϐ��͕ۑ����Ȃ�
		if (var->IsErased())
			continue;
		// ���O�̕ۑ�
		tmpstr = Ccct::Ucs2ToMbcs(var->name, charset);
		fprintf(fp, "%s,", tmpstr);
		free(tmpstr);
		// �l�̕ۑ�
		vector<CValueSub>::iterator	itv;
		switch(var->v.GetType()) {
		case F_TAG_INT:
			ws_itoa(wstr, var->v.i_value, 10);
			tmpstr = Ccct::Ucs2ToMbcs(wstr, charset);
			fprintf(fp, "%s,", tmpstr);
			free(tmpstr);
			break;
		case F_TAG_DOUBLE:
			ws_ftoa(wstr, var->v.d_value);
			tmpstr = Ccct::Ucs2ToMbcs(wstr, charset);
			fprintf(fp, "%s,", tmpstr);
			free(tmpstr);
			break;
		case F_TAG_STRING:
			wstr = var->v.s_value;
			ws_replace(wstr, L"\"", wstring(ESC_DQ));
			for(c_ch = 1; c_ch <= END_OF_CTRL_CH; c_ch++)
				ws_replace(wstr, wstring() + (wchar_t)c_ch,
					wstring(ESC_CTRL) + (wchar_t)(c_ch + CTRL_CH_START));
			wstr = L"\"" + wstr + L"\"";
			tmpstr = Ccct::Ucs2ToMbcs(wstr, charset);
			fprintf(fp, "%s,", tmpstr);
			free(tmpstr);
			break;
		case F_TAG_ARRAY:
			if (!var->v.array.size()) {
				fprintf(fp, ESC_IARRAY_SAVE);
				fprintf(fp, ":");
				fprintf(fp, ESC_IARRAY_SAVE);
			}
			else {
				for(itv = var->v.array.begin(); itv != var->v.array.end(); itv++) {
					if (itv != var->v.array.begin())
						fprintf(fp, ":");
					wstr = itv->GetValueString();
					ws_replace(wstr, L"\"", wstring(ESC_DQ));
					for(c_ch = 1; c_ch <= END_OF_CTRL_CH; c_ch++)
						ws_replace(wstr, wstring() + (wchar_t)c_ch,
							wstring(ESC_CTRL) + (wchar_t)(c_ch + CTRL_CH_START));
					tmpstr = Ccct::Ucs2ToMbcs(wstr, charset);
					if (itv->GetType() == F_TAG_STRING)
						fprintf(fp, "\"%s\"", tmpstr);
					else
						fprintf(fp, "%s", tmpstr);
					free(tmpstr);
				}
				if (var->v.array.size() == 1) {
					fprintf(fp, ":");
					fprintf(fp, ESC_IARRAY_SAVE);
				}
			}
			fprintf(fp, ",");
			break;
		default:
			logger.Error(E_W, 7, var->name);
			break;
		};
		// �f���~�^�̕ۑ�
		tmpstr = Ccct::Ucs2ToMbcs(var->delimiter, charset);
		fprintf(fp, "%s\n", tmpstr);
		free(tmpstr);
	}

	// �t�@�C�������
	fclose(fp);

	// �����_���C����߂�
	setlocale(LC_NUMERIC,old_locale.c_str());

	logger.Message(8);
}
/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::RestoreVariable
 *  �@�\�T�v�F  �O��ۑ������ϐ����e�𕜌����܂�
 * -----------------------------------------------------------------------
 */
void	CBasis::RestoreVariable(void)
{
	std::string old_locale = setlocale(LC_NUMERIC,NULL);
	setlocale(LC_NUMERIC,"English"); //�����_�����

	// �t�@�C�����J��
	wstring	filename = path + modulename + L"_variable.cfg";
	logger.Message(6);
	logger.Filename(filename);
	FILE	*fp = w_fopen((wchar_t *)filename.c_str(), L"r");
	if (fp == NULL) {
		logger.Error(E_N, 0);
		return;
	}

	// ���e��ǂݎ��A�����������Ă���
	wstring	linebuffer;
	for (int i = 1; ; i++) {
		// 1�s�ǂݍ���
		wstring	readline;
		if (ws_fgets(readline, fp, charset, 0, i) == WS_EOF)
			break;
		// ���s�͏���
		CutCrLf(readline);
		// ��s�Ȃ玟�̍s��
		if (readline.size() == 0) {
			linebuffer = L"";
			continue;
		}
		// ���ɓǂݎ��ς̕�����ƌ���
		linebuffer += readline;
		// �_�u���N�H�[�e�[�V���������Ă��邩�m�F����B���Ă��Ȃ��ꍇ�́A
		// ���̍s�֒l�������Ă���Ǝv����̂Ŏ��̍s�̓ǂݎ���
		if (IsInDQ(linebuffer, 0, linebuffer.size() - 1))
			continue;

		wstring	parseline = linebuffer;
		linebuffer = L"";
		// �ϐ������擾
		CVariable	addvariable;
		wstring	varname, value, delimiter;
		if (!Split_IgnoreDQ(parseline, varname, value, L",")) {
			logger.Error(E_W, 1, filename, i);
			continue;
		}
		// �ϐ����̐�����������
		if (IsLegalVariableName(varname)) {
			logger.Error(E_W, 2, filename, i);
			continue;
		}
		// �l�ƃf���~�^�����o��
		parseline = value;
		if (!Split_IgnoreDQ(parseline, value, delimiter, L",")) {
			logger.Error(E_W, 3, filename, i);
			continue;
		}
		// �l���`�F�b�N���Č^�𔻒�
		int	type;
		if (IsIntString(value))
			type = F_TAG_INT;
		else if (IsDoubleString(value))
			type = F_TAG_DOUBLE;
		else if (!IsLegalStrLiteral(value))
			type = F_TAG_STRING;
		else {
			if (value.find(L':') == -1) {
				logger.Error(E_W, 4, filename, i);
				continue;
			}
			type = F_TAG_ARRAY;
		}
		// �f���~�^�̐�����������
		if (!delimiter.size()) {
			logger.Error(E_W, 5, filename, i);
			continue;
		}
		// �ϐ����쐬
		int	index = variable.Make(varname, 0);
		variable.SetType(index, type);
		if (type == F_TAG_INT)
			// �����^
			variable.SetValue(index, ws_atoi(value, 10));
		else if (type == F_TAG_DOUBLE) 
			// �����^
			variable.SetValue(index, ws_atof(value));
		else if (type == F_TAG_STRING) {
			// ������^
			CutDoubleQuote(value);
			ws_replace(value, wstring(ESC_DQ), L"\"");
			for(int c_ch = 1; c_ch <= END_OF_CTRL_CH; c_ch++)
				ws_replace(value, wstring(ESC_CTRL) + (wchar_t)(c_ch + CTRL_CH_START),
					wstring() + (wchar_t)c_ch);
			variable.SetValue(index, value);
		}
		else if (type == F_TAG_ARRAY)
			// �z��^
			RestoreArrayVariable(*(variable.GetValuePtr(index)), value);
		else {
			logger.Error(E_W, 6, filename, i);
			continue;
		}
		variable.SetDelimiter(index, delimiter);
	}

	// �t�@�C�������
	fclose(fp);

	// �����_���C����߂�
	setlocale(LC_NUMERIC,old_locale.c_str());

	logger.Message(8);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::RestoreArrayVariable
 *  �@�\�T�v�F  RestoreVariable����Ă΂�܂��B�z��ϐ��̓��e�𕜌����܂�
 * -----------------------------------------------------------------------
 */
void	CBasis::RestoreArrayVariable(CValue &var, wstring &value)
{
	var.array.clear();

	for( ; ; ) {
		CValueSub	addvs;
		wstring	par, remain;
		if (!Split_IgnoreDQ(value, par, remain, L":")) {
			if (!value.compare(ESC_IARRAY))
				break;
			else if (IsIntString(value))
				addvs = ws_atoi(value, 10);
			else if (IsDoubleString(value))
				addvs = ws_atof(value);
			else {
				addvs = value;
				CutDoubleQuote(addvs.s_value);
				ws_replace(addvs.s_value, wstring(ESC_DQ), L"\"");
				for(int c_ch = 1; c_ch <= END_OF_CTRL_CH; c_ch++)
					ws_replace(addvs.s_value, wstring(ESC_CTRL) + (wchar_t)(c_ch + CTRL_CH_START),
						wstring() + (wchar_t)c_ch);
			}
			var.array.push_back(addvs);
			break;
		}

		if (!par.compare(ESC_IARRAY)) {
			value = remain;
			continue;
		}
		else if (IsIntString(par))
			addvs = ws_atoi(par, 10);
		else if (IsDoubleString(par))
			addvs = ws_atof(par);
		else {
			addvs = par;
			CutDoubleQuote(addvs.s_value);
			ws_replace(addvs.s_value, wstring(ESC_DQ), L"\"");
			for(int c_ch = 1; c_ch <= END_OF_CTRL_CH; c_ch++)
				ws_replace(addvs.s_value, wstring(ESC_CTRL) + (wchar_t)(c_ch + CTRL_CH_START),
					wstring() + (wchar_t)c_ch);
		}
		var.array.push_back(addvs);
		value = remain;
	}
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::ExecuteLoad
 *  �@�\�T�v�F  load�֐������s���܂�
 * -----------------------------------------------------------------------
 */
void	CBasis::ExecuteLoad(void)
{
	if (IsSurpress() || loadindex == -1)
		return;

	// �������idll�̃p�X�j���쐬
	CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
	CValueSub	arg0(path);
	arg.array.push_back(arg0);
	// ���s�@���ʂ͎g�p���Ȃ��̂ł��̂܂܎̂Ă�
	calldepth.Init();
	CLocalVariable	lvar;
	int	exitcode;
	logger.Io(0, path);
	function[loadindex].Execute(arg, lvar, exitcode);
	wstring empty;
	logger.Io(1, empty);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::ExecuteRequest
 *  �@�\�T�v�F  request�֐������s���܂�
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
HGLOBAL	CBasis::ExecuteRequest(HGLOBAL h, long *len)
{
	if (IsSurpress() || requestindex == -1) {
		GlobalFree(h);
		*len = 0;
		return NULL;
	}

	// ���͕�������擾
	string	istr;
	istr.assign((char *)h, 0, (int)*len);
	GlobalFree(h);
	// �������i���͕�����j���쐬�@�����ŕ����R�[�h��UCS-2�֕ϊ�
	CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
	wchar_t	*wistr = Ccct::MbcsToUcs2(istr, charset);
	if (wistr != NULL) {
		CValueSub	arg0 = wistr;
		logger.Io(0, arg0.s_value);
		arg.array.push_back(arg0);
		free(wistr);
	}
	else
		logger.Io(0, wstring(L""));
	// ���s
	calldepth.Init();
	CLocalVariable	lvar;
	int	exitcode;
	CValue	result = function[requestindex].Execute(arg, lvar, exitcode);
	// ���ʂ𕶎���Ƃ��Ď擾���A�����R�[�h��MBCS�ɕϊ�
	wstring	res = result.GetValueString();
	logger.Io(1, res);
	char	*mostr = Ccct::Ucs2ToMbcs(res, charset);
	if (mostr == NULL) {
		// �����R�[�h�ϊ����s�ANULL��Ԃ�
		*len = 0;
		return NULL;
	}
	// �����R�[�h�ϊ������������̂ŁA���ʂ�GMEM�փR�s�[���ĕԂ�
	*len = (long)strlen(mostr);
	HGLOBAL	r_h = ::GlobalAlloc(GMEM_FIXED, *len);
	memcpy(r_h, mostr, *len);
	free(mostr);
	return r_h;
}
#else
char* CBasis::ExecuteRequest(char* h, long* len) {
    if (IsSurpress() || requestindex == -1) {
	free(h);
	*len = 0;
	return NULL;
    }
    
    // ���͕�������擾
    string istr(h, *len);
    // �������i���͕�����j���쐬�@�����ŕ����R�[�h��UCS-2�֕ϊ�
    CValue arg(F_TAG_ARRAY, 0/*dmy*/);
    wchar_t *wistr = Ccct::MbcsToUcs2(istr, charset);
    if (wistr != NULL) {
	CValueSub arg0 = wistr;
	logger.Io(0, arg0.s_value);
	arg.array.push_back(arg0);
	free(wistr);
    }
    else {
	wstring empty;
	logger.Io(0, empty);
    }
    
    // ���s
    calldepth.Init();
    CLocalVariable	lvar;
    int	exitcode;
    CValue	result = function[requestindex].Execute(arg, lvar, exitcode);
    // ���ʂ𕶎���Ƃ��Ď擾���A�����R�[�h��MBCS�ɕϊ�
    wstring	res = result.GetValueString();
    logger.Io(1, res);
    char *mostr = Ccct::Ucs2ToMbcs(res, charset);
    if (mostr == NULL) {
	// �����R�[�h�ϊ����s�ANULL��Ԃ�
	*len = 0;
	return NULL;
    }
    // �����R�[�h�ϊ������������̂ŁA���ʂ�GMEM�փR�s�[���ĕԂ�
    *len = (long)strlen(mostr);
    char* r_h = static_cast<char*>(malloc(*len));
    memcpy(r_h, mostr, *len);
    free(mostr);
    return r_h;
}
#endif

/* -----------------------------------------------------------------------
 *  �֐���  �F  CBasis::ExecuteUnload
 *  �@�\�T�v�F  unload�֐������s���܂�
 * -----------------------------------------------------------------------
 */
void	CBasis::ExecuteUnload(void)
{
	if (IsSurpress()|| loadindex == -1)
		return;

	// ���s�@���������@���ʂ͎g�p���Ȃ��̂ł��̂܂܎̂Ă�
	CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
	calldepth.Init();
	CLocalVariable	lvar;
	int	exitcode;
	wstring empty;
	logger.Io(0, empty);
	function[unloadindex].Execute(arg, lvar, exitcode);
	logger.Io(1, empty);
}
