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
// 利用制限等はなさそうだったのでそのまま使わせていただきました。
//
// 細かい点はわたしのほうで弄りましたが、肝心の読み替え部分はオリジナルのままです。
//
// 変更点
// ・オリジナルはロケールJapaneseのみ対応だったので、OSデフォルト言語を調べてsetlocaleする機能を追加。
// ・MBC to UCS2、及びその逆を行う関数を差し替えました。
// ・高速化のための修正を若干。
// 　1.オリジナルでは変換前に必要な領域のサイズをしっかり調べていましたが、その処理は省き、どのように
// 　  変換されても収まるだけの領域を用意するようにしました。
// 　2.オリジナルはcallocでしたが、極力mallocへ置き換えました。若干速い（はず）。
//

#ifndef CCCTH
#define CCCTH

#ifndef POSIX
#  include "stdafx.h"
#else
#  include <string>
using namespace std;
#endif

//----

class Ccct {
public:
	static char		*Ucs2ToMbcs(const wchar_t *wstr, int charset);
	static char		*Ucs2ToMbcs(const wstring &wstr, int charset);
	static wchar_t	*MbcsToUcs2(const char *mstr, int charset);
	static wchar_t	*MbcsToUcs2(const string &mstr, int charset);

#ifndef POSIX
    static wchar_t*	mbcs_to_utf16be(const char *pAnsiStr, char charset);
    static char*	utf16be_to_utf8(const wchar_t *pUcsStr);
    static char*	utf16be_to_mbcs(const wchar_t *pUcsStr, char charset);
    static wchar_t*	utf8_to_utf16be(const char *pUtf8Str);

protected:
    static int		utf16be_to_utf8_sub( char *pUtf8, const wchar_t *pUcs2, int nUcsNum);
    static int		utf8_to_utf16be_sub( wchar_t *pUcs2, const char *pUtf8, int nUtf8Num);
	static char		*sys_setlocale(int category);
#endif
};

//----

#endif
