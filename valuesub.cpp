// 
// AYA version 5
//
// 配列に積まれている値を扱うクラス　CValueSub
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#else
#  include <math.h>
#endif
#include "function.h"
#include "value.h"
#include "misc.h"
#include "log.h"
#include "wsex.h"
#include "ccct.h"

extern CLog logger;

/* -----------------------------------------------------------------------
 *  関数名  ：  CValueSub::GetValueInt
 *  機能概要：  値をintで取得します
 *
 *  返値　　：  0/1/2=エラー発生/取得できた/取得できた(型が読み替えられた)
 * -----------------------------------------------------------------------
 */
int	CValueSub::GetValueInt(void) const
{
	switch(type) {
	case F_TAG_INT:
		return i_value;
	case F_TAG_DOUBLE:
		return (int)floor(d_value);
	case F_TAG_STRING:
		return ws_atoi(s_value, 10);
	default:
		return 0;
	};
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CValueSub::GetValueDouble
 *  機能概要：  値をdoubleで取得します
 *
 *  返値　　：  0/1/2=エラー発生/取得できた/取得できた(型が読み替えられた)
 * -----------------------------------------------------------------------
 */
double	CValueSub::GetValueDouble(void) const
{
	switch(type) {
	case F_TAG_INT:
		return (double)i_value;
	case F_TAG_DOUBLE:
		return d_value;
	case F_TAG_STRING:
		return ws_atof(s_value);
	default:
		return 0.0;
	};
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CValueSub::GetValue
 *  機能概要：  値をwstringで返します
 * -----------------------------------------------------------------------
 */
wstring	CValueSub::GetValueString(void) const
{
	switch(type) {
	case F_TAG_INT: {
			wstring	result;
			ws_itoa(result, i_value, 10);
			return result;
		}
	case F_TAG_DOUBLE: {
			wstring	result;
			ws_ftoa(result, d_value);
			return result;
		}
	case F_TAG_STRING:
		return s_value;
	default:
		return wstring(L"");
	};
}

/* -----------------------------------------------------------------------
 *  operator = (int)
 * -----------------------------------------------------------------------
 */
CValueSub &CValueSub::operator =(int value)
{
	type    = F_TAG_INT;
	i_value = value;
	s_value = L"";

	return *this;
}

/* -----------------------------------------------------------------------
 *  operator = (double)
 * -----------------------------------------------------------------------
 */
CValueSub &CValueSub::operator =(double value)
{
	type    = F_TAG_DOUBLE;
	d_value = value;
	s_value = L"";

	return *this;
}

/* -----------------------------------------------------------------------
 *  operator = (wstring)
 * -----------------------------------------------------------------------
 */
CValueSub &CValueSub::operator =(const wstring &value)
{
	type    = F_TAG_STRING;
	s_value = value;

	return *this;
}

/* -----------------------------------------------------------------------
 *  operator = (wchar_t*)
 * -----------------------------------------------------------------------
 */
CValueSub &CValueSub::operator =(const wchar_t *value)
{
	type    = F_TAG_STRING;
	s_value = value;

	return *this;
}

/* -----------------------------------------------------------------------
 *  operator + (CValueSub)
 *
 *  int+doubleはdouble扱い、intもしくはdouble+wstringは文字列結合扱いとなります。
 * -----------------------------------------------------------------------
 */
CValueSub CValueSub::operator +(const CValueSub &value) const
{
	CValueSub	result;

	switch(type) {
	case F_TAG_INT:
		switch(value.type) {
		case F_TAG_INT:
			result = i_value + value.i_value;
			break;
		case F_TAG_DOUBLE:
			result = (double)i_value + value.d_value;
			break;
		case F_TAG_STRING: {
				wstring	tmpstr;
				ws_itoa(tmpstr, i_value, 10);
				tmpstr += value.s_value;
				result = tmpstr;
			}
			break;
		default:
			result = L"";
			break;
		};
		break;
	case F_TAG_DOUBLE:
		switch(value.type) {
		case F_TAG_INT:
			result = d_value + (double)value.i_value;
			break;
		case F_TAG_DOUBLE:
			result = d_value + value.d_value;
			break;
		case F_TAG_STRING: {
				wstring	tmpstr;
				ws_ftoa(tmpstr, d_value);
				tmpstr += value.s_value;
				result = tmpstr;
			}
			break;
		default:
			result = L"";
			break;
		};
		break;
	case F_TAG_STRING:
		switch(value.type) {
		case F_TAG_INT: {
				wstring	tmpstr;
				ws_itoa(tmpstr, value.i_value, 10);
				tmpstr.insert(0, s_value);
				result = tmpstr;
			}
			break;
		case F_TAG_DOUBLE: {
				wstring	tmpstr;
				ws_ftoa(tmpstr, value.d_value);
				tmpstr.insert(0, s_value);
				result = tmpstr;
			}
			break;
		case F_TAG_STRING:
			result = s_value + value.s_value;
			break;
		default:
			result = L"";
			break;
		};
		break;
	default:
		result = L"";
		break;
	};

	return result;
}

/* -----------------------------------------------------------------------
 *  operator - (CValueSub)
 *
 *  intとdoubleの演算はdouble扱いです。wstringとの演算は空文字列を返します。
 * -----------------------------------------------------------------------
 */
CValueSub CValueSub::operator -(const CValueSub &value) const
{
	CValueSub	result;

	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			result = i_value - value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = (double)i_value - value.d_value;
		else
			result = L"";
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			result = d_value - (double)value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = d_value - value.d_value;
		else
			result = L"";
	}
	else
		result = L"";

	return result;
}

/* -----------------------------------------------------------------------
 *  operator * (CValueSub)
 *
 *  intとdoubleの演算はdouble扱いです。wstringとの演算は空文字列を返します。
 * -----------------------------------------------------------------------
 */
CValueSub CValueSub::operator *(const CValueSub &value) const
{
	CValueSub	result;

	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			result = i_value*value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = (double)i_value*value.d_value;
		else
			result = L"";
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			result = d_value*(double)value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = d_value*value.d_value;
		else
			result = L"";
	}
	else
		result = L"";

	return result;
}

