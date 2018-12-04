// 
// AYA version 5
//
// �����R�[�h�ϊ��N���X�@Ccct
//
// �ϊ������̃R�[�h�͈ȉ��̃T�C�g�Ō��J����Ă�����̂𗘗p���Ă���܂��B
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
 *  �֐���  �F  Ccct::Ucs2ToMbcs
 *  �@�\�T�v�F  UTF-16BE -> MBCS �֕�����̃R�[�h�ϊ�
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
 *  �֐���  �F  Ccct::MbcsToUcs2
 *  �@�\�T�v�F  MBCS -> UTF-16BE �֕�����̃R�[�h�ϊ�
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
 *  �֐���  �F  Ccct::utf16be_to_sjis
 *  �@�\�T�v�F  UTF-16BE -> MBCS �֕�����̃R�[�h�ϊ�
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
        pUcsStr++; // �擪��BOM(byte Order Mark)������΁C�X�L�b�v����
        nLen--;
    }

    pAnsiStr = (char *)malloc((nLen + 1)*sizeof(wchar_t));
    if (!pAnsiStr) return NULL;

    // 1�������ϊ�����B
    // �܂Ƃ߂ĕϊ�����ƁA�ϊ��s�\�����ւ̑Ή�������Ȃ̂�
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
        default: // �ϊ��s�\
            pAnsiStr[nMbpos++] = ' ';
            break;
        }
    }
    pAnsiStr[nMbpos] = '\0';

    delete [] pcMbchar;

    return pAnsiStr;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  Ccct::mbcs_to_utf16be
 *  �@�\�T�v�F  MBCS -> UTF-16 �֕�����̃R�[�h�ϊ�
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
 *  �֐���  �F  Ccct::utf16be_to_utf8
 *  �@�\�T�v�F  UTF-16 -> UTF-8 �֕�����̃R�[�h�ϊ�
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
 *  �֐���  �F  Ccct::utf16be_to_utf8_sub
 *  �@�\�T�v�F  UTF-16 -> UTF-8�ϊ��iutf16be_to_utf8�Ŏg�p���܂��j
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
            pUtf8[nUtf8] = ((pUcs2[nUcs2] & 0x07C0) >> 6 ) | 0xc0; // 2002.08.17 �C��
            pUtf8[nUtf8+1] = (pUcs2[nUcs2] & 0x003f) | 0x80;
            nUtf8 += 2;
        } else {
            pUtf8[nUtf8] = ((pUcs2[nUcs2] & 0xf000) >> 12) | 0xe0; // 2002.08.04 �C��
            pUtf8[nUtf8+1] = ((pUcs2[nUcs2] & 0x0fc0) >> 6) | 0x80;
            pUtf8[nUtf8+2] = (pUcs2[nUcs2] & 0x003f) | 0x80;
            nUtf8 += 3;
        }
    }

    pUtf8[nUtf8] = '\0';

    return nUtf8;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  Ccct::utf8_to_utf16be
 *  �@�\�T�v�F  UTF-8 -> UTF-16BE �֕�����̃R�[�h�ϊ�
 * -----------------------------------------------------------------------
 */
wchar_t *Ccct::utf8_to_utf16be(const char *pUtf8Str)
{
    int nUtf8Num;
    wchar_t *pUcsStr;

    nUtf8Num = strlen(pUtf8Str); // UTF-8������ɂ́C'\0' ���Ȃ�

	pUcsStr = (wchar_t *)malloc((nUtf8Num + 1)*sizeof(wchar_t));
	utf8_to_utf16be_sub( pUcsStr, pUtf8Str, nUtf8Num);

    return pUcsStr;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  Ccct::utf8_to_utf16be_sub
 *  �@�\�T�v�F  UTF-8 -> UTF-16BE�ϊ��iutf16be_to_utf8�Ŏg�p���܂��j
 * -----------------------------------------------------------------------
 */
int Ccct::utf8_to_utf16be_sub( wchar_t *pUcs2, const char *pUtf8, int nUtf8Num)
{
    int	nUcs2 = 0;

    for (int nUtf8 = 0; nUtf8 < nUtf8Num; ) {
        if ( ( pUtf8[nUtf8] & 0x80) == 0x00) { // �ŏ�ʃr�b�g = 0
            pUcs2[nUcs2] = ( pUtf8[nUtf8] & 0x7f);
            nUtf8 += 1;
        } else if ((pUtf8[nUtf8] & 0xe0) == 0xc0) { // ���3�r�b�g = 110
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
 *  �֐���  �F  Ccct::sys_setlocale
 *  �@�\�T�v�F  OS�f�t�H���g�̌���ID�Ń��P�[���ݒ肷��
 *  �@�@�@�@�@  �悭�킩��Ȃ��̂łƂ肠�������ؒ��p�����ɓƘI����
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

