// 
// AYA version 5
//
// 1�̃t�@�C���������N���X�@CFile1
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#else
#  include "manifest.h"
#endif
#include "file.h"
#include "misc.h"
#include "ccct.h"
#include "wsex.h"

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFile1::Open
 *  �@�\�T�v�F  �t�@�C�����I�[�v�����܂�
 *
 *  �Ԓl�@�@�F�@0/1=���s/����(���Ƀ��[�h����Ă���܂�)
 * -----------------------------------------------------------------------
 */
int	CFile1::Open(void)
{
	if (fp != NULL)
		return 1;

	char	*filepath = Ccct::Ucs2ToMbcs(name, CHARSET_DEFAULT);
	if (filepath == NULL)
		return 0;

	fp = w_fopen((wchar_t *)name.c_str(), (wchar_t *)mode.c_str());
	free(filepath);
	
	return (fp != NULL) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFile1::Close
 *  �@�\�T�v�F  �t�@�C�����N���[�Y���܂�
 *
 *  �Ԓl�@�@�F�@1/2=����/���[�h����Ă��Ȃ��A�������͊���unload����Ă���
 * -----------------------------------------------------------------------
 */
int	CFile1::Close(void)
{
	if (fp == NULL)
		return 2;

	fclose(fp);

	return 1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFile1::Write
 *  �@�\�T�v�F  �t�@�C���ɕ�������������݂܂�
 *
 *  �Ԓl�@�@�F�@0/1=���s/����
 * -----------------------------------------------------------------------
 */
int	CFile1::Write(const wstring &istr)
{
	if (fp == NULL)
		return 0;

	// ��������}���`�o�C�g�����R�[�h�ɕϊ�
	char	*t_istr = Ccct::Ucs2ToMbcs(istr, charset);
	if (t_istr == NULL)
		return 0;

	long	len = (long)strlen(t_istr);

	// ��������
	fwrite(t_istr, sizeof(char), len, fp);
	free(t_istr);

	return 1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFile1::Read
 *  �@�\�T�v�F  �t�@�C�����當�����1�s�ǂݎ��܂�
 *
 *  �Ԓl�@�@�F�@-1/0/1=EOF/���s/����
 * -----------------------------------------------------------------------
 */
int	CFile1::Read(wstring &ostr)
{
	ostr = L"";

	if (fp == NULL)
		return 0;

	if (ws_fgets(ostr, fp, charset, 0, bomcheck) == WS_EOF)
		return -1;

	bomcheck++;

	return 1;
}
