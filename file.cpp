// 
// AYA version 5
//
// �t�@�C���������N���X�@CFile
// written by umeici. 2004
// 
// write/read�̓x��list����Ώۂ��������Ă��܂����A��x�Ɏ�舵���t�@�C����
// �����Ă������Ǝv���̂ŁA����ł����s���x�ɖ��͂Ȃ��ƍl���Ă��܂��B
//

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "file.h"
#include "misc.h"

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFile::Add
 *  �@�\�T�v�F  �w�肳�ꂽ�t�@�C�����I�[�v�����܂�
 *
 *  �Ԓl�@�@�F�@0/1/2=���s/����/���ɃI�[�v�����Ă���
 * -----------------------------------------------------------------------
 */
int	CFile::Add(const wstring &name, const wstring &mode)
{
        list<CFile1>::iterator it;
	for(it = filelist.begin(); it != filelist.end(); it++)
		if (!name.compare(it->GetName()))
			return 2;

	wstring	t_mode = mode;
	if (!t_mode.compare(L"read"))
		t_mode = L"r";
	else if (!t_mode.compare(L"write"))
		t_mode = L"w";
	else if (!t_mode.compare(L"append"))
		t_mode = L"a";

	if (t_mode.compare(L"r") &&
		t_mode.compare(L"w") &&
		t_mode.compare(L"a"))
		return 0;

	filelist.push_back(CFile1(name, charset, t_mode));
	it = filelist.end();
	it--;
	if (!it->Open()) {
		filelist.erase(it);
		return 0;
	}

	return 1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFile::Delete
 *  �@�\�T�v�F  �w�肳�ꂽ�t�@�C�����N���[�Y���܂�
 *
 *  �Ԓl�@�@�F�@1/2=����/�I�[�v������Ă��Ȃ��A�������͊���fclose����Ă���
 * -----------------------------------------------------------------------
 */
int	CFile::Delete(const wstring &name)
{
	for(list<CFile1>::iterator it = filelist.begin(); it != filelist.end(); it++)
		if (!name.compare(it->GetName())) {
			int	result = it->Close();
			filelist.erase(it);
			return result;
		}

	return 2;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFile::DeleteAll
 *  �@�\�T�v�F  ���ׂẴt�@�C�����N���[�Y���܂�
 * -----------------------------------------------------------------------
 */
void	CFile::DeleteAll(void)
{
	filelist.clear();
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFile::Write
 *  �@�\�T�v�F  �t�@�C���ɕ�������������݂܂�
 *
 *  �Ԓl�@�@�F�@0/1=���s/����
 * -----------------------------------------------------------------------
 */
int	CFile::Write(const wstring &name, const wstring &istr)
{
	for(list<CFile1>::iterator it = filelist.begin(); it != filelist.end(); it++)
		if (!name.compare(it->GetName()))
			return it->Write(istr);

	return 0;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CFile::Read
 *  �@�\�T�v�F  �t�@�C�����當�����1�s�ǂݎ��܂�
 *
 *  �Ԓl�@�@�F�@-1/0/1=EOF/���s/����
 * -----------------------------------------------------------------------
 */
int	CFile::Read(const wstring &name, wstring &ostr)
{
	for(list<CFile1>::iterator it = filelist.begin(); it != filelist.end(); it++)
		if (!name.compare(it->GetName())) {
			int	result = it->Read(ostr);
			return result;
		}

	return 0;
}
