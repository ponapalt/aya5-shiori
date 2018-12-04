// 
// AYA version 5
//
// stl::wstring��char*���Ɏg�����߂̊֐��Ȃ�
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
 *  �֐���  �F  ws_atoi
 *  �@�\�T�v�F  wstring��int�֕ϊ�
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
 *  �֐���  �F  ws_atof
 *  �@�\�T�v�F  wstring��double�֕ϊ�
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
 *  �֐���  �F  ws_itoa
 *  �@�\�T�v�F  int��wstring�֕ϊ�
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
 *  �֐���  �F  ws_ftoa
 *  �@�\�T�v�F  double��wstring�֕ϊ�
 *
 * �ǂ�����΂����̂�������Ȃ������̂ŁA�܂�swprintf�ŕϊ����Ă���
 * �I�[�̖��ʂ�L'0'�����Ƃ����䏈���ɂ��Ă��܂�
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
 *  �֐���  �F  ws_eraseend
 *  �@�\�T�v�F  wstring�̏I�[����c�����
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
 *  �֐���  �F  ws_replace
 *  �@�\�T�v�F  str����before�����ׂ�after�ɒu�����܂�
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
 *  �֐���  �F  w_fopen
 *  �@�\�T�v�F  UCS-2������̃t�@�C�����ŃI�[�v���ł���fopen
 *
 *  �⑫�@wchar_t*�𒼐ړn����_wfopen��Win9x�n���T�|�[�g�̂��ߎg���Ȃ��̂ł��B���O�B
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
FILE	*w_fopen(const wchar_t *fname, const wchar_t *mode)
{
	// �t�@�C�����ƃI�[�v�����[�h����MBCS�֕ϊ�
	char	*mfname = Ccct::Ucs2ToMbcs(fname, CHARSET_DEFAULT);
	if (mfname == NULL)
		return NULL;
	char	*mmode  = Ccct::Ucs2ToMbcs(mode,  CHARSET_DEFAULT);
	if (mmode == NULL) {
		free(mfname);
		return NULL;
	}
	// �I�[�v��
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
 *  �֐���  �F  write_utf8bom
 *  �@�\�T�v�F  UTF-8 BOM����������
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
 *  �֐���  �F  ws_fgets
 *  �@�\�T�v�F  wstring�Ɏ��o����ȈՔ�fgets�A�Í�������UCS-2 BOM�폜���s�Ȃ�
 * -----------------------------------------------------------------------
 */
int ws_fgets(wstring &str, FILE *stream, int charset, int ayc/*1��AYC��������*/, int lc/*1��BOM�`�F�b�NON*/) {
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
	    // �s�̏I���
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
 *  �֐���  �F  cutbom
 *  �@�\�T�v�F  wstring�擪��UCS-2 BOM�����������菜��
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
 *  �֐���  �F  decodecipher
 *  �@�\�T�v�F  AYA�Í������ꂽ�����𕜍�����
 *
 *  �����̃r�b�g���]�Ƃ��������ł�
 * -----------------------------------------------------------------------
 */
inline void	decodecipher(int &c)
{
	c = (((c & 0x7) << 5) + (c >> 3)) ^ 0x5a;
}

