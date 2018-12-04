// 
// AYA version 5
//
// ロギング用クラス　CLog
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#else
#  include "posix_utils.h"
#  include "messages.h"
#  include <sys/time.h>
#  include <iostream>
#endif
#include "basis.h"
#include "comment.h"
#include "wsex.h"
#include "misc.h"
#include "log.h"
#include "ccct.h"

/* -----------------------------------------------------------------------
 *  関数名  ：  CLog::Start
 *  機能概要：  ロギングを開始します
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
void	CLog::Start(const wstring &p, int cs, int ml, HWND hw, char il)
#else
void	CLog::Start(const wstring &p, int cs, int ml, char il)
#endif
{
	path    = p;
	charset = cs;
	msglang = ml;
#ifndef POSIX
	hWnd    = hw;
#endif
	iolog   = il;
#ifdef POSIX
	fix_filepath(path);
#endif
	
#ifndef POSIX
	// もしhWndがNULLなら起動中のチェックツールを探して取得する
	if (hWnd == NULL)
		hWnd = GetCheckerWnd();
#endif

	// ロギング有効/無効の判定
	if (!path.size()) {
		fileen = 0;
#ifndef POSIX
		if (hWnd == NULL) {
			enable = 0;
			return;
		}
#else
		enable = 0;
		return;
#endif
	}

	// 文字列作成
	wstring	str = (msglang) ? msge[0] : msgj[0];
	str += GetDateString();
	str += L"\n\n";

	// ファイルへ書き込み
	if (fileen) {
		char	*tmpstr = Ccct::Ucs2ToMbcs(str, charset);
		if (tmpstr != NULL) {
			FILE	*fp = w_fopen((wchar_t *)path.c_str(), L"w");
			if (fp != NULL) {
/*				if (charset == CHARSET_UTF8)
					write_utf8bom(fp);*/
				fprintf(fp, "%s", tmpstr);
				fclose(fp);
			}
			free(tmpstr);
		}
	}

#ifndef POSIX
	// チェックツールへ送出　最初に文字コードを設定してから文字列を送出
	if (charset == CHARSET_SJIS)
		SendLogToWnd(L"", E_SJIS);
	else if (charset == CHARSET_UTF8)
		SendLogToWnd(L"", E_UTF8);
	else	// CHARSET_DEFAULT
		SendLogToWnd(L"", E_DEFAULT);

	SendLogToWnd(str, E_I);
#endif
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLog::Termination
 *  機能概要：  ロギングを終了します
 * -----------------------------------------------------------------------
 */