/* -----------------------------------------------------------------------
 *  operator / (CValueSub)
 *
 *  intとdoubleの演算はdouble扱いです。wstringとの演算は空文字列を返します。
 * -----------------------------------------------------------------------
 */
CValueSub CValueSub::operator /(const CValueSub &value) const
{
	CValueSub	result;

	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			result = (!value.i_value) ? 0 : i_value/value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = (value.d_value == 0.0) ? 0.0 : (double)i_value/value.d_value;
		else
			result = L"";
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			result = (!value.i_value) ? 0 : d_value/(double)value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = (value.d_value == 0.0) ? 0.0 : d_value/value.d_value;
		else
			result = L"";
	}
	else
		result = L"";

	return result;
}

/* -----------------------------------------------------------------------
 *  operator % (CValueSub)
 *
 *  intとdoubleの演算はdouble扱いです。wstringとの演算は空文字列を返します。
 * -----------------------------------------------------------------------
 */
CValueSub CValueSub::operator %(const CValueSub &value) const
{
	CValueSub	result;

	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			result = (!value.i_value) ? 0 : i_value%value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = (value.d_value == 0.0) ? 0 : i_value%(int)value.d_value;
		else
			result = L"";
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			result = (!value.i_value) ? 0 : (int)d_value%value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = (value.d_value == 0.0) ? 0 : (int)d_value%(int)value.d_value;
		else
			result = L"";
	}
	else
		result = L"";

	return result;
}

/* -----------------------------------------------------------------------
 *  operator == (CValueSub)
 *
 *  intとdoubleの演算はdouble扱いです。wstringとの演算は空文字列を返します。
 * -----------------------------------------------------------------------
 */
CValueSub CValueSub::operator ==(const CValueSub &value) const
{
	CValueSub	result;

	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			result = (i_value == value.i_value) ? 1 : 0;
		else if (value.type == F_TAG_DOUBLE)
			result = ((double)i_value == value.d_value) ? 1 : 0;
		else
			result = 0;
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			result = (d_value == (double)value.i_value) ? 1 : 0;
		else if (value.type == F_TAG_DOUBLE)
			result = (d_value == value.d_value) ? 1 : 0;
		else
			result = 0;
	}
	else if (type == F_TAG_STRING) {
		if (value.type == F_TAG_STRING)
			result = (!s_value.compare(value.s_value)) ? 1 : 0;
		else
			result = 0;
	}
	else
		result = 0;

	return result;
}

