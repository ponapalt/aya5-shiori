// 
// AYA version 5
//
// 数式の項を扱うクラス　CCell/CSerial
// written by umeici. 2004
// 
// CCellはステートメント内の数式の項を、CSerialはその演算順序を保持するだけのクラスです。
// 操作はこれらのインスタンスを持つクラスが行ないます。
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
	int	tindex;						// 演算子のCCell位置番号
	vector<int>	index;				// 演算対象のCCell位置番号のリスト
									// 演算子がF_TAG_FUNCPARAM/F_TAG_SYSFUNCPARAMの場合は、index[0]が関数を示します
public:
	CSerial::CSerial(int t) { tindex = t; }

	CSerial::CSerial(void) {}
	CSerial::~CSerial(void) {}
};

//----

class	CCell
{
public:
	wstring	name;					// この項の"名前"（thisがローカル変数の時に使用します）
	int		index;					// 位置番号（thisが変数/ローカル変数/関数の際に使用します）
	int		depth;					// {}入れ子の深さ（thisがローカル変数の時に使用します）

	CValue	v;						// 値（thisがリテラル値の際に使用します）
	CValue	ansv;					// 値（thisが変数/ローカル変数/関数の際に、その内容/返値を格納します）
	CValue	emb_ansv;				// 値（%[n]で参照される値を保持します）
	CValue	order;					// 演算時に使用（配列の序数を一時的に保持します）
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
