// 
// AYA version 5
//
// 文字コード変換クラス　Ccct
//
// 変換部分のコードは以下のサイトで公開されているものを利用しております。
// class CUnicodeF
// kamoland
// http://kamoland.com/comp/unicode.html
//

#include "StdAfx.h"
#include "ccct.h"

/*
#define PRIMARYLANGID(lgid)    ((WORD)(lgid) & 0x3ff)
*/

/* -----------------------------------------------------------------------
 *  関数名  ：  Ccct::Ucs2ToMbcs
 *  機能概要：  UTF-16BE -> MBCS へ文字列のコード変換
 *
 * (written by umeici)
 * -----------------------------------------------------------------------
 */
char	*Ccct::Ucs2ToMbcs(const wchar_t *wstr, int charset)
{
	if (charset == CHARSET_UTF8)
		return Ccct::utf16be_to_utf8(wstr);
	else
		return Ccct::utf16be_to_mbcs(wstr, charset);
}

//----

char	*Ccct::Ucs2ToMbcs(const wstring &wstr, int charset)
{
	return Ucs2ToMbcs((wchar_t *)wstr.c_str(), charset);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  Ccct::MbcsToUcs2
 *  機能概要：  MBCS -> UTF-16BE へ文字列のコード変換
 *
 * (written by umeici)
 * -----------------------------------------------------------------------
 */
wchar_t	*Ccct::MbcsToUcs2(const char *mstr, int charset)
{
	if (charset == CHARSET_UTF8)
		return Ccct::utf8_to_utf16be(mstr);
	else
		return Ccct::mbcs_to_utf16be(mstr, charset);
}

//----

wchar_t	*Ccct::MbcsToUcs2(const string &mstr, int charset)
{
	return MbcsToUcs2((char *)mstr.c_str(), charset);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  Ccct::utf16be_to_sjis
 *  機能概要：  UTF-16BE -> MBCS へ文字列のコード変換
 * -----------------------------------------------------------------------
 */
char *Ccct::utf16be_to_mbcs(const wchar_t *pUcsStr, char charset)
{
    char *pAnsiStr = NULL;
    int nLen;

    if (!pUcsStr)
		return NULL;

	if (charset == CHARSET_SJIS)
		setlocale(LC_CTYPE, "Japanese");
	else
		sys_setlocale(LC_CTYPE);

    nLen = wcslen( pUcsStr);

    if (pUcsStr[0] == (wchar_t)0xfeff || pUcsStr[0] == (wchar_t)0xfffe) {
        pUcsStr++; // 先頭にBOM(byte Order Mark)があれば，スキップする
        nLen--;
    }

    pAnsiStr = (char *)malloc((nLen + 1)*sizeof(wchar_t));
    if (!pAnsiStr) return NULL;

    // 1文字ずつ変換する。
    // まとめて変換すると、変換不能文字への対応が困難なので
    int nRet, i, nMbpos = 0;
    char *pcMbchar = new char[MB_CUR_MAX];

    for (i = 0; i < nLen; i++) {
        nRet = wctomb(pcMbchar, pUcsStr[i]);
        switch ( nRet) {
        case 1:
            pAnsiStr[nMbpos++] = pcMbchar[0];
            break;
        case 2:
            pAnsiStr[nMbpos++] = pcMbchar[0];
            pAnsiStr[nMbpos++] = pcMbchar[1];
            break;
        default: // 変換不能
            pAnsiStr[nMbpos++] = ' ';
            break;
        }
    }
    pAnsiStr[nMbpos] = '\0';

    delete [] pcMbchar;

    return pAnsiStr;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  Ccct::mbcs_to_utf16be
 *  機能概要：  MBCS -> UTF-16 へ文字列のコード変換
 * -----------------------------------------------------------------------
 */
wchar_t *Ccct::mbcs_to_utf16be(const char *pAnsiStr, char charset)
{
    int len;
    wchar_t *pUcsStr = NULL;

    if (!pAnsiStr)
		return NULL;

	if (charset == CHARSET_SJIS)
		setlocale(LC_CTYPE, "Japanese");
	else
		sys_setlocale(LC_CTYPE);

    len = strlen(pAnsiStr);
    int nBytesOut = sizeof(wchar_t)*(len);

    pUcsStr = (wchar_t *)calloc(nBytesOut + 2, sizeof(char));
    if (!pUcsStr)
		return NULL;

	if (len)
		mbstowcs(pUcsStr, pAnsiStr, len+1);
	else
		*pUcsStr = L'\0';

    return pUcsStr;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  Ccct::utf16be_to_utf8
 *  機能概要：  UTF-16 -> UTF-8 へ文字列のコード変換
 * -----------------------------------------------------------------------
 */
char *Ccct::utf16be_to_utf8(const wchar_t *pUcsStr)
{
    int nUcsNum;
    char *pUtf8Str;

    nUcsNum = wcslen(pUcsStr);

    pUtf8Str = (char *)malloc((nUcsNum + 1)*3*sizeof(char));
    utf16be_to_utf8_sub( pUtf8Str, pUcsStr, nUcsNum);

    return pUtf8Str;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  Ccct::utf16be_to_utf8_sub
 *  機能概要：  UTF-16 -> UTF-8変換（utf16be_to_utf8で使用します）
 * -----------------------------------------------------------------------
 */
int Ccct::utf16be_to_utf8_sub( char *pUtf8, const wchar_t *pUcs2, int nUcsNum)
{
    int nUtf8 = 0;

    for (int nUcs2 = 0; nUcs2 < nUcsNum; nUcs2++) {
        if ( (unsigned short)pUcs2[nUcs2] <= 0x007f) {
            pUtf8[nUtf8] = (pUcs2[nUcs2] & 0x007f);
            nUtf8 += 1;
        } else if ( (unsigned short)pUcs2[nUcs2] <= 0x07ff) {
            pUtf8[nUtf8] = ((pUcs2[nUcs2] & 0x07C0) >> 6 ) | 0xc0; // 2002.08.17 修正
            pUtf8[nUtf8+1] = (pUcs2[nUcs2] & 0x003f) | 0x80;
            nUtf8 += 2;
        } else {
            pUtf8[nUtf8] = ((pUcs2[nUcs2] & 0xf000) >> 12) | 0xe0; // 2002.08.04 修正
            pUtf8[nUtf8+1] = ((pUcs2[nUcs2] & 0x0fc0) >> 6) | 0x80;
            pUtf8[nUtf8+2] = (pUcs2[nUcs2] & 0x003f) | 0x80;
            nUtf8 += 3;
        }
    }

    pUtf8[nUtf8] = '\0';

    return nUtf8;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  Ccct::utf8_to_utf16be
 *  機能概要：  UTF-8 -> UTF-16BE へ文字列のコード変換
 * -----------------------------------------------------------------------
 */
wchar_t *Ccct::utf8_to_utf16be(const char *pUtf8Str)
{
    int nUtf8Num;
    wchar_t *pUcsStr;

    nUtf8Num = strlen(pUtf8Str); // UTF-8文字列には，'\0' がない

	pUcsStr = (wchar_t *)malloc((nUtf8Num + 1)*sizeof(wchar_t));
	utf8_to_utf16be_sub( pUcsStr, pUtf8Str, nUtf8Num);

    return pUcsStr;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  Ccct::utf8_to_utf16be_sub
 *  機能概要：  UTF-8 -> UTF-16BE変換（utf16be_to_utf8で使用します）
 * -----------------------------------------------------------------------
 */
int Ccct::utf8_to_utf16be_sub( wchar_t *pUcs2, const char *pUtf8, int nUtf8Num)
{
    int	nUcs2 = 0;

    for (int nUtf8 = 0; nUtf8 < nUtf8Num; ) {
        if ( ( pUtf8[nUtf8] & 0x80) == 0x00) { // 最上位ビット = 0
            pUcs2[nUcs2] = ( pUtf8[nUtf8] & 0x7f);
            nUtf8 += 1;
        } else if ((pUtf8[nUtf8] & 0xe0) == 0xc0) { // 上位3ビット = 110
            pUcs2[nUcs2] = ( pUtf8[nUtf8] & 0x1f) << 6;
            pUcs2[nUcs2] |= ( pUtf8[nUtf8+1] & 0x3f);
            nUtf8 += 2;
        } else {
            pUcs2[nUcs2] = ( pUtf8[nUtf8] & 0x0f) << 12;
            pUcs2[nUcs2] |= ( pUtf8[nUtf8+1] & 0x3f) << 6;
            pUcs2[nUcs2] |= ( pUtf8[nUtf8+2] & 0x3f);
            nUtf8 += 3;
        }

        nUcs2 += 1;
    }

    pUcs2[nUcs2] = L'\0';

    return nUcs2;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  Ccct::sys_setlocale
 *  機能概要：  OSデフォルトの言語IDでロケール設定する
 *  　　　　　  よくわからないのでとりあえず日韓中英仏西伊独露だけ
 *
 * (written by umeici)
 * -----------------------------------------------------------------------
 */
char *Ccct::sys_setlocale(int category)
{
	switch(PRIMARYLANGID(GetSystemDefaultLangID())) {
		case LANG_JAPANESE:
			return setlocale(category, "Japanese");
		case LANG_KOREAN:
			return setlocale(category, "Korean");
		case LANG_CHINESE:
			return setlocale(category, "Chinese");
		case LANG_RUSSIAN:
			return setlocale(category, "Russian");
		case LANG_FRENCH:
			return setlocale(category, "French");
		case LANG_GERMAN:
			return setlocale(category, "German");
		case LANG_SPANISH:
			return setlocale(category, "Spanish");
		case LANG_ITALIAN:
			return setlocale(category, "Italian");
		default:
			return setlocale(category, "English");
	};
}

