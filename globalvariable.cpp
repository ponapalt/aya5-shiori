// 
// AYA version 5
//
// �O���[�o���ϐ��������N���X�@CGlobalVariable
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "basis.h"
#include "variable.h"
#include "value.h"
#include "misc.h"
#include "log.h"
#include "wsex.h"
#include "ccct.h"

extern CBasis	basis;

/* -----------------------------------------------------------------------
 *  �֐���  �F  CGlobalVariable::Make
 *  �@�\�T�v�F  �ϐ����쐬���܂�
 *  �����@�@�F  erased 0/1=�L�����/�������
 *
 *  �Ԓl�@�@�F  �ǉ������ʒu
 *
 *  �w�肳�ꂽ���O�̕ϐ������ɑ��݂��Ă����ꍇ�͒ǉ��͍s�킸�A�����̈ʒu��Ԃ��܂�
 * -----------------------------------------------------------------------
 */
int	CGlobalVariable::Make(const wstring &name, char erased)
{
	// ���ɑ��݂��邩���ׁA���݂��Ă����炻���Ԃ�
	int	i = 0;
	for(vector<CVariable>::iterator it = var.begin(); it != var.end(); it++, i++)
		if (!name.compare(it->name)) {
			if (!basis.IsRun()) {
				if (erased)
					it->Erase();
				else
					it->Enable();
			}
		return i;
		}

	// �쐬
	CVariable	addvariable(name);
	if (erased)
		addvariable.Erase();
	else
		addvariable.Enable();
	var.push_back(addvariable);

	return var.size() - 1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CGlobalVariable::GetMacthedLongestNameLength
 *  �@�\�T�v�F  �w�肳�ꂽ������Ƀ}�b�`���閼�O�����ϐ���T���A�}�b�`����������Ԃ��܂�
 *
 *  �������������ꍇ�͍Œ��̂��̂�Ԃ��܂��B������Ȃ������ꍇ��0��Ԃ��܂�
 * -----------------------------------------------------------------------
 */
int	CGlobalVariable::GetMacthedLongestNameLength(const wstring &name)
{
	int	max_len = 0;

	for(vector<CVariable>::iterator it = var.begin(); it != var.end(); it++) {
		int	len = it->name.size();
		if (!it->IsErased() && max_len < len && !it->name.compare(name.substr(0, len)))
			max_len = len;
	}

	return max_len;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CGlobalVariable::GetIndex
 *  �@�\�T�v�F  �w�肳�ꂽ���O�̕ϐ��̈ʒu��Ԃ��܂�
 *
 *  ������Ȃ������ꍇ��-1��Ԃ��܂�
 * -----------------------------------------------------------------------
 */
int		CGlobalVariable::GetIndex(const wstring &name)
{
	int	i = 0;
	for(vector<CVariable>::iterator it = var.begin(); it != var.end(); it++, i++)
		if (!name.compare(it->name))
			return (it->IsErased()) ? -1 : i;

	return -1;
}
