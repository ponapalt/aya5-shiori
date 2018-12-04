// 
// AYA version 5
//
// �����̍��������N���X�@CCell/CSerial
// written by umeici. 2004
// 
// CCell�̓X�e�[�g�����g���̐����̍����ACSerial�͂��̉��Z������ێ����邾���̃N���X�ł��B
// ����͂����̃C���X�^���X�����N���X���s�Ȃ��܂��B
//

#ifndef	CELLH
#define	CELLH

//----

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "value.h"

class	CSerial
{
public:
	int	tindex;						// ���Z�q��CCell�ʒu�ԍ�
	vector<int>	index;				// ���Z�Ώۂ�CCell�ʒu�ԍ��̃��X�g
									// ���Z�q��F_TAG_FUNCPARAM/F_TAG_SYSFUNCPARAM�̏ꍇ�́Aindex[0]���֐��������܂�
public:
	CSerial::CSerial(int t) { tindex = t; }

	CSerial::CSerial(void) {}
	CSerial::~CSerial(void) {}
};

//----

class	CCell
{
public:
	wstring	name;					// ���̍���"���O"�ithis�����[�J���ϐ��̎��Ɏg�p���܂��j
	int		index;					// �ʒu�ԍ��ithis���ϐ�/���[�J���ϐ�/�֐��̍ۂɎg�p���܂��j
	int		depth;					// {}����q�̐[���ithis�����[�J���ϐ��̎��Ɏg�p���܂��j

	CValue	v;						// �l�ithis�����e�����l�̍ۂɎg�p���܂��j
	CValue	ansv;					// �l�ithis���ϐ�/���[�J���ϐ�/�֐��̍ۂɁA���̓��e/�Ԓl���i�[���܂��j
	CValue	emb_ansv;				// �l�i%[n]�ŎQ�Ƃ����l��ێ����܂��j
	CValue	order;					// ���Z���Ɏg�p�i�z��̏������ꎞ�I�ɕێ����܂��j
public:
	CCell::CCell(int t) : v(t, 0/*dmy*/), ansv(), emb_ansv(), order()
	{
		index = -1;
		depth = -1;
	}

	CCell::CCell(void) : v(), ansv(), emb_ansv(), order()
	{
		index = -1;
		depth = -1;
	}

	~CCell(void) {}
};

//----

#endif
