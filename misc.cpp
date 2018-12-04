// 
// AYA version 5
//
// �G�p�֐�
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
 *  �֐���  �F  Split
 *  �@�\�T�v�F  ������𕪊����ė]���ȋ󔒂��폜���܂�
 *
 *  �Ԓl�@�@�F  0/1=���s/����
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
 *  �֐���  �F  SplitOnly
 *  �@�\�T�v�F  ������𕪊����܂�
 *
 *  �Ԓl�@�@�F  0/1=���s/����
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
 *  �֐���  �F  Split_IgnoreDQ
 *  �@�\�T�v�F  ������𕪊����ė]���ȋ󔒂��폜���܂�
 *  �@�@�@�@�@  �������_�u��/�V���O���N�H�[�g���ł͕������܂���
 *
 *  �Ԓl�@�@�F  0/1=���s/����
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
 *  �֐���  �F  SplitToMultiString
 *  �@�\�T�v�F  ������𕪊�����vector�Ɋi�[���܂�
 *
 *�@�Ԓl�@�@�F�@������(array.size())
 * -----------------------------------------------------------------------
 */
int	SplitToMultiString(const wstring &str, vector<wstring> &array, const wstring &delimiter)
{
	if (!str.size())
		return 0;

	wstring	t_str = str;
	int	dlmlen    = delimiter.size();
	for( ; ; ) {
		// �f���~�^�̔���
		int	seppoint = t_str.find(delimiter);
		if (seppoint == -1) {
			array.push_back(t_str);
			break;
		}
		// ���o����vector�ւ̒ǉ�
		wstring	i_str;
		i_str.assign(t_str, 0, seppoint);
		array.push_back(i_str);
		// ���o���������폜
		t_str.erase(0, seppoint + dlmlen);
	}

	return array.size();
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CutSpace
 *  �@�\�T�v�F  �^����ꂽ������̑O��ɔ��p�󔒂��^�u���������ꍇ�A���ׂč폜���܂�
 * -----------------------------------------------------------------------
 */
void	CutSpace(wstring &str)
{
	int	len = str.size();
	// �O��
	int	erasenum = 0;
	for(int i = 0; i < len; i++) {
#ifndef POSIX
		if (str[i] == L' ' || str[i] == L'\t' || str[i] == L'�@')
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
	// ���
	erasenum = 0;
	for(int i = len - 1; i >= 0; i--) {
#ifndef POSIX
		if (str[i] == L' ' || str[i] == L'\t' || str[i] == L'�@')
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
 *  �֐���  �F  CutDoubleQuote
 *  �@�\�T�v�F  �^����ꂽ������̑O��Ƀ_�u���N�H�[�g���������ꍇ�폜���܂�
 * -----------------------------------------------------------------------
 */
void	CutDoubleQuote(wstring &str)
{
	int	len = str.size();
	if (!len)
		return;
	// �O��
	if (str[0] == L'\"') {
		str.erase(0, 1);
		len--;
		if (!len)
			return;
	}
	// ���
	if (str[len - 1] == L'\"')
		str.erase(len - 1, 1);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CutSingleQuote
 *  �@�\�T�v�F  �^����ꂽ������̑O��ɃV���O���N�H�[�g���������ꍇ�폜���܂�
 * -----------------------------------------------------------------------
 */
void	CutSingleQuote(wstring &str)
{
	int	len = str.size();
	if (!len)
		return;
	// �O��
	if (str[0] == L'\'') {
		str.erase(0, 1);
		len--;
		if (!len)
			return;
	}
	// ���
	if (str[len - 1] == L'\'')
		str.erase(len - 1, 1);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  AddDoubleQuote
 *  �@�\�T�v�F  �^����ꂽ��������_�u���N�H�[�g�ň݂͂܂�
 * -----------------------------------------------------------------------
 */
void	AddDoubleQuote(wstring &str)
{
	str = L"\"" + str + L"\"";
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CutCrLf
 *  �@�\�T�v�F  �^����ꂽ������̌�[�ɉ��s(CRLF)���������ꍇ�폜���܂�
 * -----------------------------------------------------------------------
 */
void	CutCrLf(wstring &str)
{
	ws_eraseend(str, L'\n');
	ws_eraseend(str, L'\r');
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  GetDateString
 *  �@�\�T�v�F  �N����/�����b�̕�������쐬���ĕԂ��܂�
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
wstring	GetDateString(void)
{
	wstring	result;
	
	// �N����
	wchar_t	_datebuffer[32];
	_wstrdate(_datebuffer);
	result = _datebuffer;
	result += (basis.GetMsgLang()) ? L"(m/d/y) " : L"(��/��/�N) ";
	// �����b
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
 *  �֐���  �F  IsInDQ
 *  �@�\�T�v�F  ��������̎w��ʒu���_�u��/�V���O���N�H�[�g�͈͓������`�F�b�N���܂�
 *
 *  �Ԓl�@�@�F  0/1=�_�u��/�V���O���N�H�[�g�̊O��/����
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
 *  �֐���  �F  IsDoubleString
 *  �@�\�T�v�F  �����񂪎������l�Ƃ��Đ��������������܂�
 *
 *  �Ԓl�@�@�F  0/1=�~/��
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
 *  �֐���  �F  IsIntString
 *  �@�\�T�v�F  ������10�i�������l�Ƃ��Đ��������������܂�
 *
 *  �Ԓl�@�@�F  0/1=�~/��
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
 *  �֐���  �F  IsIntBinString
 *  �@�\�T�v�F  ������2�i�������l�Ƃ��Đ��������������܂�
 *  �����@�@�F  header 0/1=�擪"0x"�Ȃ�/����
 *
 *  �Ԓl�@�@�F  0/1=�~/��
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
 *  �֐���  �F  IsIntHexString
 *  �@�\�T�v�F  ������16�i�������l�Ƃ��Đ��������������܂�
 *
 *  �Ԓl�@�@�F  0/1=�~/��
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
 *  �֐���  �F  IsLegalFunctionName
 *  �@�\�T�v�F  �����񂪊֐����Ƃ��ēK�����𔻒肵�܂�
 *
 *  �Ԓl�@�@�F  0/��0=��/�~
 *
 *  �@�@�@�@�@  1/2/3/4/5/6=�󕶎���/���l�݂̂ō\��/�擪�����l��������"_"/�g���Ȃ��������܂�ł���
 *  �@�@�@�@�@  �@�V�X�e���֐��Ɠ���/���䕶�������͉��Z�q�Ɠ���
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
 *  �֐���  �F  IsLegalVariableName
 *  �@�\�T�v�F  �����񂪕ϐ����Ƃ��ēK�����𔻒肵�܂�
 *
 *  �Ԓl�@�@�F  0/1�`6/16��0=��(�O���[�o���ϐ�)/�~/��(���[�J���ϐ�)
 *
 *  �@�@�@�@�@  1/2/3/4/5/6=�󕶎���/���l�݂̂ō\��/�擪�����l/�g���Ȃ��������܂�ł���
 *  �@�@�@�@�@  �@�V�X�e���֐��Ɠ���/���䕶�������͉��Z�q�Ɠ���
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
 *  �֐���  �F  IsLegalStrLiteral
 *  �@�\�T�v�F  �_�u���N�H�[�g�ň͂܂�Ă���ׂ�������̐��������������܂�
 *
 *  �Ԓl�@�@�F  0/1/2/3=����/�_�u���N�H�[�g�����Ă��Ȃ�/
 *  �@�@�@�@�@  �@�_�u���N�H�[�g�ň͂܂�Ă��邪���̒��Ƀ_�u���N�H�[�g����܂���Ă���/
 *  �@�@�@�@�@  �@�_�u���N�H�[�g�ň͂܂�Ă��Ȃ�
 * -----------------------------------------------------------------------
 */
char	IsLegalStrLiteral(const wstring &str)
{
	int	len = str.size();
	if (!len)
		return 3;

	// �擪�̃_�u���N�H�[�g�`�F�b�N
	int	flg = (str[0] == L'\"') ? 1 : 0;
	// ��[�̃_�u���N�H�[�g�`�F�b�N
	if (len > 1)
		if (str[len - 1] == L'\"')
			flg += 2;
	// �����Ă���_�u���N�H�[�g�̒T��
	if (len > 2) {
		int	lenm1 = len - 1;
		for(int i = 1; i < lenm1; i++)
			if (str[i] == L'\"') {
				flg = 4;
				break;
			}
	}

	// ���ʂ�Ԃ��܂�
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
 *  �֐���  �F  IsLegalPlainStrLiteral
 *  �@�\�T�v�F  �V���O���N�H�[�g�ň͂܂�Ă���ׂ�������̐��������������܂�
 *
 *  �Ԓl�@�@�F  0/1/2/3=����/�_�u���N�H�[�g�����Ă��Ȃ�/
 *  �@�@�@�@�@  �@�_�u���N�H�[�g�ň͂܂�Ă��邪���̒��Ƀ_�u���N�H�[�g����܂���Ă���/
 *  �@�@�@�@�@  �@�_�u���N�H�[�g�ň͂܂�Ă��Ȃ�
 * -----------------------------------------------------------------------
 */
char	IsLegalPlainStrLiteral(const wstring &str)
{
	int	len = str.size();
	if (!len)
		return 3;

	// �擪�̃V���O���N�H�[�g�`�F�b�N
	int	flg = (str[0] == L'\'') ? 1 : 0;
	// ��[�̃V���O���N�H�[�g�`�F�b�N
	if (len > 1)
		if (str[len - 1] == L'\'')
			flg += 2;
	// �����Ă���V���O���N�H�[�g�̒T��
	if (len > 2) {
		int	lenm1 = len - 1;
		for(int i = 1; i < lenm1; i++)
			if (str[i] == L'\'') {
				flg = 4;
				break;
			}
	}

	// ���ʂ�Ԃ��܂�
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
 *  �֐���  �F  GetFunctionIndexFromName
 *  �@�\�T�v�F  �֐����ɑΉ�����function�z��̏������擾���܂�
 * -----------------------------------------------------------------------
 */
int	GetFunctionIndexFromName(const wstring& str)
{
	int i = function_wm.search(str, 0);
	if((i != -1) && !function[i].name.compare(str)) {
		// str�̍ŏ����֐����Ƀ}�b�`�����ꍇ��WordMatch��-1�ȊO��Ԃ��̂ŁC
		// ���S��v���ǂ����ēx�`�F�b�N���K�v�D
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
 *  �֐���  �F  ToFullPath
 *  �@�\�T�v�F  �n���ꂽ�����񂪑��΃p�X�\�L�Ȃ��΃p�X�ɏ��������܂�
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
 *  �֐���  �F  IsNt
 *  �@�\�T�v�F  0/1=9x�n/NT�n�@��Ԃ��܂�
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
