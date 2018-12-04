// 
// AYA version 5
//
// 構文解析/中間コードの生成を行うクラス　CParser0
// written by umeici. 2004
// 
// 構文解析時にCBasisから一度だけCParser0::Parseが実行されます。
// CParser0::ParseEmbedStringはeval系の処理で使用されます。
//

#ifndef	PARSER0H
#define	PARSER0H

//----

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "variable.h"
#include "function.h"
#include "value.h"

class	CDefine
{
public:
	wstring	before;
	wstring	after;
public:
	CDefine::CDefine(wstring &bef, wstring &aft)
	{
		before = bef;
		after  = aft;
	}

	CDefine::CDefine(void) {}
	CDefine::~CDefine(void) {}
};

//----

class	CParser0
{
public:
	static char	Parse(int charset, const vector<wstring>& dics, int &lindex, int &ulindex, int &rindex);
	static char	ParseEmbedString(wstring& str, CStatement &st, const wstring &dicfilename, int linecount);

protected:
	static char	LoadDictionary1(const wstring& filename, vector<CDefine>& gdefines, int charset);
	static char	GetPreProcess(wstring& str, vector<CDefine>& defines, vector<CDefine>& gdefines, const wstring& dicfilename,
					int linecount);
	static void	ExecDefinePreProcess(wstring &str, const vector<CDefine>& defines);
	static char	IsCipheredDic(const wstring& filename);
	static void	SeparateFactor(vector<wstring> &s, wstring &line);
	static char	DefineFunctions(vector<wstring> &s, const wstring& dicfilename, int linecount, int &depth, int &targetfunction);
	static int	MakeFunction(const wstring& name, int chtype, const wstring& dicfilename);
	static char	StoreInternalStatement(int targetfunc, wstring &str, int& depth, const wstring& dicfilename, int linecount);
	static char	MakeStatement(int type, int targetfunc, wstring &str, const wstring& dicfilename, int linecount);
	static char	StructWhen(wstring &str, vector<CCell> &cells, const wstring& dicfilename, int linecount);
	static char	StructFormula(wstring &str, vector<CCell> &cells, const wstring& dicfilename, int linecount);
	static void	StructFormulaCell(wstring &str, vector<CCell> &cells);

	static char	AddSimpleIfBrace(void);

	static char	SetCellType(void);
	static char	SetCellType1(CCell& scell, char emb, const wstring& dicfilename, int linecount);

	static char	MakeCompleteFormula(void);
	static char	ParseEmbeddedFactor(void);
	static char	ParseEmbeddedFactor1(CStatement& st, const wstring& dicfilename);
	static void	ConvertPlainString(void);
	static void	ConvertPlainString1(CStatement& st, const wstring& dicfilename);
	static char	ConvertEmbedStringToFormula(wstring& str, const wstring& dicfilename, int linecount);
	static char	CheckDepthAndSerialize(void);
	static char	CheckDepth1(CStatement& st, const wstring& dicfilename);
	static char	CheckDepthAndSerialize1(CStatement& st, const wstring& dicfilename);
	static char	MakeCompleteConvertionWhenToIf(void);
};

//----

#endif
