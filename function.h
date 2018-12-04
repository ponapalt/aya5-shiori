// 
// AYA version 5
//
// �֐��������N���X�@CFunction/CStatement
// written by umeici. 2004
// 
// CFunction���֐��ACStatement���֐����̃X�e�[�g�����g�ł��B
// CStatement�͒l�̕ێ��݂̂ŁA����̓C���X�^���X������CFunction�ōs���܂��B
//

#ifndef	FUNCTIONH
#define	FUNCTIONH

//----

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "value.h"
#include "cell.h"
#include "variable.h"
#include "selecter.h"

class	CStatement
{
public:
	int				type;			// �X�e�[�g�����g�̎��
	vector<CCell>	cell;			// �����̍��̌Q�@
	vector<CSerial>	serial;			// �����̉��Z����
	int				jumpto;			// ��ѐ�s�ԍ� break/continue/return/if/elseif/else/for/foreach�Ŏg�p���܂�
									// �Y���P�ʏI�[��"}"�̈ʒu���i�[����Ă��܂�
	int	linecount;					// �����t�@�C�����̍s�ԍ�
public:
	CStatement(int t, int l)
	{
		type      = t;
		linecount = l;
	}

	CStatement(void) {}
	~CStatement(void) {}
};

//----

class	CFunction
{
public:
	wstring				name;			// ���O
	int					namelen;		// ���O�̒���
	vector<CStatement>	statement;		// ���ߌS
	CDuplEvInfo			dupl;			// �d����𐧌�
	wstring				dicfilename;	// �Ή����鎫���t�@�C����

protected:
	int					statelenm1;		// statement�̒���-1�i1�������Ă���̂͏I�[��"}"���������Ȃ����߂ł��j

public:
	CFunction(const wstring& n, int ct, const wstring& df) : dupl(ct)
	{
		name        = n;
		namelen     = name.size();
		dicfilename = df;
	}

	CFunction(void) : dupl(CHOICETYPE_RANDOM) {}
	~CFunction(void) {}

	void	CompleteSetting(void);
	CValue	Execute(CValue &arg, CLocalVariable &lvar, int &exitcode);

	CValue	GetFormulaAnswer(CLocalVariable &lvar, CStatement &st);

protected:
	int		ExecuteInBrace(int line, CValue &result, CLocalVariable &lvar, int type, int &exitcode);

	char	Foreach(CStatement &st0, CStatement &st1, CLocalVariable &lvar, int &foreachcount);

	CValue	*GetValuePtrForCalc(CCell &cell, CStatement &st, CLocalVariable &lvar);
	void	SolveEmbedCell(CCell &cell, CStatement &st, CLocalVariable &lvar);

	char	Cumma(CValue &answer, vector<int> &sid, CStatement &st, CLocalVariable &lvar);
	char	Subst(int type, CValue &answer, vector<int> &sid, CStatement &st, CLocalVariable &lvar);
	char	SubstToArray(CCell &vcell, CCell &ocell, CValue &answer, CStatement &st, CLocalVariable &lvar);
	char	Array(CCell &anscell, vector<int> &sid, CStatement &st, CLocalVariable &lvar);
	CValue	_in_(CValue &src, CValue &dst);
	CValue	not_in_(CValue &src, CValue &dst);
	char	ExecFunctionWithArgs(CValue &answer, vector<int> &sid, CStatement &st, CLocalVariable &lvar);
	char	ExecSystemFunctionWithArgs(CCell& cell, vector<int> &sid, CStatement &st, CLocalVariable &lvar);
	void	ExecHistoryP1(int start_index, CCell& cell, CValue &arg, CStatement &st);
	void	ExecHistoryP2(CCell &cell, CStatement &st);
	char	Feedback(CCell &anscell, vector<int> &sid, CStatement &st, CLocalVariable &lvar);
	CValue	Exc(CValue &value);
	CValue	EncodeArrayOrder(CCell &vcell, CValue &order, CLocalVariable &lvar);
	void	FeedLineToTail(int &line);
};

//----

#endif
