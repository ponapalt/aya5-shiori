// 
// AYA version 5
//
// stl::wstringをchar*風に使うための関数など
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#else
#  include "posix_utils.h"
#  include "messages.h"
#  include <wchar.h>
#  include <iostream>
#  include <sstream>
#  include <iomanip>
#  include <boost/lexical_cast.hpp>
using namespace std;
using namespace boost;
#endif
#include "wsex.h"
#include "ccct.h"
#include "misc.h"
#include "log.h"
#include "basis.h"

extern CBasis	basis;
extern CLog		logger;

/* -----------------------------------------------------------------------
 *  関数名  ：  ws_atoi
 *  機能概要：  wstringをintへ変換
 * -----------------------------------------------------------------------
 */
int	ws_atoi(const wstring &str, int base)
{
	int	len = str.size();
	if (!len)
		return 0;

	wchar_t	*dmy;
	return (int)wcstol(str.c_str(), &dmy, base);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  ws_atof
 *  機能概要：  wstringをdoubleへ変換
 * -----------------------------------------------------------------------
 */
double	ws_atof(const wstring &str)
{
	int	len = str.size();
	if (!len)
		return 0.0;

	wchar_t	*dmy;
	return wcstod(str.c_str(), &dmy);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  ws_itoa
 *  機能概要：  intをwstringへ変換
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
void	ws_itoa(wstring &str, int num, int base)
{
	wchar_t	tmpstr[WS_MAXLEN];
	_itow(num, tmpstr, base);
	str = tmpstr;
}
#else
void ws_itoa(wstring &str, int num, int base) {
    ostringstream s;
    s << setbase(base) << num;
    str = widen(s.str());
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  ws_ftoa
 *  機能概要：  doubleをwstringへ変換
 *
 * どうすればいいのか分からなかったので、まずswprintfで変換してから
 * 終端の無駄なL'0'を削るという謎処理にしています
 * -----------------------------------------------------------------------
 */
void	ws_ftoa(wstring &str, double num)
{
	wchar_t	tmpstr[WS_MAXLEN];
#ifndef POSIX
	swprintf(tmpstr, L"%lf", num);
#else
	swprintf(tmpstr, WS_MAXLEN, L"%lf", num);
#endif
	str = tmpstr;

	for( ; ; ) {
		if (str.size() < 4)
			break;
		wchar_t	lastword = str[str.size() - 1];
		if (lastword == L'0')
			str.erase(str.end() - 1);
		else {
			if (lastword == L',')
				str += L'0';
			break;
		}
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  ws_eraseend
 *  機能概要：  wstringの終端からcを削る
 * -----------------------------------------------------------------------
 */
void	ws_eraseend(wstring &str, wchar_t c)
{
	int	len = str.size();
	if (!len)
		return;

	if (str[len - 1] == c)
		str.erase(str.end() - 1);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  ws_replace
 *  機能概要：  str内のbeforeをすべてafterに置換します
 * -----------------------------------------------------------------------
 */
void	ws_replace(wstring &str, const wstring &before, const wstring &after)
{
	int	sz_bef = before.size();
	int	sz_aft = after.size();
	for(int rp_pos = 0; ; rp_pos += sz_aft) {
		rp_pos = str.find(before, rp_pos);
		if (rp_pos == -1)
			break;
		str.replace(rp_pos, sz_bef, after);
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  w_fopen
 *  機能概要：  UCS-2文字列のファイル名でオープンできるfopen
 *
 *  補足　wchar_t*を直接渡せる_wfopenはWin9x系未サポートのため使えないのです。無念。
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
FILE	*w_fopen(const wchar_t *fname, const wchar_t *mode)
{
	// ファイル名とオープンモードををMBCSへ変換
	char	*mfname = Ccct::Ucs2ToMbcs(fname, CHARSET_DEFAULT);
	if (mfname == NULL)
		return NULL;
	char	*mmode  = Ccct::Ucs2ToMbcs(mode,  CHARSET_DEFAULT);
	if (mmode == NULL) {
		free(mfname);
		return NULL;
	}
	// オープン
	FILE	*fp = fopen(mfname, mmode);
	free(mfname);
	free(mmode);

	return fp;
}
#else
FILE* w_fopen(const wchar_t* fname, const wchar_t* mode) {
    string s_fname = narrow(wstring(fname));
    string s_mode = narrow(wstring(mode));

    fix_filepath(s_fname);

    return fopen(s_fname.c_str(), s_mode.c_str());
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  write_utf8bom
 *  機能概要：  UTF-8 BOMを書き込む
 * -----------------------------------------------------------------------
 */
/*
void	write_utf8bom(FILE *fp)
{
	fputc(0xef, fp);
	fputc(0xbb, fp);
	fputc(0xbf, fp);
}
*/
/* -----------------------------------------------------------------------
 *  関数名  ：  ws_fgets
 *  機能概要：  wstringに取り出せる簡易版fgets、暗号復号とUCS-2 BOM削除も行なう
 * -----------------------------------------------------------------------
 */
int ws_fgets(wstring &str, FILE *stream, int charset, int ayc/*1でAYC復号する*/, int lc/*1でBOMチェックON*/) {
    string buf;
    int c;
    while (true) {
	c = fgetc(stream);
	if (c == EOF) {
	    break;
	}
	if (ayc) {
	    decodecipher(c);
	}
	buf += static_cast<char>(c);
	if (c == '\x0a') {
	    // 行の終わり
	    break;
	}
    }

    wchar_t *wstr = Ccct::MbcsToUcs2(buf, charset);
    str = wstr;
    free(wstr);

    if (charset == CHARSET_UTF8 && lc == 1) {
	cutbom(str);
    }

    if (c == EOF && str.empty()) {
	return WS_EOF;
    }
    else {
	return str.size();
    }
}

/* -----------------------------------------------------------------------
 *  関数名  ：  cutbom
 *  機能概要：  wstring先頭にUCS-2 BOMがあったら取り除く
 * -----------------------------------------------------------------------
 */
void	cutbom(wstring &wstr)
{
	if (wstr.size() < 2)
		return;
	if (wstr[0] == (wchar_t)0xfeff || wstr[0] == (wchar_t)0xffef)
		wstr.erase(0, 1);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  decodecipher
 *  機能概要：  AYA暗号化された文字を復号する
 *
 *  ただのビット反転とかき混ぜです
 * -----------------------------------------------------------------------
 */
inline void	decodecipher(int &c)
{
	c = (((c & 0x7) << 5) + (c >> 3)) ^ 0x5a;
}

