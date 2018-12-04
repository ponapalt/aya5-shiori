// 
// AYA version 5
//
// 雑用関数
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#else
#  include "posix_utils.h"
#  include <time.h>
#endif
#include "basis.h"
#include "sysfunc.h"
#include "comment.h"
#include "wsex.h"
#include "misc.h"
#include "log.h"
#include "ccct.h"
#include "wordmatch.h"

extern CBasis				basis;
extern vector<CFunction>	function;
extern CGlobalVariable		variable;
extern CLog					logger;

extern CWordMatch			function_wm;


#ifndef POSIX
#  define for if(0);else for
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  Split
 *  機能概要：  文字列を分割して余分な空白を削除します
 *
 *  返値　　：  0/1=失敗/成功
 * -----------------------------------------------------------------------
 */
char	Split(const wstring &str, wstring &dstr0, wstring &dstr1, const wchar_t *sepstr)
{
	int	seppoint = str.find(sepstr);
	if (seppoint == -1)
		return 0;

	dstr0.assign(str, 0, seppoint);
	seppoint += ::wcslen(sepstr);
	dstr1.assign(str.substr(seppoint, str.size() - seppoint));

	CutSpace(dstr0);
	CutSpace(dstr1);

	return 1;
}

//----

char	Split(const wstring &str, wstring &dstr0, wstring &dstr1, const wstring &sepstr)
{
	int	seppoint = str.find(sepstr);
	if (seppoint == -1)
		return 0;

	dstr0.assign(str, 0, seppoint);
	seppoint += sepstr.size();
	dstr1.assign(str.substr(seppoint, str.size() - seppoint));

	CutSpace(dstr0);
	CutSpace(dstr1);

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  SplitOnly
 *  機能概要：  文字列を分割します
 *
 *  返値　　：  0/1=失敗/成功
 * -----------------------------------------------------------------------
 */
char	SplitOnly(const wstring &str, wstring &dstr0, wstring &dstr1, const wchar_t *sepstr)
{
	int	seppoint = str.find(sepstr);
	if (seppoint == -1)
		return 0;

	dstr0.assign(str, 0, seppoint);
	seppoint += ::wcslen(sepstr);
	dstr1.assign(str.substr(seppoint, str.size() - seppoint));

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  Split_IgnoreDQ
 *  機能概要：  文字列を分割して余分な空白を削除します
 *  　　　　　  ただしダブル/シングルクォート内では分割しません
 *
 *  返値　　：  0/1=失敗/成功
 * -----------------------------------------------------------------------
 */
char	Split_IgnoreDQ(const wstring &str, wstring &dstr0, wstring &dstr1, const wchar_t *sepstr)
{
	int	sepstrlen = ::wcslen(sepstr);
	int	seppoint = 0;
	for( ; ; ) {
		seppoint = str.find(sepstr, seppoint);
		if (seppoint == -1)
			return 0;
		if (IsInDQ(str, 0, seppoint)) {
			seppoint += sepstrlen;
			continue;
		}
		break;
	}

	dstr0.assign(str, 0, seppoint);
	seppoint += sepstrlen;
	dstr1.assign(str.substr(seppoint, str.size() - seppoint));

	CutSpace(dstr0);
	CutSpace(dstr1);

	return 1;
}

//----

char	Split_IgnoreDQ(const wstring &str, wstring &dstr0, wstring &dstr1, const wstring &sepstr)
{
	return Split_IgnoreDQ(str,dstr0,dstr1,sepstr.c_str());
}

/* -----------------------------------------------------------------------
 *  関数名  ：  SplitToMultiString
 *  機能概要：  文字列を分割してvectorに格納します
 *
 *　返値　　：　分割数(array.size())
 * -----------------------------------------------------------------------
 */
int	SplitToMultiString(const wstring &str, vector<wstring> &array, const wstring &delimiter)
{
	if (!str.size())
		return 0;

	wstring	t_str = str;
	int	dlmlen    = delimiter.size();
	for( ; ; ) {
		// デリミタの発見
		int	seppoint = t_str.find(delimiter);
		if (seppoint == -1) {
			array.push_back(t_str);
			break;
		}
		// 取り出しとvectorへの追加
		wstring	i_str;
		i_str.assign(t_str, 0, seppoint);
		array.push_back(i_str);
		// 取り出した分を削除
		t_str.erase(0, seppoint + dlmlen);
	}

	return array.size();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CutSpace
 *  機能概要：  与えられた文字列の前後に半角空白かタブがあった場合、すべて削除します
 * -----------------------------------------------------------------------
 */
void	CutSpace(wstring &str)
{
	int	len = str.size();
	// 前方
	int	erasenum = 0;
	for(int i = 0; i < len; i++) {
#ifndef POSIX
		if (str[i] == L' ' || str[i] == L'\t' || str[i] == L'　')
#else
		if (str[i] == L' ' || str[i] == L'\t' || str[i] == L'\u3000')
#endif
			erasenum++;
		else
			break;
	}
	if (erasenum) {
		str.erase(0, erasenum);
		len -= erasenum;
	}
	// 後方
	erasenum = 0;
	for(int i = len - 1; i >= 0; i--) {
#ifndef POSIX
		if (str[i] == L' ' || str[i] == L'\t' || str[i] == L'　')
#else
		if (str[i] == L' ' || str[i] == L'\t' || str[i] == L'\u3000')
#endif
			erasenum++;
		else
			break;
	}
	if (erasenum)
		str.erase(len - erasenum, erasenum);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CutDoubleQuote
 *  機能概要：  与えられた文字列の前後にダブルクォートがあった場合削除します
 * -----------------------------------------------------------------------
 */
void	CutDoubleQuote(wstring &str)
{
	int	len = str.size();
	if (!len)
		return;
	// 前方
	if (str[0] == L'\"') {
		str.erase(0, 1);
		len--;
		if (!len)
			return;
	}
	// 後方
	if (str[len - 1] == L'\"')
		str.erase(len - 1, 1);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CutSingleQuote
 *  機能概要：  与えられた文字列の前後にシングルクォートがあった場合削除します
 * -----------------------------------------------------------------------
 */
void	CutSingleQuote(wstring &str)
{
	int	len = str.size();
	if (!len)
		return;
	// 前方
	if (str[0] == L'\'') {
		str.erase(0, 1);
		len--;
		if (!len)
			return;
	}
	// 後方
	if (str[len - 1] == L'\'')
		str.erase(len - 1, 1);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  AddDoubleQuote
 *  機能概要：  与えられた文字列をダブルクォートで囲みます
 * -----------------------------------------------------------------------
 */
void	AddDoubleQuote(wstring &str)
{
	str = L"\"" + str + L"\"";
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CutCrLf
 *  機能概要：  与えられた文字列の後端に改行(CRLF)があった場合削除します
 * -----------------------------------------------------------------------
 */
void	CutCrLf(wstring &str)
{
	ws_eraseend(str, L'\n');
	ws_eraseend(str, L'\r');
}

/* -----------------------------------------------------------------------
 *  関数名  ：  GetDateString
 *  機能概要：  年月日/時分秒の文字列を作成して返します
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
wstring	GetDateString(void)
{
	wstring	result;
	
	// 年月日
	wchar_t	_datebuffer[32];
	_wstrdate(_datebuffer);
	result = _datebuffer;
	result += (basis.GetMsgLang()) ? L"(m/d/y) " : L"(月/日/年) ";
	// 時分秒
	wchar_t	_timebuffer[32];
	_wstrtime(_timebuffer);
	result += _timebuffer;

	return result;
}
#else
wstring GetDateString() {
    char buf[64];
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    strftime(buf, 128, "%m/%d/%Y %H:%M:%S", tm);
    
    return widen(string(buf));
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  IsInDQ
 *  機能概要：  文字列内の指定位置がダブル/シングルクォート範囲内かをチェックします
 *
 *  返値　　：  0/1=ダブル/シングルクォートの外部/内部
 * -----------------------------------------------------------------------
 */
char	IsInDQ(const wstring &str, int startpoint, int checkpoint)
{
	int	dq    = 0;
	int quote = 0;

	int	len      = str.size();
	for(int i = startpoint; i < checkpoint; i++) {
		if (i >= len)
			break;
		if (str[i] == L'\"') {
			if (!quote)
				dq ^= 1;
		}
		else if (str[i] == L'\'') {
			if (!dq)
				quote ^= 1;
		}
	}

	return dq | quote;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  IsDoubleString
 *  機能概要：  文字列が実数数値として正当かを検査します
 *
 *  返値　　：  0/1=×/○
 * -----------------------------------------------------------------------
 */
char	IsDoubleString(const wstring &str)
{
	int	len = str.size();
	if (!len)
		return 0;

	int	i = (str[0] == L'-' || str[0] == L'+') ? 1 : 0;
	int	dotcount = 0;
	for( ; i < len; i++)
//		if (!::iswdigit((int)str[i])) {
		if (str[i] < L'0' || str[i] > L'9') {
			if (str[i] == L'.' && !dotcount)
				dotcount++;
			else
				return 0;
		}

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  IsIntString
 *  機能概要：  文字列が10進整数数値として正当かを検査します
 *
 *  返値　　：  0/1=×/○
 * -----------------------------------------------------------------------
 */
char	IsIntString(const wstring &str)
{
	int	len = str.size();
	if (!len)
		return 0;

	int	i = (str[0] == L'-' || str[0] == L'+') ? 1 : 0;

	for( ; i < len; i++)
//		if (!::iswdigit((int)str[i]))
		if (str[i] < L'0' || str[i] > L'9')
			return 0;

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  IsIntBinString
 *  機能概要：  文字列が2進整数数値として正当かを検査します
 *  引数　　：  header 0/1=先頭"0x"なし/あり
 *
 *  返値　　：  0/1=×/○
 * -----------------------------------------------------------------------
 */
char	IsIntBinString(const wstring &str, char header)
{
	int	len = str.size();
	if (!len)
		return 0;

	int	i = (str[0] == L'-' || str[0] == L'+') ? 1 : 0;

	if (header) {
		if (::wcscmp(PREFIX_BIN, str.substr(i, PREFIX_BASE_LEN).c_str()))
			return 0;
		i += PREFIX_BASE_LEN;
	}
	
	for( ; i < len; i++) {
		wchar_t	j = str[i];
		if (j != L'0' && j != L'1')
			return 0;
	}

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  IsIntHexString
 *  機能概要：  文字列が16進整数数値として正当かを検査します
 *
 *  返値　　：  0/1=×/○
 * -----------------------------------------------------------------------
 */
char	IsIntHexString(const wstring &str, char header)
{
	int	len = str.size();
	if (!len)
		return 0;

	int	i = (str[0] == L'-' || str[0] == L'+') ? 1 : 0;

	if (header) {
		if (::wcscmp(PREFIX_HEX, str.substr(i, PREFIX_BASE_LEN).c_str()))
			return 0;
		i += PREFIX_BASE_LEN;
	}

	for( ; i < len; i++) {
		wchar_t	j = str[i];
		if (j >= L'0' && j <= L'9')
			continue;
		else if (j >= L'a' && j <= L'f')
			continue;
		else if (j >= L'A' && j <= L'F')
			continue;

		return 0;
	}

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  IsLegalFunctionName
 *  機能概要：  文字列が関数名として適正かを判定します
 *
 *  返値　　：  0/非0=○/×
 *
 *  　　　　　  1/2/3/4/5/6=空文字列/数値のみで構成/先頭が数値もしくは"_"/使えない文字を含んでいる
 *  　　　　　  　システム関数と同名/制御文もしくは演算子と同名
 * -----------------------------------------------------------------------
 */
char	IsLegalFunctionName(const wstring &str)
{
	int	len = str.size();
	if (!len)
		return 1;

	if (IsIntString(str))
		return 2;

//	if (::iswdigit(str[0]) || str[0] == L'_')
	if ((str[0] >= L'0' && str[0] <= L'9') || str[0] == L'_')
		return 3;

	for(int i = 0; i < len; i++) {
		wchar_t	c = str[i];
		if ((c >= (wchar_t)0x0000 && c <= (wchar_t)0x0026) ||
			(c >= (wchar_t)0x0028 && c <= (wchar_t)0x002d) ||
			 c == (wchar_t)0x002f ||
			(c >= (wchar_t)0x003a && c <= (wchar_t)0x0040) ||
			 c == (wchar_t)0x005b ||
			(c >= (wchar_t)0x005d && c <= (wchar_t)0x005e) ||
			 c == (wchar_t)0x0060 ||
			(c >= (wchar_t)0x007b && c <= (wchar_t)0x007f))
			return 4;
	}

	for(int i= 0; i < SYSFUNC_NUM; i++)
		if (!str.compare(sysfunc[i]))
			return 5;

	for(int i= 0; i < FLOWCOM_NUM; i++)
		if (!str.compare(flowcom[i]))
			return 6;
	for(int i= 0; i < FORMULATAG_NUM; i++)
//		if (!str.compare(formulatag[i]))
		if (str.find(formulatag[i]) != -1)
			return 6;

	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  IsLegalVariableName
 *  機能概要：  文字列が変数名として適正かを判定します
 *
 *  返値　　：  0/1～6/16非0=○(グローバル変数)/×/○(ローカル変数)
 *
 *  　　　　　  1/2/3/4/5/6=空文字列/数値のみで構成/先頭が数値/使えない文字を含んでいる
 *  　　　　　  　システム関数と同名/制御文もしくは演算子と同名
 * -----------------------------------------------------------------------
 */
char	IsLegalVariableName(const wstring &str)
{
	int	len = str.size();
	if (!len)
		return 1;

	if (IsIntString(str))
		return 2;

//	if (::iswdigit((int)str[0]))
	if (str[0] >= L'0' && str[0] <= L'9')
		return 3;

	for(int i = 0; i < len; i++) {
		wchar_t	c = str[i];
		if ((c >= (wchar_t)0x0000  && c <= (wchar_t)0x0026) ||
			(c >= (wchar_t)0x0028  && c <= (wchar_t)0x002d) ||
			 c == (wchar_t)0x002f ||
			(c >= (wchar_t)0x003a && c <= (wchar_t)0x0040) ||
			 c == (wchar_t)0x005b ||
			(c >= (wchar_t)0x005d && c <= (wchar_t)0x005e) ||
			 c == (wchar_t)0x0060 ||
			(c >= (wchar_t)0x007b && c <= (wchar_t)0x007f))
			return 4;
	}

	for(int i= 0; i < SYSFUNC_NUM; i++)
		if (!str.compare(sysfunc[i]))
			return 5;

	for(int i= 0; i < FLOWCOM_NUM; i++)
		if (!str.compare(flowcom[i]))
			return 6;
	for(int i= 0; i < FORMULATAG_NUM; i++)
//		if (!str.compare(formulatag[i]))
		if (str.find(formulatag[i]) != -1)
			return 6;

	return (str[0] == L'_') ? 16 : 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  IsLegalStrLiteral
 *  機能概要：  ダブルクォートで囲まれているべき文字列の正当性を検査します
 *
 *  返値　　：  0/1/2/3=正常/ダブルクォートが閉じていない/
 *  　　　　　  　ダブルクォートで囲まれているがその中にダブルクォートが包含されている/
 *  　　　　　  　ダブルクォートで囲まれていない
 * -----------------------------------------------------------------------
 */
char	IsLegalStrLiteral(const wstring &str)
{
	int	len = str.size();
	if (!len)
		return 3;

	// 先頭のダブルクォートチェック
	int	flg = (str[0] == L'\"') ? 1 : 0;
	// 後端のダブルクォートチェック
	if (len > 1)
		if (str[len - 1] == L'\"')
			flg += 2;
	// 内包されているダブルクォートの探索
	if (len > 2) {
		int	lenm1 = len - 1;
		for(int i = 1; i < lenm1; i++)
			if (str[i] == L'\"') {
				flg = 4;
				break;
			}
	}

	// 結果を返します
	switch(flg) {
	case 3:
		return 0;
	case 1:
	case 2:
	case 5:
	case 6:
		return 1;
	case 7:
		return 2;
	default:
		return 3;
	};
}

/* -----------------------------------------------------------------------
 *  関数名  ：  IsLegalPlainStrLiteral
 *  機能概要：  シングルクォートで囲まれているべき文字列の正当性を検査します
 *
 *  返値　　：  0/1/2/3=正常/ダブルクォートが閉じていない/
 *  　　　　　  　ダブルクォートで囲まれているがその中にダブルクォートが包含されている/
 *  　　　　　  　ダブルクォートで囲まれていない
 * -----------------------------------------------------------------------
 */
char	IsLegalPlainStrLiteral(const wstring &str)
{
	int	len = str.size();
	if (!len)
		return 3;

	// 先頭のシングルクォートチェック
	int	flg = (str[0] == L'\'') ? 1 : 0;
	// 後端のシングルクォートチェック
	if (len > 1)
		if (str[len - 1] == L'\'')
			flg += 2;
	// 内包されているシングルクォートの探索
	if (len > 2) {
		int	lenm1 = len - 1;
		for(int i = 1; i < lenm1; i++)
			if (str[i] == L'\'') {
				flg = 4;
				break;
			}
	}

	// 結果を返します
	switch(flg) {
	case 3:
		return 0;
	case 1:
	case 2:
	case 5:
	case 6:
		return 1;
	case 7:
		return 2;
	default:
		return 3;
	};
}

/* -----------------------------------------------------------------------
 *  関数名  ：  GetFunctionIndexFromName
 *  機能概要：  関数名に対応するfunction配列の序数を取得します
 * -----------------------------------------------------------------------
 */
int	GetFunctionIndexFromName(const wstring& str)
{
	int i = function_wm.search(str, 0);
	if((i != -1) && !function[i].name.compare(str)) {
		// strの最初が関数名にマッチした場合にWordMatchは-1以外を返すので，
		// 完全一致かどうか再度チェックが必要．
		return i;
	}
	return -1;
/*
	int	sz = function.size();
	for(int i = 0; i < sz; i++)
		if (!function[i].name.compare(str))
			return i;

	return -1;
*/
}

/* -----------------------------------------------------------------------
 *  関数名  ：  ToFullPath
 *  機能概要：  渡された文字列が相対パス表記なら絶対パスに書き換えます
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
wstring	ToFullPath(wstring &str)
{
	wchar_t	drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_wsplitpath(str.c_str(), drive, dir, fname, ext);

	if (!::wcslen(drive))
		return basis.path + str;

	return str;
}
#else
wstring ToFullPath(wstring &str) {
    if (str.length() > 0 && str[0] == L'/') {
	return str;
    }
    else {
	return basis.path + str;
    }
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  IsNt
 *  機能概要：  0/1=9x系/NT系　を返します
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
char	IsNt(void)
{
	OSVERSIONINFO	osi = { sizeof(OSVERSIONINFO) };

	GetVersionEx(&osi);
	return (osi.dwPlatformId == VER_PLATFORM_WIN32_NT) ? 1 : 0;
}
#endif
