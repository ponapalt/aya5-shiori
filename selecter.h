// 
// AYA version 5
//
// �o�͂̑I�����s�Ȃ��N���X�@CSelecter/CDuplEvInfo
// written by umeici. 2004
// 
// CSelecter�͏o�͂̑I�����s�Ȃ��܂��B
// CDuplEvInfo�͏d��������s�Ȃ��܂��B
//

#ifndef	SELECTERH
#define	SELECTERH

//----

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "value.h"
#include "cell.h"
#include "variable.h"

#define	CHOICETYPE_RANDOM		0	/* ��ɖ���ׂɃ����_���i�f�t�H���g�j*/
#define	CHOICETYPE_NONOVERLAP	1	/* �����_�������ꏄ����܂ŏd���I�����Ȃ� */
#define	CHOICETYPE_SEQUENTIAL	2	/* ���ԂɑI������ */
#define	CHOICETYPE_VOID			3	/* �o�͂Ȃ� */
#define	CHOICETYPE_ARRAY		4	/* �ȈՔz��Ґ� */

#define	CHOICETYPE_NUM			5

const wchar_t	choicetype[CHOICETYPE_NUM][16] = {
	L"random",
	L"nonoverlap",
	L"sequential",
	L"void",
	L"array"
};

//----

class CVecValue
{
public:
	vector<CValue>	array;
};

//----

class	CDuplEvInfo
{
protected:
	int				type;			// �I�����

	vector<int>		num;			// --�ŋ�؂�ꂽ�̈斈�̌�␔
	vector<int>		roundorder;		// ���񏇏�
	int				total;			// �o�͌��l�̑���
	int				index;			// ���݂̏���ʒu
public:
	CDuplEvInfo(int tp)
	{
		type  = tp;
		total = 0;
		index = 0;
	}

	int		GetType(void) { return type; }

	CValue	Choice(int areanum, const vector<CVecValue> &values, int mode);

protected:
	void	InitRoundOrder(int mode);
	char	UpdateNums(int areanum, const vector<CVecValue> &values);
	CValue	GetValue(int areanum, const vector<CVecValue> &values);
};

//----

class CSelecter
{
protected:
	vector<CVecValue>	values;			// �o�͌��l
	int					areanum;		// �o�͌���~�ς���̈�̐�
	CDuplEvInfo			*duplctl;		// �Ή�����d��������ւ̃|�C���^
	int					aindex;			// switch�\���Ŏg�p
public:
	CSelecter(CDuplEvInfo *dc, int aid);
	CSelecter(void) {}

	void	AddArea(void);
	void	Append(const CValue &value);
	CValue	Output(void);

protected:
	CValue StructArray1(int index);
	CValue StructArray(void);
	CValue	ChoiceRandom(void);
	CValue	ChoiceRandom1(int index);
	CValue	ChoiceByIndex(void);
	CValue	ChoiceByIndex1(int index);
};

//----

#endif
