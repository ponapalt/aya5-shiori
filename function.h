// 
// AYA version 5
//
// 関数を扱うクラス　CFunction/CStatement
// written by umeici. 2004
// 
// CFunctionが関数、CStatementが関数内のステートメントです。
// CStatementは値の保持のみで、操作はインスタンスを持つCFunctionで行います。
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
	int				type;			// ステートメントの種別
	vector<CCell>	cell;			// 数式の項の群　
	vector<CSerial>	serial;			// 数式の演算順序
	int				jumpto;			// 飛び先行番号 break/continue/return/if/elseif/else/for/foreachで使用します
									// 該当単位終端の"}"の位置が格納されています
	int	linecount;					// 辞書ファイル中の行番号
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
	wstring				name;			// 名前
	int					namelen;		// 名前の長さ
	vector<CStatement>	statement;		// 命令郡
	CDuplEvInfo			dupl;			// 重複回避制御
	wstring				dicfilename;	// 対応する辞書ファイル名

protected:
	int					statelenm1;		// statementの長さ-1（1を減じているのは終端の"}"を処理しないためです）

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
