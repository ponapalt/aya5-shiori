// 
// AYA version 5
//
// �ϐ��������N���X�@CVariable/CLVSubStack/CLocalVariable/CGlobalVariable
// written by umeici. 2004
// 
// CVariable�N���X�͒l�̕ێ������s�Ȃ��܂���B
//
// CLVSubStack/CLocalVariable�̓��[�J���ϐ����Ǘ����܂��B
// �X�^�b�N�Ƃ͖��΂���Ŏ��ۂ̍\���͉ϒ��z��ł��i�����_���A�N�Z�X�����������̂Łj
//
// CLocalVariable�̃C���X�^���X�͊֐����s�J�n���ɍ쐬����A�֐�����Ԃ�ۂɔj������܂��B
// �\���Ƃ��Ă� vector<vector<CVariable>> �ŁA{}����q�̐[�����Ƀ��[�J���ϐ��̔z���
// �����Ă��邱�ƂɂȂ�܂��B{}����q�ɓ���ۂɔz�񂪒ǉ�����A�o��ۂɔj������܂��B
//
// CGlobalVariable�̓O���[�o���ϐ����Ǘ����܂��B
//

#ifndef	VARIABLEH
#define	VARIABLEH

//----

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "value.h"
#include "cell.h"

class	CVariable
{
public:
	wstring	name;					// ���O
	wstring	delimiter;				// �f���~�^

	CValue	v;						// �l

protected:
	char	erased;					// �������ꂽ���Ƃ������t���O�i�O���[�o���ϐ��Ŏg�p�j
									// 0/1=�L��/�������ꂽ

public:
	CVariable::CVariable(const wstring &n) : v()
	{
		name       = n;
		delimiter  = VAR_DELIMITER;

		erased     = 0;
	}

	CVariable::CVariable(wchar_t *n) : v()
	{
		name       = n;
		delimiter  = VAR_DELIMITER;

		erased     = 0;
	}

	CVariable::CVariable(void) : v()
	{
		name       = L"";
		delimiter  = VAR_DELIMITER;

		erased     = 0;
	}

	~CVariable(void) {}

	void	Enable(void) { erased = 0; }
	void	Erase(void) { erased = 1; }
	char	IsErased(void) { return erased; }
};

//----

class	CLVSubStack
{
public:
	vector<CVariable>	substack;
};

//----

class	CLocalVariable
{
protected:
	vector<CLVSubStack>	stack;
	int	depth;

public:
	CLocalVariable(void);
	~CLocalVariable(void);

	CVariable	*GetArgvPtr(void);

	void	AddDepth(void);
	void	DelDepth(void);

	int		GetDepth(void) { return depth; }

public:
	void	GetIndex(const wchar_t *name, int &id, int &dp);
	void	GetIndex(const wstring &name, int &id, int &dp);

	void	Make(const wchar_t *name);
	void	Make(const wstring &name);
	void	Make(const wchar_t *name, const CValue &value);
	void	Make(const wstring &name, const CValue &value);
	void	Make(const wstring &name, const CValueSub &value);
	void	Make(const wchar_t *name, const wstring &delimiter);
	void	Make(const wstring &name, const wstring &delimiter);

	CValue	GetValue(const wchar_t *name);
	CValue	GetValue(const wstring &name);

	wstring	GetDelimiter(const wchar_t *name);
	wstring	GetDelimiter(const wstring &name);

	void	SetDelimiter(const wchar_t *name, const wstring &value);
	void	SetDelimiter(const wstring &name, const wstring &value);

	void	SetValue(const wchar_t *name, const CValue &value);
	void	SetValue(const wstring &name, const CValue &value);

	int		GetMacthedLongestNameLength(const wstring &name);

	void	Erase(const wstring &name);
};

//----

class	CGlobalVariable
{
protected:
	vector<CVariable>	var;

public:
	void	CompleteSetting(void)
	{
		vector<CVariable>(var).swap(var);
	}

	int		Make(const wstring &name, char erased);
	int		GetMacthedLongestNameLength(const wstring &name);

	int		GetIndex(const wstring &name);

	wstring	GetName(int index) { return var[index].name; }
	int		GetNumber(void) { return var.size(); }
	CVariable	*GetPtr(int index) { return &(var[index]); }
	CValue	*GetValuePtr(int index) { return &(var[index].v); }
	void	SetType(int index, int type) { var[index].v.SetType(type); }

	CValue	GetValue(int index) { return var[index].v; }
	wstring	GetDelimiter(int index) { return var[index].delimiter; }

	void	SetValue(int index, const CValue &value) { var[index].Enable(); var[index].v = value; }
	void	SetValue(int index, int value) { var[index].Enable(); var[index].v = value; }
	void	SetValue(int index, double value) { var[index].Enable(); var[index].v = value; }
	void	SetValue(int index, const wstring &value) { var[index].Enable(); var[index].v = value; }
	void	SetValue(int index, const wchar_t *value) { var[index].Enable(); var[index].v = value; }
	void	SetValue(int index, const vector<CValueSub> &value) { var[index].Enable(); var[index].v = value; }
	void	SetValue(int index, const CValueSub &value) { var[index].Enable(); var[index].v = value; }
	void	SetDelimiter(int index, const wstring value) { var[index].Enable(); var[index].delimiter = value; }

	void	Erase(const wstring &name)
	{
		int	index = GetIndex(name);
		if (index >= 0) {
			SetValue(index, L"");
			var[index].Erase();
		}
	}

};

//----

#endif
