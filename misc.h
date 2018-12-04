// 
// AYA version 5
//
// �G�p�֐�
// written by umeici. 2004
// 

#ifndef	MISCH
#define	MISCH

//----

#ifndef POSIX
#  include "stdafx.h"
#else
#  include <string>
#  include <vector>
using namespace std;
#endif

char	Split(const wstring &str, wstring &dstr0, wstring &dstr1, const wchar_t *sepstr);
char	Split(const wstring &str, wstring &dstr0, wstring &dstr1, const wstring &sepstr);
char	SplitOnly(const wstring &str, wstring &dstr0, wstring &dstr1, wchar_t *sepstr);
char	Split_IgnoreDQ(const wstring &str, wstring &dstr0, wstring &dstr1, const wchar_t *sepstr);
char	Split_IgnoreDQ(const wstring &str, wstring &dstr0, wstring &dstr1, const wstring &sepstr);
int		SplitToMultiString(const wstring &str, vector<wstring> &array, const wstring &delimiter);
void	CutSpace(wstring &str);
void	CutDoubleQuote(wstring &str);
void	CutSingleQuote(wstring &str);
void	AddDoubleQuote(wstring &str);
void	CutCrLf(wstring &str);

wstring	GetDateString(void);

char	IsInDQ(const wstring &str, int startpoint, int checkpoint);
char	IsDoubleString(const wstring &str);
char	IsIntString(const wstring &str);
char	IsIntBinString(const wstring &str, char header);
char	IsIntHexString(const wstring &str, char header);

char	IsLegalFunctionName(const wstring &str);
char	IsLegalVariableName(const wstring &str);
char	IsLegalStrLiteral(const wstring &str);
char	IsLegalPlainStrLiteral(const wstring &str);

int		GetFunctionIndexFromName(const wstring& str);

wstring	ToFullPath(wstring &str);

char	IsNt(void);

//----

// �֐��Ăяo���̐[�����������邽�߂̃N���X

#define	CALLDEPTH_MAX	32	/* �Ăяo���[������̃f�t�H���g�l */

class	CCallDepth
{
protected:
	int	depth;
	int	maxdepth;
public:
	CCallDepth(void) { depth = 0; maxdepth = CALLDEPTH_MAX; }

	void	SetMaxDepth(int value) { maxdepth = value; }

	void	Init(void) { depth = 0; }

	char	Add(void)
	{
		if (depth > maxdepth)
			return 0;

		depth++;
		return 1;
	}

	void	Del(void) { depth--; }
};

//----

#endif
