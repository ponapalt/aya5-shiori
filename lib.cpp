// 
// AYA version 5
//
// �O�����C�u�����������N���X�@CLib
// written by umeici. 2004
// 
// request�̓x��list����Ώۂ��������Ă��܂����A�ÓI��load���Ă���DLL��
// �����Ă������Ǝv���̂ŁA����ł����s���x�ɖ��͂Ȃ��ƍl���Ă��܂��B
//

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "lib.h"
#include "misc.h"
#include "log.h"

extern CLog	logger;

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLib::Add
 *  �@�\�T�v�F  �w�肳�ꂽ�t�@�C������DLL�����[�h���Aload�����s���܂�
 *
 *  �Ԓl�@�@�F�@0/1/2=���s/����/���Ƀ��[�h����Ă���
 * -----------------------------------------------------------------------
 */
int	CLib::Add(const wstring &name)
{
        list<CLib1>::iterator it;
	for(it = liblist.begin(); it != liblist.end(); it++)
		if (!name.compare(it->GetName()))
			return 2;

	liblist.push_back(CLib1(name, charset));
	it = liblist.end();
	it--;
	if (!it->LoadLib()) {
		liblist.erase(it);
		return 0;
	}

	if (!it->Load()) {
		liblist.erase(it);
		return 0;
	}

	return 1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLib::Delete
 *  �@�\�T�v�F  �w�肳�ꂽ�t�@�C������DLL�ɂ�unload�����s���A�����[�X���܂�
 *
 *  �Ԓl�@�@�F�@0/1/2=���s/����/���[�h����Ă��Ȃ��A�������͊���unload����Ă���
 * -----------------------------------------------------------------------
 */
int	CLib::Delete(const wstring &name)
{
	for(list<CLib1>::iterator it = liblist.begin(); it != liblist.end(); it++)
		if (!name.compare(it->GetName())) {
			int	result = it->Unload();
			it->Release();
			liblist.erase(it);
			return result;
		}

	return 2;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLib::DeleteAll
 *  �@�\�T�v�F  ���ׂĂ�DLL�ɂ�unload()�����s���A�����[�X���܂�
 * -----------------------------------------------------------------------
 */
void	CLib::DeleteAll(void)
{
	liblist.clear();
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLib::Request
 *  �@�\�T�v�F  request�����s���A���ʂ��擾���܂�
 *
 *  �Ԓl�@�@�F�@0/1=���s/����
 * -----------------------------------------------------------------------
 */
int	CLib::Request(const wstring &name, const wstring &istr, wstring &ostr)
{
	for(list<CLib1>::iterator it = liblist.begin(); it != liblist.end(); it++)
		if (!name.compare(it->GetName())) {
			logger.IoLib(0, istr, name);
			int	result = it->Request(istr, ostr);
			logger.IoLib(1, ostr, name);
			return result;
		}

	ostr = L"";
	return 0;
}