void	CLog::Termination(void)
{
	if (!enable)
		return;

	Message(1);

	wstring	str = GetDateString();
	str += L"\n\n";

	Write(str);

#ifndef POSIX
	SendLogToWnd(L"", E_END);
#endif
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLog::Write
 *  機能概要：  ログに文字列を書き込みます
 * -----------------------------------------------------------------------
 */
void	CLog::Write(const wstring &str, int mode)
{
	if (!enable)
		return;

	// 文字列中の\rは消す
	wstring	cstr = str;
	int	len = cstr.size();
	for(int i = 0; i < len; ) {
		if (cstr[i] == L'\r') {
			cstr.erase(i, 1);
			len--;
			continue;
		}
		i++;
	}

	// ファイルへ書き込み
	if (fileen) {
		char	*tmpstr = Ccct::Ucs2ToMbcs(cstr, charset);
		if (tmpstr != NULL) {
			FILE	*fp = w_fopen((wchar_t *)path.c_str(), L"a");
			if (fp != NULL) {
				fprintf(fp, "%s", tmpstr);
				fclose(fp);
			}
			free(tmpstr);
		}
	}

#ifndef POSIX
	// チェックツールへ送出
	SendLogToWnd(cstr, mode);
#endif
}

//----

void	CLog::Write(const wchar_t *str, int mode)
{
	if (str == NULL)
		return;
	if (!wcslen(str))
		return;

	wstring	lstr = str;
	Write(lstr, mode);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLog::Filename
 *  機能概要：  既定のフォーマットでファイル名をログに記録します
 * -----------------------------------------------------------------------
 */
void	CLog::Filename(const wstring &filename)
{
	wstring	str =  L"- ";
	str += filename;
	str += L"\n";
	Write(str);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLog::Message
 *  機能概要：  idで指定された既定のメッセージをログに書き込みます
 * -----------------------------------------------------------------------
 */
void	CLog::Message(int id)
{
	Write((msglang) ? (wchar_t *)msge[id] : (wchar_t *)msgj[id], 0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLog::Error
 *  機能概要：  ログにmodeとidで指定されたエラー文字列を書き込みます
 *
 *  引数　　：　ref         付加情報
 *  　　　　　  dicfilename エラーを起こした箇所を含む辞書ファイルの名前
 *  　　　　　  linecount   エラーを起こした行番号
 *
 *  　　　　　  refとdicfilenameはNULL、linecountは-1とすることで、これらを
 *  　　　　　  非表示にできます
 * -----------------------------------------------------------------------
 */
void	CLog::Error(int mode, int id, const wchar_t *ref, const wstring &dicfilename, int linecount)
{
	if (!enable)
		return;

	// ログに書き込み文字列を作成（辞書ファイル名と行番号）
	wstring	logstr;
	if (dicfilename.empty())
		logstr = L"-(-) : ";
	else {
		logstr = dicfilename + L"(";
		if (linecount == -1)
			logstr += L"-) : ";
		else {
			wstring	lcstr;
			ws_itoa(lcstr, linecount, 10);
			logstr += lcstr;
			logstr += L") : ";
		}
	}
	// ログに書き込み文字列を作成（本文）
	if (msglang) {
		// 英語
		if (mode == E_F)
			logstr += msgfe[id];
		else if (mode == E_E)
			logstr += msgee[id];
		else if (mode == E_W)
			logstr += msgwe[id];
		else
			logstr += msgne[id];
	}
	else {
		// 日本語
		if (mode == E_F)
			logstr += msgfj[id];
		else if (mode == E_E)
			logstr += msgej[id];
		else if (mode == E_W)
			logstr += msgwj[id];
		else
			logstr += msgnj[id];
	}
	// ログに書き込み文字列を作成（付加情報）
	if (ref != NULL) {
		logstr += L" : ";
		logstr += ref;
	}
	// 書き込み
	logstr += L'\n';
	Write(logstr, mode);
}

//----

void	CLog::Error(int mode, int id, const wstring& ref, const wstring& dicfilename, int linecount)
{
	Error(mode, id, (wchar_t *)ref.c_str(), dicfilename, linecount);
}

//----

void	CLog::Error(int mode, int id, const wchar_t *ref)
{
        Error(mode, id, ref, wstring(), -1);
}

//----

void	CLog::Error(int mode, int id, const wstring& ref)
{
        Error(mode, id, (wchar_t *)ref.c_str(), wstring(), -1);
}

//----

void	CLog::Error(int mode, int id, const wstring& dicfilename, int linecount)
{
	Error(mode, id, (wchar_t *)NULL, dicfilename, linecount);
}

//----

void	CLog::Error(int mode, int id)
{
        Error(mode, id, (wchar_t *)NULL, wstring(), -1);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLog::Io
 *  機能概要：  入出力文字列と実行時間をログに記録します
 *  引数　　：  io 0/1=開始時/終了時
 * -----------------------------------------------------------------------
 */
void	CLog::Io(char io, const wstring &str)
{
	if (!enable || !iolog)
		return;

	static int		starttime = 0;
	static	wchar_t	t_str[STRMAX];

	if (!io) {
		Write(L"// request\n");
		Write(str + L"\n");
#ifndef POSIX
		starttime = GetTickCount();
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		starttime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
	}
	else {
#ifndef POSIX
		swprintf(t_str, L"// response (Execution time : %d[ms])\n", GetTickCount() - starttime);
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		swprintf(t_str, STRMAX, L"// response (Execution time : %d[ms])\n", tv.tv_sec * 1000 + tv.tv_usec / 1000 - starttime);
#endif
		Write(t_str);
		Write(str + L"\n");
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLog::IoLib
 *  機能概要：  外部ライブラリ入出力文字列と実行時間をログに記録します
 *  引数　　：  io 0/1=開始時/終了時
 * -----------------------------------------------------------------------
 */
void	CLog::IoLib(char io, const wstring &str, const wstring &name)
{
	if (!enable || !iolog)
		return;

	static int		starttime = 0;
	static	wchar_t	t_str[STRMAX];

	if (!io) {
		Write(L"// request to library\n// name : ");
		Write(name + L"\n");
		Write(str + L"\n");
#ifndef POSIX
		starttime = GetTickCount();
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		starttime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
	}
	else {
#ifndef POSIX
		swprintf(t_str, L"// response from library (Execution time : %d[ms])\n", GetTickCount() - starttime);
#else
		struct timeval tv;
		gettimeofday(&tv, NULL);
		swprintf(t_str, STRMAX, L"// response from library (Execution time : %d[ms])\n", tv.tv_sec * 1000 + tv.tv_usec / 1000 - starttime);
#endif
		Write(t_str);
		Write(str + L"\n");
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLog::SendLogToWnd
 *  機能概要：  チェックツールに制御メッセージおよびログ文字列をWM_COPYDATAで送信します
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
void	CLog::SendLogToWnd(const wchar_t *str, int mode)
{
	if (hWnd == NULL)
		return;

	COPYDATASTRUCT cds;
	cds.dwData = mode;
	cds.cbData = (wcslen(str) + 1)*sizeof(wchar_t);
	cds.lpData = (LPVOID)str;
	::SendMessage(hWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
}

//----

void	CLog::SendLogToWnd(const wstring &str, int mode)
{
	SendLogToWnd((wchar_t *)str.c_str(), mode);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CLog::GetCheckerWnd
 *  機能概要：  チェックツールのhWndを取得しますに
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
HWND	CLog::GetCheckerWnd(void)
{
	return FindWindow(CLASSNAME_CHECKTOOL, NULL);
}
#endif
