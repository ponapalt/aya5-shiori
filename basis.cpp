// 
// AYA version 5
//
// 主な制御を行なうクラス　CBasis
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
 * CBasisコンストラクタ
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
 *  関数名  ：  CBasis::SetModuleHandle
 *  機能概要：  モジュールハンドルを取得します
 *
 *  ついでにモジュールの主ファイル名取得も行います
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
void	CBasis::SetModuleHandle(HANDLE h)
{
	// モジュールハンドルを取得
	hmodule = (HMODULE)h;

	// モジュールの主ファイル名を取得
	// NT系ではいきなりUNICODEで取得できるが、9x系を考慮してMBCSで取得してからUCS-2へ変換
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
 *  関数名  ：  CBasis::SetPath
 *  機能概要：  HGLOBALに格納されたファイルパスを取得し、HGLOBALは開放します
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
void	CBasis::SetPath(HGLOBAL h, int len)
{
	// 取得と領域開放
	string	mbpath;
	mbpath.assign((char *)h, 0, len);
	GlobalFree(h);
	// 文字コードをUCS-2へ変換（ここでのマルチバイト文字コードはOSデフォルト）
	wchar_t	*wcpath = Ccct::MbcsToUcs2(mbpath, CHARSET_DEFAULT);
	path = wcpath;
	free(wcpath);
}
#else
void CBasis::SetPath(char* h, long len) {
    // 取得と領域開放
    path = widen(string(h, static_cast<string::size_type>(len)));
    free(h);
    // スラッシュで終わってなければ付ける。
    if (path.length() == 0 || path[path.length() - 1] != L'/') {
	path += L'/';
    }
    // モジュールハンドルの取得は出来ないので、力技で位置を知る。
    // このディレクトリにある全ての*.dll(case insensitive)を探し、
    // 中身にaya5.dllという文字列を含んでいたら、それを選ぶ。
    // ただし対応する*.txtが無ければdllの中身は見ずに次へ行く。
    modulename = L"aya5";
    DIR* dh = opendir(narrow(path).c_str());
    if (dh == NULL) {
	cerr << narrow(path) << "is not a directory!" << endl;
	exit(1);
    }
    while (true) {
	struct dirent* ent = readdir(dh);
	if (ent == NULL) {
	    break; // もう無い
	}
	string fname(ent->d_name, strlen(ent->d_name)/*ent->d_namlen*/);	// by umeici. 2005/1/16 5.6.0.232
	if (lc(get_extension(fname)) == "dll") {
	    string txt_file = narrow(path) + change_extension(fname, "txt");
	    struct stat sb;
	    if (::stat(txt_file.c_str(), &sb) == 0) {
		// txtファイルがあるので、中身を見てみる。
		if (file_content_search(narrow(path) + fname, "aya5.dll") != string::npos) {
		    // これは文のDLLである。
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
 *  関数名  ：  CBasis::SetLogRcvWnd
 *  機能概要：  チェックツールから渡されたhWndを保持します
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
void	CBasis::SetLogRcvWnd(long hwnd)
{
	hlogrcvWnd = (HWND)hwnd;
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::Configure
 *  機能概要：  load時に行う初期設定処理
 * -----------------------------------------------------------------------
 */
void	CBasis::Configure(void)
{
	// 基礎設定ファイル（例えばaya.txt）を読み取り
	vector<wstring>	dics;
	LoadBaseConfigureFile(&dics);
	// 基礎設定ファイル読み取りで重篤なエラーが発生した場合はここで終了
	if (surpress)
		return;

	// ロギングを開始
#ifndef POSIX
	logger.Start(logpath, charset, msglang, hlogrcvWnd, iolog);
#else
	logger.Start(logpath, charset, msglang, iolog);
#endif

	// 辞書読み込みと構文解析
	if (CParser0::Parse(charset, dics, loadindex, unloadindex, requestindex))
		SetSurpress();

	if (checkparser)
		CLogExCode::OutExecutionCodeForCheck();

	// 前回終了時に保存した変数を復元
	RestoreVariable();

	if (checkparser)
		CLogExCode::OutVariableInfoForCheck();

	// ここまでの処理で重篤なエラーが発生した場合はここで終了
	if (surpress)
		return;

	// 外部ライブラリとファイルの文字コードを初期化
	libs.SetCharset(charset);
	files.SetCharset(charset);

	// load関数を実行
	ExecuteLoad();

	run = 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::Termination
 *  機能概要：  unload時に行う終了処理
 * -----------------------------------------------------------------------
 */
void	CBasis::Termination(void)
{
	// 動作抑止されていなければ終了時の処理を実行
	if (!surpress) {
		// unload
		ExecuteUnload();
		// ロードしているすべてのライブラリをunload
		libs.DeleteAll();
		// 開いているすべてのファイルを閉じる
		files.DeleteAll();
		// 変数の保存
		SaveVariable();
	}

	// ロギングを終了
	logger.Termination();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::IsSurpress
 *  機能概要：  現在の自律抑止状態を返します
 *
 *  返値　　：  0/1=非抑止/抑止
 *
 *  基礎設定ファイルの読み取りや辞書ファイルの解析中に、動作継続困難なエラーが発生すると
 *  SetSurpress()によって抑止設定されます。抑止設定されると、load/request/unloadでの動作が
 *  すべてマスクされます。この時、requestの返値は常に空文字列になります。（HGLOBAL=NULL、
 *  len=0で応答します）
 * -----------------------------------------------------------------------
 */
char	CBasis::IsSurpress(void)
{
	return surpress;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::SetSurpress
 *  機能概要：  自律動作抑止を設定します
 * -----------------------------------------------------------------------
 */
void	CBasis::SetSurpress(void)
{
	surpress = 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::ResetSurpress
 *  機能概要：  自律動作抑止機能をリセットします
 * -----------------------------------------------------------------------
 */
void	CBasis::ResetSurpress(void)
{
	surpress = 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::LoadBaseConfigureFile
 *  機能概要：  基礎設定ファイルを読み取り、各種パラメータをセットします
 *
 *  基礎設定ファイルはDLLと同階層に存在する名前が"DLL主ファイル名.txt"のファイルです。
 *
 *  辞書ファイルの文字コードはShift_JIS以外にもUTF-8とOSデフォルトのコードに対応できますが、
 *  この基礎設定ファイルはOSデフォルトのコードで読み取られることに注意してください。
 *  国際化に関して考慮する場合は、このファイル内の記述にマルチバイト文字を使用するべきでは
 *  ありません（文字コード0x7F以下のASCII文字のみで記述すべきです）。
 * -----------------------------------------------------------------------
 */
void	CBasis::LoadBaseConfigureFile(vector<wstring> *dics)
{
	// 設定ファイル("name".txt)読み取り

	// ファイルを開く
    	wstring	filename = path + modulename + L".txt";
	FILE	*fp = w_fopen((wchar_t *)filename.c_str(), L"r");
	if (fp == NULL) {
		SetSurpress();
		return;
	}

	// 読み取り処理
	CComment	comment;
	for (int i = 1; ; i++) {
		// 1行読み込み
		wstring	readline;
		if (ws_fgets(readline, fp, CHARSET_DEFAULT, 0, i) == WS_EOF)
			break;
		// 改行は消去
		CutCrLf(readline);
		// コメントアウト処理
		comment.ExecuteSS_Top(readline);
		comment.Execute(readline);
		comment.ExecuteSS_Tail(readline);
		// 空行、もしくは全体がコメント行だった場合は次の行へ
		if (readline.size() == 0)
			continue;
		// パラメータを設定
		wstring	cmd, param;
		if (Split(readline, cmd, param, L",")) {
			SetParameter(cmd, param, dics);
		}
		else {
			logger.Error(E_W, 0, filename, i);
		}
	}

	// ファイルを閉じる
	fclose(fp);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::SetParameter
 *  機能概要：  LoadBaseConfigureFileから呼ばれます。各種パラメータを設定します
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
	// checkparser　※非公開機能
	
	else if (!cmd.compare(L"checkparser")) {
		if (!param.compare(L"on"))
			checkparser = 1;
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::SaveVariable
 *  機能概要：  変数値をファイルに保存します
 *
 *  ファイル名は"DLL主ファイル名_variable.cfg"です。
 *  ファイルフォーマットは1行1変数、デリミタ半角カンマで、
 *
 *  変数名,内容,デリミタ
 *
 *  の形式で保存されます。内容は整数/実数の場合はそのまま、文字列ではダブルクォートされます。
 *  配列の場合は各要素間がコロンで分割されます。以下に要素数3、デリミタ"@"での例を示します。
 *
 *  var,1:"TEST":0.3,@
 *
 *  デリミタはダブルクォートされません。
 *
 *  基礎設定ファイルで設定した文字コードで保存されます。
 * -----------------------------------------------------------------------
 */
void	CBasis::SaveVariable(void)
{
	// 変数の保存
	std::string old_locale = setlocale(LC_NUMERIC,NULL);
	setlocale(LC_NUMERIC,"English"); //小数点問題回避

	// ファイルを開く
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
	// UTF-8の場合は先頭にBOMを保存
	if (charset == CHARSET_UTF8)
		write_utf8bom(fp);
#endif
	// UTF-8なのにBOMを付けるのはやめた方が宜しいかと…
	// トラブルの原因になるので。

	// 了解です。外してしまいます。
	// メモ
	// UTF-8にはバイトオーダーによるバリエーションが存在しないのでBOMは必要ない。
	// 付与することは出来る。しかし対応していないソフトで読めなくなるので付けないほうが
	// 良い。
*/

	// 順次保存
	wstring	wstr;
	int	c_ch;
	char	*tmpstr;
	int	var_num = variable.GetNumber();
	for(int i = 0; i < var_num; i++) {
		CVariable	*var = variable.GetPtr(i);
		// 内容が空文字列の変数は保存しない
		if (var->v.GetType() == F_TAG_STRING && !var->v.s_value.size())
			continue;
		// 内容が空汎用配列の変数は保存しない
//		if (var->v.GetType() == F_TAG_ARRAY && !var->v.array.size())
//			continue;
		// 消去フラグが立っている変数は保存しない
		if (var->IsErased())
			continue;
		// 名前の保存
		tmpstr = Ccct::Ucs2ToMbcs(var->name, charset);
		fprintf(fp, "%s,", tmpstr);
		free(tmpstr);
		// 値の保存
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
		// デリミタの保存
		tmpstr = Ccct::Ucs2ToMbcs(var->delimiter, charset);
		fprintf(fp, "%s\n", tmpstr);
		free(tmpstr);
	}

	// ファイルを閉じる
	fclose(fp);

	// 小数点問題修正を戻す
	setlocale(LC_NUMERIC,old_locale.c_str());

	logger.Message(8);
}
/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::RestoreVariable
 *  機能概要：  前回保存した変数内容を復元します
 * -----------------------------------------------------------------------
 */
void	CBasis::RestoreVariable(void)
{
	std::string old_locale = setlocale(LC_NUMERIC,NULL);
	setlocale(LC_NUMERIC,"English"); //小数点問題回避

	// ファイルを開く
	wstring	filename = path + modulename + L"_variable.cfg";
	logger.Message(6);
	logger.Filename(filename);
	FILE	*fp = w_fopen((wchar_t *)filename.c_str(), L"r");
	if (fp == NULL) {
		logger.Error(E_N, 0);
		return;
	}

	// 内容を読み取り、順次復元していく
	wstring	linebuffer;
	for (int i = 1; ; i++) {
		// 1行読み込み
		wstring	readline;
		if (ws_fgets(readline, fp, charset, 0, i) == WS_EOF)
			break;
		// 改行は消去
		CutCrLf(readline);
		// 空行なら次の行へ
		if (readline.size() == 0) {
			linebuffer = L"";
			continue;
		}
		// 既に読み取り済の文字列と結合
		linebuffer += readline;
		// ダブルクォーテーションが閉じているか確認する。閉じていない場合は、
		// 次の行へ値が続いていると思われるので次の行の読み取りへ
		if (IsInDQ(linebuffer, 0, linebuffer.size() - 1))
			continue;

		wstring	parseline = linebuffer;
		linebuffer = L"";
		// 変数名を取得
		CVariable	addvariable;
		wstring	varname, value, delimiter;
		if (!Split_IgnoreDQ(parseline, varname, value, L",")) {
			logger.Error(E_W, 1, filename, i);
			continue;
		}
		// 変数名の正当性を検査
		if (IsLegalVariableName(varname)) {
			logger.Error(E_W, 2, filename, i);
			continue;
		}
		// 値とデリミタを取り出す
		parseline = value;
		if (!Split_IgnoreDQ(parseline, value, delimiter, L",")) {
			logger.Error(E_W, 3, filename, i);
			continue;
		}
		// 値をチェックして型を判定
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
		// デリミタの正当性を検査
		if (!delimiter.size()) {
			logger.Error(E_W, 5, filename, i);
			continue;
		}
		// 変数を作成
		int	index = variable.Make(varname, 0);
		variable.SetType(index, type);
		if (type == F_TAG_INT)
			// 整数型
			variable.SetValue(index, ws_atoi(value, 10));
		else if (type == F_TAG_DOUBLE) 
			// 実数型
			variable.SetValue(index, ws_atof(value));
		else if (type == F_TAG_STRING) {
			// 文字列型
			CutDoubleQuote(value);
			ws_replace(value, wstring(ESC_DQ), L"\"");
			for(int c_ch = 1; c_ch <= END_OF_CTRL_CH; c_ch++)
				ws_replace(value, wstring(ESC_CTRL) + (wchar_t)(c_ch + CTRL_CH_START),
					wstring() + (wchar_t)c_ch);
			variable.SetValue(index, value);
		}
		else if (type == F_TAG_ARRAY)
			// 配列型
			RestoreArrayVariable(*(variable.GetValuePtr(index)), value);
		else {
			logger.Error(E_W, 6, filename, i);
			continue;
		}
		variable.SetDelimiter(index, delimiter);
	}

	// ファイルを閉じる
	fclose(fp);

	// 小数点問題修正を戻す
	setlocale(LC_NUMERIC,old_locale.c_str());

	logger.Message(8);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::RestoreArrayVariable
 *  機能概要：  RestoreVariableから呼ばれます。配列変数の内容を復元します
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
 *  関数名  ：  CBasis::ExecuteLoad
 *  機能概要：  load関数を実行します
 * -----------------------------------------------------------------------
 */
void	CBasis::ExecuteLoad(void)
{
	if (IsSurpress() || loadindex == -1)
		return;

	// 第一引数（dllのパス）を作成
	CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
	CValueSub	arg0(path);
	arg.array.push_back(arg0);
	// 実行　結果は使用しないのでそのまま捨てる
	calldepth.Init();
	CLocalVariable	lvar;
	int	exitcode;
	logger.Io(0, path);
	function[loadindex].Execute(arg, lvar, exitcode);
	wstring empty;
	logger.Io(1, empty);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::ExecuteRequest
 *  機能概要：  request関数を実行します
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

	// 入力文字列を取得
	string	istr;
	istr.assign((char *)h, 0, (int)*len);
	GlobalFree(h);
	// 第一引数（入力文字列）を作成　ここで文字コードをUCS-2へ変換
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
	// 実行
	calldepth.Init();
	CLocalVariable	lvar;
	int	exitcode;
	CValue	result = function[requestindex].Execute(arg, lvar, exitcode);
	// 結果を文字列として取得し、文字コードをMBCSに変換
	wstring	res = result.GetValueString();
	logger.Io(1, res);
	char	*mostr = Ccct::Ucs2ToMbcs(res, charset);
	if (mostr == NULL) {
		// 文字コード変換失敗、NULLを返す
		*len = 0;
		return NULL;
	}
	// 文字コード変換が成功したので、結果をGMEMへコピーして返す
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
    
    // 入力文字列を取得
    string istr(h, *len);
    // 第一引数（入力文字列）を作成　ここで文字コードをUCS-2へ変換
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
    
    // 実行
    calldepth.Init();
    CLocalVariable	lvar;
    int	exitcode;
    CValue	result = function[requestindex].Execute(arg, lvar, exitcode);
    // 結果を文字列として取得し、文字コードをMBCSに変換
    wstring	res = result.GetValueString();
    logger.Io(1, res);
    char *mostr = Ccct::Ucs2ToMbcs(res, charset);
    if (mostr == NULL) {
	// 文字コード変換失敗、NULLを返す
	*len = 0;
	return NULL;
    }
    // 文字コード変換が成功したので、結果をGMEMへコピーして返す
    *len = (long)strlen(mostr);
    char* r_h = static_cast<char*>(malloc(*len));
    memcpy(r_h, mostr, *len);
    free(mostr);
    return r_h;
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CBasis::ExecuteUnload
 *  機能概要：  unload関数を実行します
 * -----------------------------------------------------------------------
 */
void	CBasis::ExecuteUnload(void)
{
	if (IsSurpress()|| loadindex == -1)
		return;

	// 実行　引数無し　結果は使用しないのでそのまま捨てる
	CValue	arg(F_TAG_ARRAY, 0/*dmy*/);
	calldepth.Init();
	CLocalVariable	lvar;
	int	exitcode;
	wstring empty;
	logger.Io(0, empty);
	function[unloadindex].Execute(arg, lvar, exitcode);
	logger.Io(1, empty);
}
