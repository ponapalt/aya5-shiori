// 
// AYA version 5
//
// ���[�J���ϐ��������N���X�@CLocalVariable
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "variable.h"
#include "value.h"
#include "misc.h"
#include "log.h"
#include "wsex.h"
#include "ccct.h"

extern CLog	logger;

/* -----------------------------------------------------------------------
 *  CLocalVariable�R���X�g���N�^
 * -----------------------------------------------------------------------
 */
CLocalVariable::CLocalVariable(void)
{
	depth = -1;

	AddDepth();
}

/* -----------------------------------------------------------------------
 *  CLocalVariable�f�X�g���N�^
 * -----------------------------------------------------------------------
 */
CLocalVariable::~CLocalVariable(void)
{
	stack.clear();
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLocalVariable::GetArgvAdr
 *  �@�\�T�v�F  _argv���i�[����Ă���ʒu�̃|�C���^��Ԃ��܂�
 *
 *  vector�͍Ċ��蓖�Ăɂ���ăA�h���X���ω�����̂ŁA�����œ�����l��
 *  �ꎞ�I�ɗ��p���邱�Ƃ����o���܂���B
 * -----------------------------------------------------------------------
 */
CVariable	*CLocalVariable::GetArgvPtr(void)
{
	return &(stack[0].substack[0]);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLocalVariable::AddDepth
 *  �@�\�T�v�F  ��K�w�[�����[�J���ϐ��X�^�b�N���쐬���܂�
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::AddDepth(void)
{
	CLVSubStack	addsubstack;
	stack.push_back(addsubstack);
	depth++;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLocalVariable::DelDepth
 *  �@�\�T�v�F  �ł��[�����[�J���ϐ��X�^�b�N��j�����܂�
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::DelDepth(void)
{
	if (stack.size()) {
		stack.erase(stack.end() - 1);
		depth--;
	}
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLocalVariable::Make
 *  �@�\�T�v�F  ���[�J���ϐ����쐬���܂�
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::Make(const wchar_t *name)
{
	CVariable	addlv(name);
	stack[depth].substack.push_back(addlv);
}

//----

void	CLocalVariable::Make(const wstring &name)
{
	CVariable	addlv(name);
	stack[depth].substack.push_back(addlv);
}

//----

void	CLocalVariable::Make(const wchar_t *name, const CValue &value)
{
	CVariable	addlv(name);
	addlv.v = value;
	stack[depth].substack.push_back(addlv);
}

//----

void	CLocalVariable::Make(const wstring &name, const CValue &value)
{
	CVariable	addlv(name);
	addlv.v = value;
	stack[depth].substack.push_back(addlv);
}

//----

void	CLocalVariable::Make(const wstring &name, const CValueSub &value)
{
	CVariable	addlv(name);
	addlv.v = value;
	stack[depth].substack.push_back(addlv);
}

//----

void	CLocalVariable::Make(const wchar_t *name, const wstring &delimiter)
{
	CVariable	addlv(name);
	addlv.delimiter = delimiter;
	stack[depth].substack.push_back(addlv);
}

//----

void	CLocalVariable::Make(const wstring &name, const wstring &delimiter)
{
	CVariable	addlv(name);
	addlv.delimiter = delimiter;
	stack[depth].substack.push_back(addlv);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLocalVariable::GetIndex
 *  �@�\�T�v�F  �w�肳�ꂽ���O�̃��[�J���ϐ����X�^�b�N���猟�����܂�
 *
 *  ������Ȃ������ꍇ��-1��Ԃ��܂�
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::GetIndex(const wchar_t *name, int &id, int &dp)
{
	for(int i = stack.size() - 1; i >= 0; i--)
		for(int j = stack[i].substack.size() - 1; j >= 0; j--)
			if (!stack[i].substack[j].name.compare(name)) {
				if (stack[i].substack[j].IsErased()) {
					id = -1;
					dp = -1;
				}
				else {
					dp = i;
					id = j;
				}
				return;
			}

	id = -1;
	dp = -1;
}

//----

void	CLocalVariable::GetIndex(const wstring &name, int &id, int &dp)
{
	for(int i = stack.size() - 1; i >= 0; i--)
		for(int j = stack[i].substack.size() - 1; j >= 0; j--)
			if (!stack[i].substack[j].name.compare(name)) {
				if (stack[i].substack[j].IsErased()) {
					id = -1;
					dp = -1;
				}
				else {
					dp = i;
					id = j;
				}
				return;
			}

	id = -1;
	dp = -1;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLocalVariable::GetValue
 *  �@�\�T�v�F  �w�肳�ꂽ���[�J���ϐ��̒l���擾���܂�
 *
 *  �w�肳�ꂽ�ϐ������݂��Ȃ������ꍇ�͋󕶎��񂪕Ԃ�܂��B
 * -----------------------------------------------------------------------
 */

CValue	CLocalVariable::GetValue(const wchar_t *name)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0)
		return stack[dp].substack[id].v;

	return CValue();
}

//----

CValue	CLocalVariable::GetValue(const wstring &name)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0)
		return stack[dp].substack[id].v;

	return CValue();
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLocalVariable::GetDelimiter
 *  �@�\�T�v�F  �w�肳�ꂽ���[�J���ϐ��̃f���~�^���擾���܂�
 * -----------------------------------------------------------------------
 */
wstring	CLocalVariable::GetDelimiter(const wchar_t *name)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0)
		return stack[dp].substack[id].delimiter;

	return wstring(L"");
}

//----

wstring	CLocalVariable::GetDelimiter(const wstring &name)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0)
		return stack[dp].substack[id].delimiter;

	return wstring(L"");
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLocalVariable::SetDelimiter
 *  �@�\�T�v�F  �w�肳�ꂽ���[�J���ϐ��ɒl���i�[���܂�
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::SetDelimiter(const wchar_t *name, const wstring &value)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0) {
		stack[dp].substack[id].delimiter = value;
		return;
	}

	// �X�^�b�N���ɑ��݂��Ȃ���Εϐ���V���ɍ쐬���đ��
	Make(name, value);
}

//----

void	CLocalVariable::SetDelimiter(const wstring &name, const wstring &value)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0) {
		stack[dp].substack[id].delimiter = value;
		return;
	}

	// �X�^�b�N���ɑ��݂��Ȃ���Εϐ���V���ɍ쐬���đ��
	Make(name, value);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLocalVariable::SetValue
 *  �@�\�T�v�F  �w�肳�ꂽ���[�J���ϐ��ɒl���i�[���܂�
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::SetValue(const wchar_t *name, const CValue &value)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0) {
		stack[dp].substack[id].v = value;
		return;
	}

	// �X�^�b�N���ɑ��݂��Ȃ���Εϐ���V���ɍ쐬���đ��
	Make(name, value);
}

//----

void	CLocalVariable::SetValue(const wstring &name, const CValue &value)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0) {
		stack[dp].substack[id].v = value;
		return;
	}

	// �X�^�b�N���ɑ��݂��Ȃ���Εϐ���V���ɍ쐬���đ��
	Make(name, value);
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLocalVariable::GetMacthedLongestNameLength
 *  �@�\�T�v�F  �w�肳�ꂽ������Ƀ}�b�`���閼�O�����ϐ���T���A�}�b�`����������Ԃ��܂�
 *
 *  �������������ꍇ�͍Œ��̂��̂�Ԃ��܂��B������Ȃ������ꍇ��0��Ԃ��܂�
 * -----------------------------------------------------------------------
 */
int	CLocalVariable::GetMacthedLongestNameLength(const wstring &name)
{
	int	max_len = 0;

	for(int i = stack.size() - 1; i >= 0; i--)
		for(int j = stack[i].substack.size() - 1; j >= 0; j--) {
			int	len = stack[i].substack[j].name.size();
			if (!stack[i].substack[j].IsErased() &&
				max_len < len &&
				!stack[i].substack[j].name.compare(name.substr(0, len)))
				max_len = len;
		}

	return max_len;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CLocalVariable::Erase
 *  �@�\�T�v�F  �w�肳�ꂽ�ϐ����������܂�
 *
 *  ���ۂɔz�񂩂�����킯�ł͂Ȃ��A�󕶎���������邾���ł�
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::Erase(const wstring &name)
{
	int	id, dp;
	GetIndex(name, id, dp);
	if (id >= 0)
		SetValue(name, CValue());
}
