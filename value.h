// 
// AYA version 5
//
// 値を扱うクラス　CValue/CValueSub
// written by umeici. 2004
// 
// CValueは型フラグと型別の情報を持っています。その中にvector<CValueSub>という可変長配列があり、
// これが汎用配列となります。CValueSubクラスは配列を持たないほかはCValueと類似しています（汎用
// 配列を多次元化出来ない、というAYA5の制限はこの構造に由来しています）。
// 

#ifndef	VALUEH
#define	VALUEH

//----

#ifdef POSIX
#  include <string>
#  include <vector>
#  include "manifest.h"
using namespace std;
#else
#  include "stdafx.h"
#endif

class	CValueSub
{
protected:
	int	type;						// 型
public:
	wstring	s_value;				// 文字列値
	int		i_value;				// 整数値
	double	d_value;				// 実数値
public:
	CValueSub(void)
	{
		type     = F_TAG_STRING;
		s_value  = L"";
	}

	CValueSub(int value)
	{
		type     = F_TAG_INT;
		i_value  = value;
	}

	CValueSub(double value)
	{
		type     = F_TAG_DOUBLE;
		d_value  = value;
	}

	CValueSub(const wstring &value)
	{
		type     = F_TAG_STRING;
		s_value  = value;
	}

	CValueSub(const wchar_t *value)
	{
		type     = F_TAG_STRING;
		s_value  = value;
	}

	~CValueSub(void) {}

	int		GetType(void) const { return type; }

	int		GetValueInt(void) const;
	double	GetValueDouble(void) const;
	wstring	GetValueString(void) const;

	CValueSub	&operator =(int value);
	CValueSub	&operator =(double value);
	CValueSub	&operator =(const wstring &value);
	CValueSub	&operator =(const wchar_t *value);

	CValueSub	operator +(const CValueSub &value) const;
	CValueSub	operator -(const CValueSub &value) const;
	CValueSub	operator *(const CValueSub &value) const;
	CValueSub	operator /(const CValueSub &value) const;
	CValueSub	operator %(const CValueSub &value) const;

	CValueSub	operator ==(const CValueSub &value) const;
};

//----

class	CValue
{
protected:
	int	type;						// 型
public:
	wstring	s_value;				// 文字列値
	int		i_value;				// 整数値
	double	d_value;				// 実数値
	vector<CValueSub>	array;		// 汎用配列
public:
	CValue(int value)
	{
		type      = F_TAG_INT;
		i_value   = value;
	}

	CValue(double value)
	{
		type      = F_TAG_DOUBLE;
		d_value   = value;
	}

	CValue(const wstring &value)
	{
		type     = F_TAG_STRING;
		s_value  = value;
	}

	CValue(const wchar_t *value)
	{
		type     = F_TAG_STRING;
		s_value  = value;
	}
	
	CValue(const CValueSub &value)
	{
		switch(value.GetType()) {
		case F_TAG_INT:
			type      = F_TAG_INT;
			i_value   = value.i_value;
			break;
		case F_TAG_DOUBLE:
			type      = F_TAG_DOUBLE;
			d_value   = value.d_value;
			break;
		case F_TAG_STRING:
			type     = F_TAG_STRING;
			s_value  = value.s_value;
			break;
		default:
			type     = F_TAG_STRING;
			break;
		};
	}

	CValue(int tp, int dmy) { type = tp; }	// 型指定して初期化　dmyはダミー
	CValue(void) { type = F_TAG_STRING; }
	~CValue(void) {}

	void	SetType(int tp) { type = tp; }
	int		GetType(void) const { return type; }

	char	GetTruth(void) const
	{
		switch(type) {
		case F_TAG_INT:	   return (i_value)        ? 1 : 0;
		case F_TAG_DOUBLE: return (d_value == 0.0) ? 0 : 1;
		case F_TAG_STRING: return (s_value == L"") ? 0 : 1;
		case F_TAG_ARRAY:  return (array.size())   ? 1 : 0;
		default:
			break;
		};
		return 0;
	}

	CValue	*SafeThis(void);

	int		GetValueInt(void) const;
	double	GetValueDouble(void) const;
	wstring	GetValueString(void) const;
	wstring	GetValueStringForLogging(void) const;

	void	SetArrayValue(const CValue &oval, const CValue &value);

	int		DecodeArrayOrder(int &order, int &order1, wstring &delimiter) const;

	CValue	&operator =(int value);
	CValue	&operator =(double value);
	CValue	&operator =(const wstring &value);
	CValue	&operator =(const wchar_t *value);
	CValue	&operator =(const vector<CValueSub> &value);
	CValue	&operator =(const CValueSub &value);

	CValue	operator +(const CValue &value) const;
	CValue	operator -(const CValue &value) const;
	CValue	operator *(const CValue &value) const;
	CValue	operator /(const CValue &value) const;
	CValue	operator %(const CValue &value) const;

	CValue	operator [](const CValue &value) const;

	CValue	operator ==(const CValue &value) const;
	CValue	operator !=(const CValue &value) const;
	CValue	operator >=(const CValue &value) const;
	CValue	operator <=(const CValue &value) const;
	CValue	operator >(const CValue &value) const;
	CValue	operator <(const CValue &value) const;

	CValue	operator ||(const CValue &value) const;
	CValue	operator &&(const CValue &value) const;
};

//----

#endif
