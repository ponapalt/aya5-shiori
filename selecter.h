// 
// AYA version 5
//
// 出力の選択を行なうクラス　CSelecter/CDuplEvInfo
// written by umeici. 2004
// 
// CSelecterは出力の選択を行ないます。
// CDuplEvInfoは重複回避を行ないます。
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

#define	CHOICETYPE_RANDOM		0	/* 常に無作為にランダム（デフォルト）*/
#define	CHOICETYPE_NONOVERLAP	1	/* ランダムだが一巡するまで重複選択しない */
#define	CHOICETYPE_SEQUENTIAL	2	/* 順番に選択する */
#define	CHOICETYPE_VOID			3	/* 出力なし */
#define	CHOICETYPE_ARRAY		4	/* 簡易配列編成 */

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
	int				type;			// 選択種別

	vector<int>		num;			// --で区切られた領域毎の候補数
	vector<int>		roundorder;		// 巡回順序
	int				total;			// 出力候補値の総数
	int				index;			// 現在の巡回位置
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
	vector<CVecValue>	values;			// 出力候補値
	int					areanum;		// 出力候補を蓄積する領域の数
	CDuplEvInfo			*duplctl;		// 対応する重複回避情報へのポインタ
	int					aindex;			// switch構文で使用
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
