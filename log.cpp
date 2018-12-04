// 
// AYA version 5
//
// ���M���O�p�N���X�@CLog
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
 *  �֐���  �F  CLog::Start
 *  �@�\�T�v�F  ���M���O���J�n���܂�
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
	// ����hWnd��NULL�Ȃ�N�����̃`�F�b�N�c�[����T���Ď擾����
	if (hWnd == NULL)
		hWnd = GetCheckerWnd();
#endif

	// ���M���O�L��/�����̔���
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

	// ������쐬
	wstring	str = (msglang) ? msge[0] : msgj[0];
	str += GetDateString();
	str += L"\n\n";

	// �t�@�C���֏�������
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
	// �`�F�b�N�c�[���֑��o�@�ŏ��ɕ����R�[�h��ݒ肵�Ă��當����𑗏o
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
 *  �֐���  �F  CLog::Termination
 *  �@�\�T�v�F  ���M���O���I�����܂�
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
 *  �֐���  �F  CLog::Write
 *  �@�\�T�v�F  ���O�ɕ�������������݂܂�
 * -----------------------------------------------------------------------
 */
void	CLog::Write(const wstring &str, int mode)
{
	if (!enable)
		return;

	// �����񒆂�\r�͏���
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

	// �t�@�C���֏�������
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
	// �`�F�b�N�c�[���֑��o
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
 *  �֐���  �F  CLog::Filename
 *  �@�\�T�v�F  ����̃t�H�[�}�b�g�Ńt�@�C���������O�ɋL�^���܂�
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
 *  �֐���  �F  CLog::Message
 *  �@�\�T�v�F  id�Ŏw�肳�ꂽ����̃��b�Z�[�W�����O�ɏ������݂܂�
 * -----------------------------------------------------------------------
 */
void	CLog::Message(int id)
{
	Write((msglang) ? (wchar_t *)msge[id] : (wchar_t *)msgj[id], 0);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLog::Error
 *  �@�\�T�v�F  ���O��mode��id�Ŏw�肳�ꂽ�G���[��������������݂܂�
 *
 *  �����@�@�F�@ref         �t�����
 *  �@�@�@�@�@  dicfilename �G���[���N�������ӏ����܂ގ����t�@�C���̖��O
 *  �@�@�@�@�@  linecount   �G���[���N�������s�ԍ�
 *
 *  �@�@�@�@�@  ref��dicfilename��NULL�Alinecount��-1�Ƃ��邱�ƂŁA������
 *  �@�@�@�@�@  ��\���ɂł��܂�
 * -----------------------------------------------------------------------
 */
void	CLog::Error(int mode, int id, const wchar_t *ref, const wstring &dicfilename, int linecount)
{
	if (!enable)
		return;

	// ���O�ɏ������ݕ�������쐬�i�����t�@�C�����ƍs�ԍ��j
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
	// ���O�ɏ������ݕ�������쐬�i�{���j
	if (msglang) {
		// �p��
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
		// ���{��
		if (mode == E_F)
			logstr += msgfj[id];
		else if (mode == E_E)
			logstr += msgej[id];
		else if (mode == E_W)
			logstr += msgwj[id];
		else
			logstr += msgnj[id];
	}
	// ���O�ɏ������ݕ�������쐬�i�t�����j
	if (ref != NULL) {
		logstr += L" : ";
		logstr += ref;
	}
	// ��������
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
 *  �֐���  �F  CLog::Io
 *  �@�\�T�v�F  ���o�͕�����Ǝ��s���Ԃ����O�ɋL�^���܂�
 *  �����@�@�F  io 0/1=�J�n��/�I����
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
 *  �֐���  �F  CLog::IoLib
 *  �@�\�T�v�F  �O�����C�u�������o�͕�����Ǝ��s���Ԃ����O�ɋL�^���܂�
 *  �����@�@�F  io 0/1=�J�n��/�I����
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
 *  �֐���  �F  CLog::SendLogToWnd
 *  �@�\�T�v�F  �`�F�b�N�c�[���ɐ��䃁�b�Z�[�W����у��O�������WM_COPYDATA�ő��M���܂�
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
 *  �֐���  �F  CLog::GetCheckerWnd
 *  �@�\�T�v�F  �`�F�b�N�c�[����hWnd���擾���܂���
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
HWND	CLog::GetCheckerWnd(void)
{
	return FindWindow(CLASSNAME_CHECKTOOL, NULL);
}
#endif
