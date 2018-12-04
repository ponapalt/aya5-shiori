// 
// AYA version 5
//
// 中間コードをログに出力するクラス　CLogExCode
// written by umeici. 2004
// 
// デバッグ用の機能です。実動作には関係しません。
// これらの関数が吐き出す出力は、ソースコードを理解していないと読み難いものです。
//

#ifndef	LOGEXCODEH
#define	LOGEXCODEH

//----

#ifndef POSIX
#include "stdafx.h"
#endif
#include "function.h"

class	CLogExCode
{
public:
	static void	OutExecutionCodeForCheck(void);
	static void	OutVariableInfoForCheck(void);
protected:
	static void	StructCellString(vector<CCell> *cellvector, wstring &formula);
	static void	StructSerialString(CStatement *st, wstring &formula);
	static void	StructArrayString(vector<CValueSub> &vs, wstring &enlist);
};

//----

#endif
