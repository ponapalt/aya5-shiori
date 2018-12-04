// 
// AYA version 5
//
// �\�����/���ԃR�[�h�̐������s���N���X�@CParser1
// written by umeici. 2004
// 
// �\����͎���CParser0�����x����CParser1::CheckExecutionCode�����s����܂��B
//

#ifndef	PARSER1H
#define	PARSER1H

//----

#ifndef POSIX
#  include "stdafx.h"
#endif

class	CVecint
{
public:
	vector<int> i_array;
};

//----

class	CParser1
{
public:
	static char	CheckExecutionCode(void);
	static char	CParser1::CheckExecutionCode1(CStatement& st, const wstring& dicfilename);

protected:
	static char	CheckSubstSyntax(CStatement& st, const wstring& dicfilename);
	static char	CheckFeedbackOperatorPos(CStatement& st, const wstring& dicfilename);
	static char	SetFormulaType(CStatement& st, const wstring& dicfilename);
	static char	SetBreakJumpNo(void);
	static char	CheckCaseSyntax(void);
	static char	CheckIfSyntax(void);
	static char	CheckElseSyntax(void);
	static char	CheckForSyntax(void);
	static char	CheckForeachSyntax(void);
	static char	SetIfJumpNo(void);
	static char	CheckFunctionArgument(CStatement& st, const wstring& dicfilename);

	static void	CompleteSetting(void);
};

//----

#endif
