// 
// AYA version 5
//
// �l�������N���X�@CValue
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

extern CLog	logger;

#define	__GETMAX(a, b)	( ((a) > (b)) ? (a) : (b) )
#define	__GETMIN(a, b)	( ((a) < (b)) ? (a) : (b) )

/* -----------------------------------------------------------------------
 *  �֐���  �F  CValue::SafeThis
 *  �@�\�T�v�F  this��Ԃ��܂��B
 *  �@�@�@�@�@  �������̃C���X�^���X���ێ�����l�Ɍ^�������ꍇ�́A�Ԃ��O�ɋ󕶎���ɕϊ����܂��B
 * -----------------------------------------------------------------------
 */
CValue	*CValue::SafeThis(void)
{
	if (type == F_TAG_NOP) {
		type    = F_TAG_STRING;
		s_value = L"";
	}

	return this;
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CValue::GetValueInt
 *  �@�\�T�v�F  �l��int�ŕԂ��܂�
 *
 *  �^���z��ł������ꍇ��0��Ԃ��܂�
 * -----------------------------------------------------------------------
 */
int	CValue::GetValueInt(void) const
{
	switch(type) {
	case F_TAG_INT:
		return i_value;
	case F_TAG_DOUBLE:
		return (int)floor(d_value);
	case F_TAG_STRING:
		return ws_atoi(s_value, 10);
	case F_TAG_ARRAY:
		return 0;
	default:
		return 0;
	};
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CValue::GetValueDouble
 *  �@�\�T�v�F  �l��double�ŕԂ��܂�
 *
 *  �^���z��ł������ꍇ��0.0��Ԃ��܂�
 * -----------------------------------------------------------------------
 */
double	CValue::GetValueDouble(void) const
{
	switch(type) {
	case F_TAG_INT:
		return (double)i_value;
	case F_TAG_DOUBLE:
		return d_value;
	case F_TAG_STRING:
		return ws_atof(s_value);
	case F_TAG_ARRAY:
		return 0.0;
	default:
		return 0.0;
	};
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CValue::GetValueString
 *  �@�\�T�v�F  �l��wstring�ŕԂ��܂�
 *
 *  �^���z��ł������ꍇ�̓f���~�^�Ō�������������ɕϊ����ĕԂ��܂�
 * -----------------------------------------------------------------------
 */
wstring	CValue::GetValueString(void) const
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
	case F_TAG_ARRAY: {
			wstring	result;
			for(vector<CValueSub>::const_iterator it = array.begin();
				it != array.end(); it++) {
				if (it != array.begin())
					result += VAR_DELIMITER;
				wstring	tmpstr = it->GetValueString();
				result += tmpstr;
			}
			return result;
		}
	default:
		return wstring(L"");
	};
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CValue::GetValueStringForLogging
 *  �@�\�T�v�F  �l��wstring�ŕԂ��܂��i���K�[�p�j
 *
 *  GetValueString�Ƃ̈Ⴂ�́A��������_�u���N�H�[�g���邩�ۂ��ł��B
 * -----------------------------------------------------------------------
 */
wstring	CValue::GetValueStringForLogging(void) const
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
	case F_TAG_STRING: {
			wstring	result = s_value;
			AddDoubleQuote(result);
			return result;
		}
	case F_TAG_ARRAY: {
			wstring	result;
			for(vector<CValueSub>::const_iterator it = array.begin();
				it != array.end(); it++) {
				if (it != array.begin())
					result += VAR_DELIMITER;
				wstring	tmpstr = it->GetValueString();
				if (it->GetType() == F_TAG_STRING)
					AddDoubleQuote(tmpstr);
				result += tmpstr;
			}
			return result;
		}
	default:
		return wstring(L"");
	};
}
/* -----------------------------------------------------------------------
 *  �֐���  �F  CValue::SetArrayValue
 *  �@�\�T�v�F  �z��̎w�肵���ʒu�֒l��ݒ肵�܂��B�K�v�ɉ����Č^�ϊ����s���܂�
 *
 *  ���̌^���ȈՔz��Ɣėp�z��̏ꍇ�͂��̂܂܏������܂����A����/�����������ꍇ��
 *  �ėp�z��Ɍ^�ϊ�����A���̒l��[0]�Ɋi�[����܂��B
 * -----------------------------------------------------------------------
 */
void	CValue::SetArrayValue(const CValue &oval, const CValue &value)
{
	// �^�ϊ����Č��̒l��[0]�֑��
	if (type == F_TAG_INT) {
		CValueSub	addvs(i_value);
		vector<CValueSub>	addvsarray;
		addvsarray.push_back(addvs);
		*this = addvsarray;
	}
	else if (type == F_TAG_DOUBLE) {
		CValueSub	addvs(value.d_value);
		vector<CValueSub>	addvsarray;
		addvsarray.push_back(addvs);
		*this = addvsarray;
	}

	// �����ƃf���~�^�̎擾
	int	order, order1;
	wstring	delimiter;
	int	aoflg = oval.DecodeArrayOrder(order, order1, delimiter);

	// �l���X�V����
	if (type == F_TAG_STRING) {
		// �ȈՔz��

		// ����l��������łȂ��ꍇ�͉������Ȃ�
		if (value.GetType() != F_TAG_STRING)
			return;
		// ���̕�������f���~�^�ŕ���
		vector<wstring>	s_array;
		int	sz = SplitToMultiString(s_value, s_array, delimiter);
		// �X�V
		if (aoflg) {
			// �͈͂�
			if (order1 < 0)
				return;
			else if (order < sz) {
				int	s_index   = __GETMAX(order, 0);
				int	e_index   = __GETMIN(order1 + 1, sz);
				int	s_indexp1 = s_index + 1;
				int	i         = 0;
				s_array[s_index] = value.s_value;
				for(vector<wstring>::iterator it = s_array.begin();
					it != s_array.end(); i++) {
					if (s_indexp1 <= i && i < e_index)
						it = s_array.erase(it);
					else if (i >= e_index)
						break;
					else
						it++;
				}
			}
			else {
				int	addsize = order - s_array.size();
				for(int i = 0; i < addsize; i++) {
					wstring	addstr;
					s_array.push_back(addstr);
				}
				s_array.push_back(value.s_value);
			}
		}
		else {
			// �͈͂Ȃ�
			if (order < 0)
				return;
			else if (order < sz)
				s_array[order] = value.s_value;
			else {
				int	addsize = order - sz;
				for(int i = 0; i < addsize; i++) {
					wstring	addstr;
					s_array.push_back(addstr);
				}
				s_array.push_back(value.s_value);
			}
		}
		// ������̕���
		sz = s_array.size();
		if (!sz)
			s_value = L"";
		else {
			s_value = s_array[0];
			for(int i = 1; i < sz; i++)
				s_value += delimiter + s_array[i];
		}
	}
	else if (type == F_TAG_ARRAY) {
		// �ėp�z��

		if (aoflg) {
			int	sz = array.size();
			// �͈͂�
			if (order1 < 0)
				return;
			if (order < sz) {
				// �z�񒆓r�̏�������
				int	s_index   = __GETMAX(order, 0);
				int	e_index   = __GETMIN(order1 + 1, sz);
				int	i         = 0;
				if (value.GetType() == F_TAG_INT)
					array[s_index] = value.i_value;
				else if (value.GetType() == F_TAG_DOUBLE)
					array[s_index] = value.d_value;
				else if (value.GetType() == F_TAG_STRING)
					array[s_index] = value.s_value;
				else if (value.GetType() == F_TAG_ARRAY) {
					i = 0;
					for(vector<CValueSub>::iterator it = array.begin(); it != array.end(); it++, i++)
						if (i == s_index) {
							it = array.erase(it);
							array.insert(it, value.array.begin(), value.array.end());
							s_index += (value.array.size() - 1);
							e_index += (value.array.size() - 1);
							break;
						}
				}
				i = 0;
				for(vector<CValueSub>::iterator it = array.begin(); it != array.end(); i++) {
					if (s_index < i && i < e_index)
						it = array.erase(it);
					else if (i >= e_index)
						break;
					else
						it++;
				}
			}
			else {
				// ��[�ւ̒ǉ�
				vector<CValueSub>	*t_array = &array;
				int	addsize = order - t_array->size();
				for(int i = 1; i <= addsize; i++) {
					CValueSub	addvs;
					t_array->push_back(addvs);
				}
				if (value.GetType() == F_TAG_INT)
					t_array->push_back(value.i_value);
				else if (value.GetType() == F_TAG_DOUBLE)
					t_array->push_back(value.d_value);
				else if (value.GetType() == F_TAG_STRING)
					t_array->push_back(value.s_value);
				else if (value.GetType() == F_TAG_ARRAY)
					t_array->insert(t_array->end(),
						value.array.begin(), value.array.end());
			}
		}
		else {
			// �͈͂Ȃ�
			if (order < 0)
				return;
			if (order < array.size()) {
				// �z�񒆓r�̏�������
				if (value.GetType() == F_TAG_INT)
					array[order] = value.i_value;
				else if (value.GetType() == F_TAG_DOUBLE)
					array[order] = value.d_value;
				else if (value.GetType() == F_TAG_STRING)
					array[order] = value.s_value;
				else if (value.GetType() == F_TAG_ARRAY) {
					for(vector<CValueSub>::iterator it = array.begin(); it != array.end(); it++, order--)
						if (!order) {
							it = array.erase(it);
							array.insert(it, value.array.begin(), value.array.end());
							break;
						}
				}
			}
			else {
				// ��[�ւ̒ǉ�
				vector<CValueSub>	*t_array = &array;
				int	addsize = order - t_array->size();
				for(int i = 1; i <= addsize; i++) {
					CValueSub	addvs;
					t_array->push_back(addvs);
				}
				if (value.GetType() == F_TAG_INT)
					t_array->push_back(value.i_value);
				else if (value.GetType() == F_TAG_DOUBLE)
					t_array->push_back(value.d_value);
				else if (value.GetType() == F_TAG_STRING)
					t_array->push_back(value.s_value);
				else if (value.GetType() == F_TAG_ARRAY)
					t_array->insert(t_array->end(),
						value.array.begin(), value.array.end());
			}
		}
	}
}

/* -----------------------------------------------------------------------
 *  �֐���  �F  CValue::DecodeArrayOrder
 *  �@�\�T�v�F  ����̏����Ŕz�񏘐����i�[���Ă���this���珘���ƃf���~�^���擾���܂�
 *
 *  �Ԓl�@�@�F  0/1=order1(�͈͎w��)����/�L��
 * -----------------------------------------------------------------------
 */
int	CValue::DecodeArrayOrder(int &order, int &order1, wstring &delimiter) const
{
	order  = 0;
	order1 = 0;
	delimiter = VAR_DELIMITER;

	if (type == F_TAG_ARRAY) {
		int	sz = array.size();
		if (sz) {
			// �v�f0:����
			if (array[0].GetType() == F_TAG_INT)
				order = array[0].i_value;
			else if (array[0].GetType() == F_TAG_DOUBLE)
				order = (int)floor(array[0].d_value);
			else
				return 0;
			if (sz == 1)
				return 0;
			// �v�f1:���l�Ȃ�͈͎w��A������Ȃ�f���~�^
			switch(array[1].GetType()) {
			case F_TAG_INT:
				order1  = array[1].i_value;
				if (order > order1)
					exchange(order, order1);
				break;
			case F_TAG_DOUBLE:
				order1  = (int)floor(array[1].d_value);
				if (order > order1)
					exchange(order, order1);
				break;
			case F_TAG_STRING:
				if (array[1].s_value.size())
					delimiter = array[1].s_value;
				return 0;
			default:
				return 0;
			};
			if (sz == 2)
				return 1;
			// �v�f2:�v�f1���͈͎w�肾�����ꍇ�̓f���~�^
			if (array[2].GetType() == F_TAG_STRING &&
				array[2].s_value.size())
				delimiter = array[2].s_value;
			return 1;
		}
	}

	return 0;
}

/* -----------------------------------------------------------------------
 *  operator = (int)
 * -----------------------------------------------------------------------
 */
CValue &CValue::operator =(int value)
{
	type    = F_TAG_INT;
	i_value = value;

	return *this;
}

/* -----------------------------------------------------------------------
 *  operator = (double)
 * -----------------------------------------------------------------------
 */
CValue &CValue::operator =(double value)
{
	type    = F_TAG_DOUBLE;
	d_value = value;

	return *this;
}

/* -----------------------------------------------------------------------
 *  operator = (wstring)
 * -----------------------------------------------------------------------
 */
CValue &CValue::operator =(const wstring &value)
{
	type    = F_TAG_STRING;
	s_value = value;

	return *this;
}

/* -----------------------------------------------------------------------
 *  operator = (wchar_t*)
 * -----------------------------------------------------------------------
 */
CValue &CValue::operator =(const wchar_t *value)
{
	type    = F_TAG_STRING;
	s_value = value;

	return *this;
}

/* -----------------------------------------------------------------------
 *  operator = (vector<CValueSub>)
 * -----------------------------------------------------------------------
 */
CValue &CValue::operator =(const vector<CValueSub> &value)
{
	type    = F_TAG_ARRAY;
	array.assign(value.begin(), value.end());

	return *this;
}

/* -----------------------------------------------------------------------
 *  operator = (CValueSub)
 * -----------------------------------------------------------------------
 */
CValue &CValue::operator =(const CValueSub &value)
{
	switch(value.GetType()) {
	case F_TAG_INT:
		*this = value.i_value;
		break;
	case F_TAG_DOUBLE:
		*this = value.d_value;
		break;
	case F_TAG_STRING:
		*this = value.s_value;
		break;
	default:
		*this = L"";
		break;
	};

	return *this;
}

/* -----------------------------------------------------------------------
 *  operator + (CValue)
 *
 *  int+double��double�����Aint��������double+wstring�͕����񌋍������ƂȂ�܂��B
 * -----------------------------------------------------------------------
 */
CValue CValue::operator +(const CValue &value) const
{
	CValue	result;

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
		case F_TAG_ARRAY: {
				result.type = F_TAG_ARRAY;
				CValueSub	t_vs = i_value;
				for(vector<CValueSub>::const_iterator it = value.array.begin(); it != value.array.end(); it++)
					result.array.push_back(t_vs + *it);
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
		case F_TAG_ARRAY: {
				result.type = F_TAG_ARRAY;
				CValueSub	t_vs = d_value;
				for(vector<CValueSub>::const_iterator it = value.array.begin(); it != value.array.end(); it++)
					result.array.push_back(t_vs + *it);
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
		case F_TAG_ARRAY: {
				result.type = F_TAG_ARRAY;
				CValueSub	t_vs = s_value;
				for(vector<CValueSub>::const_iterator it = value.array.begin(); it != value.array.end(); it++)
					result.array.push_back(t_vs + *it);
			}
			break;
		default:
			result = L"";
			break;
		};
		break;
	case F_TAG_ARRAY:
		switch(value.type) {
		case F_TAG_INT: {
				result.type = F_TAG_ARRAY;
				CValueSub	t_vs = value.i_value;
				for(vector<CValueSub>::const_iterator it = array.begin(); it != array.end(); it++)
					result.array.push_back(*it + t_vs);
			}
			break;
		case F_TAG_DOUBLE: {
				result.type = F_TAG_ARRAY;
				CValueSub	t_vs = value.d_value;
				for(vector<CValueSub>::const_iterator it = array.begin(); it != array.end(); it++)
					result.array.push_back(*it + t_vs);
			}
			break;
		case F_TAG_STRING: {
				result.type = F_TAG_ARRAY;
				CValueSub	t_vs = value.s_value;
				for(vector<CValueSub>::const_iterator it = array.begin(); it != array.end(); it++)
					result.array.push_back(*it + t_vs);
			}
			break;
		case F_TAG_ARRAY: {
				result.type = F_TAG_ARRAY;
				vector<CValueSub>::const_iterator it, it2;
				for(it = array.begin(), it2 = value.array.begin();
					it != array.end() && it2 != value.array.end(); it++, it2++)
					result.array.push_back((*it) + (*it2));
			}
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
 *  operator - (CValue)
 *
 *  int��double�̉��Z��double�����ł��Bwstring�Ƃ̉��Z�͋󕶎����Ԃ��܂��B
 * -----------------------------------------------------------------------
 */
CValue CValue::operator -(const CValue &value) const
{
	CValue	result;

	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			result = i_value - value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = (double)i_value - value.d_value;
		else if (value.type == F_TAG_ARRAY) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = i_value;
			for(vector<CValueSub>::const_iterator it = value.array.begin(); it != value.array.end(); it++)
				result.array.push_back(t_vs - *it);
		}
		else
			result = L"";
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			result = d_value - (double)value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = d_value - value.d_value;
		else if (value.type == F_TAG_ARRAY) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = d_value;
			for(vector<CValueSub>::const_iterator it = value.array.begin(); it != value.array.end(); it++)
				result.array.push_back(t_vs - *it);
		}
		else
			result = L"";
	}
	else if (type == F_TAG_ARRAY) {
		if (value.type == F_TAG_INT) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = value.i_value;
			for(vector<CValueSub>::const_iterator it = array.begin(); it != array.end(); it++)
				result.array.push_back(*it - t_vs);
		}
		else if (value.type == F_TAG_DOUBLE) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = value.d_value;
			for(vector<CValueSub>::const_iterator it = array.begin(); it != array.end(); it++)
				result.array.push_back(*it - t_vs);
			result.type = F_TAG_ARRAY;
		}
		else if (value.type == F_TAG_ARRAY) {
			result.type = F_TAG_ARRAY;
			vector<CValueSub>::const_iterator it, it2;
			for(it = array.begin(), it2 = value.array.begin();
				it != array.end() && it2 != value.array.end(); it++, it2++)
				result.array.push_back((*it) - (*it2));
		}
	}
	else
		result = L"";

	return result;
}

/* -----------------------------------------------------------------------
 *  operator * (CValue)
 *
 *  int��double�̉��Z��double�����ł��Bwstring�Ƃ̉��Z�͋󕶎����Ԃ��܂��B
 * -----------------------------------------------------------------------
 */
CValue CValue::operator *(const CValue &value) const
{
	CValue	result;

	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			result = i_value*value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = (double)i_value*value.d_value;
		else if (value.type == F_TAG_ARRAY) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = i_value;
			for(vector<CValueSub>::const_iterator it = value.array.begin(); it != value.array.end(); it++)
				result.array.push_back(t_vs*(*it));
		}
		else
			result = L"";
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			result = d_value*(double)value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = d_value*value.d_value;
		else if (value.type == F_TAG_ARRAY) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = d_value;
			for(vector<CValueSub>::const_iterator it = value.array.begin(); it != value.array.end(); it++)
				result.array.push_back(t_vs*(*it));
		}
		else
			result = L"";
	}
	else if (type == F_TAG_ARRAY) {
		if (value.type == F_TAG_INT) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = value.i_value;
			for(vector<CValueSub>::const_iterator it = array.begin(); it != array.end(); it++)
				result.array.push_back((*it)*t_vs);
		}
		else if (value.type == F_TAG_DOUBLE) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = value.d_value;
			for(vector<CValueSub>::const_iterator it = array.begin(); it != array.end(); it++)
				result.array.push_back((*it)*t_vs);
			result.type = F_TAG_ARRAY;
		}
		else if (value.type == F_TAG_ARRAY) {
			result.type = F_TAG_ARRAY;
			vector<CValueSub>::const_iterator it, it2;
			for(it = array.begin(), it2 = value.array.begin();
				it != array.end() && it2 != value.array.end(); it++, it2++)
				result.array.push_back((*it)*(*it2));
		}
	}
	else
		result = L"";

	return result;
}

/* -----------------------------------------------------------------------
 *  operator / (CValue)
 *
 *  int��double�̉��Z��double�����ł��Bwstring�Ƃ̉��Z�͋󕶎����Ԃ��܂��B
 * -----------------------------------------------------------------------
 */
CValue CValue::operator /(const CValue &value) const
{
	CValue	result;

	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			result = (!value.i_value) ? 0 : i_value/value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = (value.d_value == 0.0) ? 0.0 : (double)i_value/value.d_value;
		else if (value.type == F_TAG_ARRAY) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = i_value;
			for(vector<CValueSub>::const_iterator it = value.array.begin(); it != value.array.end(); it++)
				result.array.push_back(t_vs/(*it));
		}
		else
			result = L"";
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			result = (!value.i_value) ? 0 : d_value/(double)value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = (value.d_value == 0.0) ? 0.0 : d_value/value.d_value;
		else if (value.type == F_TAG_ARRAY) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = d_value;
			for(vector<CValueSub>::const_iterator it = value.array.begin(); it != value.array.end(); it++)
				result.array.push_back(t_vs/(*it));
		}
		else
			result = L"";
	}
	else if (type == F_TAG_ARRAY) {
		if (value.type == F_TAG_INT) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = value.i_value;
			for(vector<CValueSub>::const_iterator it = array.begin(); it != array.end(); it++)
				result.array.push_back((*it)/t_vs);
		}
		else if (value.type == F_TAG_DOUBLE) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = value.d_value;
			for(vector<CValueSub>::const_iterator it = array.begin(); it != array.end(); it++)
				result.array.push_back((*it)/t_vs);
			result.type = F_TAG_ARRAY;
		}
		else if (value.type == F_TAG_ARRAY) {
			result.type = F_TAG_ARRAY;
			vector<CValueSub>::const_iterator it, it2;
			for(it = array.begin(), it2 = value.array.begin();
				it != array.end() && it2 != value.array.end(); it++, it2++)
				result.array.push_back((*it)/(*it2));
		}
	}
	else
		result = L"";

	return result;
}

/* -----------------------------------------------------------------------
 *  operator % (CValue)
 *
 *  int��double�̉��Z��double�����ł��Bwstring�Ƃ̉��Z�͋󕶎����Ԃ��܂��B
 * -----------------------------------------------------------------------
 */
CValue CValue::operator %(const CValue &value) const
{
	CValue	result;

	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			result = (!value.i_value) ? 0 : i_value%value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = (value.d_value == 0.0) ? 0 : i_value%(int)value.d_value;
		else if (value.type == F_TAG_ARRAY) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = i_value;
			for(vector<CValueSub>::const_iterator it = value.array.begin(); it != value.array.end(); it++)
				result.array.push_back(t_vs%(*it));
		}
		else
			result = L"";
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			result = (!value.i_value) ? 0 : (int)d_value%value.i_value;
		else if (value.type == F_TAG_DOUBLE)
			result = (value.d_value == 0.0) ? 0 : (int)d_value%(int)value.d_value;
		else if (value.type == F_TAG_ARRAY) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = d_value;
			for(vector<CValueSub>::const_iterator it = value.array.begin(); it != value.array.end(); it++)
				result.array.push_back(t_vs%(*it));
		}
		else
			result = L"";
	}
	else if (type == F_TAG_ARRAY) {
		if (value.type == F_TAG_INT) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = value.i_value;
			for(vector<CValueSub>::const_iterator it = array.begin(); it != array.end(); it++)
				result.array.push_back((*it)%t_vs);
		}
		else if (value.type == F_TAG_DOUBLE) {
			result.type = F_TAG_ARRAY;
			CValueSub	t_vs = value.d_value;
			for(vector<CValueSub>::const_iterator it = array.begin(); it != array.end(); it++)
				result.array.push_back((*it)%t_vs);
			result.type = F_TAG_ARRAY;
		}
		else if (value.type == F_TAG_ARRAY) {
			result.type = F_TAG_ARRAY;
			vector<CValueSub>::const_iterator it, it2;
			for(it = array.begin(), it2 = value.array.begin();
				it != array.end() && it2 != value.array.end(); it++, it2++)
				result.array.push_back((*it)%(*it2));
		}
	}
	else
		result = L"";

	return result;
}

/* -----------------------------------------------------------------------
 *  operator [] (CValue)
 *
 *  this�̌^��wstring�̏ꍇ�͊ȈՔz��Aarray�̏ꍇ�͔z�񈵂��ł��B
 *  int/double�ł͏����ɂ�炸���̒l���Ԃ���܂��B
 *
 *  �������͈͊O�̏ꍇ�͋󕶎����Ԃ��܂��B
 *
 *  �����̌^�͏��array�ł���A����̃t�H�[�}�b�g�ɏ������Ă���K�v������܂��B
 *  �i�Ăяo�����ł��̂悤�ɐ��`����K�v������܂��j
 * -----------------------------------------------------------------------
 */
CValue CValue::operator [](const CValue &value) const
{
	int	order, order1;
	wstring	delimiter;
	int	aoflg = value.DecodeArrayOrder(order, order1, delimiter);

	if (type == F_TAG_INT || type == F_TAG_DOUBLE) {
		// ���l�@������0�Ȃ�this�A1�ȊO�ł͋󕶎����Ԃ�
		if (!order)
			return *this;
		else
			return CValue();
	}
	if (type == F_TAG_STRING) {
		// �ȈՔz��

		// ��������f���~�^�ŕ���
		vector<wstring>	s_array;
		int	sz = SplitToMultiString(s_value, s_array, delimiter);
		// �l�̎擾
		if (aoflg) {
			// �͈͂���
			if (order1 < 0 || order >= sz)
				return CValue();
			else {
				int	s_index   = __GETMAX(order, 0);
				int	e_index   = __GETMIN(order1 + 1, sz);
				int	i         = 0;
				int	j         = 0;
				wstring	result_str;
				for(vector<wstring>::iterator it = s_array.begin();
					it != s_array.end(); it++, i++) {
					if (s_index <= i && i < e_index) {
						if (j)
							result_str += delimiter;
						j = 1;
						result_str += *it;
					}
					else if (i >= e_index)
						break;
				}
				return result_str;
			}
		}
		else {
			// �͈͂Ȃ�
			if (0 <= order && order < sz)
				return CValue(s_array[order]);
			else 
				return CValue();
		}
	}
	else if (type == F_TAG_ARRAY) {
		// �ėp�z��

		int	sz = array.size();
		// �l�̎擾
		if (aoflg) {
			// �͈͂���
			if (order1 < 0 || order >= sz)
				return CValue(F_TAG_ARRAY, 0/*dmy*/);
			else {
				int	s_index   = __GETMAX(order, 0);
				int	e_index   = __GETMIN(order1 + 1, sz);
				int	i         = 0;
				CValue	result_array(F_TAG_ARRAY, 0/*dmy*/);
				for(vector<CValueSub>::const_iterator it = array.begin();
					it != array.end(); it++, i++) {
					if (s_index <= i && i < e_index)
						result_array.array.push_back(*it);
					else if (i >= e_index)
						break;
				}
				return result_array;
			}
		}
		else {
			// �͈͂Ȃ�
			if (0 <= order && order < sz) {
				CValue result;
				result = array[order];
				return result;
			}
			else 
				return CValue(F_TAG_ARRAY, 0/*dmy*/);
		}
	}

	return CValue();
}

/* -----------------------------------------------------------------------
 *  operator == (CValue)
 *
 *  int��double�̔�r��double�����ł��Bint/double��wstring/array��Aarray�Ƃ��̑��Ƃ̔�r��
 *  ��ɋU�ƂȂ�܂��B
 * -----------------------------------------------------------------------
 */
CValue CValue::operator ==(const CValue &value) const
{
	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			return CValue((i_value == value.i_value) ? 1 : 0);
		else if (value.type == F_TAG_DOUBLE)
			return CValue(((double)i_value == value.d_value) ? 1 : 0);
		else
			return CValue(0);
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			return CValue((d_value == (double)value.i_value) ? 1 : 0);
		else if (value.type == F_TAG_DOUBLE)
			return CValue((d_value == value.d_value) ? 1 : 0);
		else
			return CValue(0);
	}
	else if (type == F_TAG_STRING) {
		if (value.type == F_TAG_STRING)
			return CValue((!s_value.compare(value.s_value)) ? 1 : 0);
		else
			return CValue(0);
	}
	else if (type == F_TAG_ARRAY) {
		if (value.type == F_TAG_ARRAY) {
			int	len = array.size();
			if (len != value.array.size())
				return CValue(0);
			else {
				vector<CValueSub>::const_iterator it, it2;
				int	i = 0;
				for(it = array.begin(), it2 = value.array.begin();
					it != array.end() && it2 != value.array.end(); it++, it2++)
					i += ((*it) == (*it2)).GetValueInt();
				return CValue((i == len) ? 1 : 0);
			}
		}
		else
			return CValue(0);
	}

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  operator != (CValue)
 * -----------------------------------------------------------------------
 */
CValue CValue::operator !=(const CValue &value) const
{
	return CValue(1 - (*this == value).GetValueInt());
}

/* -----------------------------------------------------------------------
 *  operator >= (CValue)
 *
 *  int��double�̔�r��double�����ł��Bint/double��wstring/array��Aarray�Ƃ��̑��Ƃ̔�r��
 *  ��ɋU�ƂȂ�܂��B
 *  wstring�̏ꍇ�͎������̔�r�ƂȂ�܂��B
 * -----------------------------------------------------------------------
 */
CValue CValue::operator >=(const CValue &value) const
{
	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			return CValue((i_value >= value.i_value) ? 1 : 0);
		else if (value.type == F_TAG_DOUBLE)
			return CValue(((double)i_value >= value.d_value) ? 1 : 0);
		else
			return CValue(0);
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			return CValue((d_value >= (double)value.i_value) ? 1 : 0);
		else if (value.type == F_TAG_DOUBLE)
			return CValue((d_value >= value.d_value) ? 1 : 0);
		else
			return CValue(0);
	}
	else if (type == F_TAG_STRING) {
		if (value.type == F_TAG_STRING)
			return CValue((wcscmp(s_value.c_str(), value.s_value.c_str()) >= 0) ? 1 : 0);
		else
			return CValue(0);
	}

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  operator <= (CValue)
 *
 *  int��double�̔�r��double�����ł��Bint/double��wstring/array��Aarray�Ƃ��̑��Ƃ̔�r��
 *  ��ɋU�ƂȂ�܂��B
 *  wstring�̏ꍇ�͎������̔�r�ƂȂ�܂��B
 * -----------------------------------------------------------------------
 */
CValue CValue::operator <=(const CValue &value) const
{
	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			return CValue((i_value <= value.i_value) ? 1 : 0);
		else if (value.type == F_TAG_DOUBLE)
			return CValue(((double)i_value <= value.d_value) ? 1 : 0);
		else
			return CValue(0);
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			return CValue((d_value<= (double)value.i_value) ? 1 : 0);
		else if (value.type == F_TAG_DOUBLE)
			return CValue((d_value <= value.d_value) ? 1 : 0);
		else
			return CValue(0);
	}
	else if (type == F_TAG_STRING) {
		if (value.type == F_TAG_STRING)
			return CValue((wcscmp(s_value.c_str(), value.s_value.c_str()) <= 0) ? 1 : 0);
		else
			return CValue(0);
	}

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  operator > (CValue)
 *
 *  int��double�̔�r��double�����ł��Bint/double��wstring/array��Aarray�Ƃ��̑��Ƃ̔�r��
 *  ��ɋU�ƂȂ�܂��B
 *  wstring�̏ꍇ�͎������̔�r�ƂȂ�܂��B
 * -----------------------------------------------------------------------
 */
CValue CValue::operator >(const CValue &value) const
{
	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			return CValue((i_value > value.i_value) ? 1 : 0);
		else if (value.type == F_TAG_DOUBLE)
			return CValue(((double)i_value > value.d_value) ? 1 : 0);
		else
			return CValue(0);
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			return CValue((d_value > (double)value.i_value) ? 1 : 0);
		else if (value.type == F_TAG_DOUBLE)
			return CValue((d_value > value.d_value) ? 1 : 0);
		else
			return CValue(0);
	}
	else if (type == F_TAG_STRING) {
		if (value.type == F_TAG_STRING)
			return CValue((wcscmp(s_value.c_str(), value.s_value.c_str()) > 0) ? 1 : 0);
		else
			return CValue(0);
	}

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  operator < (CValue)
 *
 *  int��double�̔�r��double�����ł��Bint/double��wstring/array��Aarray�Ƃ��̑��Ƃ̔�r��
 *  ��ɋU�ƂȂ�܂��B
 *  wstring�̏ꍇ�͎������̔�r�ƂȂ�܂��B
 * -----------------------------------------------------------------------
 */
CValue CValue::operator <(const CValue &value) const
{
	if (type == F_TAG_INT) {
		if (value.type == F_TAG_INT)
			return CValue((i_value < value.i_value) ? 1 : 0);
		else if (value.type == F_TAG_DOUBLE)
			return CValue(((double)i_value < value.d_value) ? 1 : 0);
		else
			return CValue(0);
	}
	else if (type == F_TAG_DOUBLE) {
		if (value.type == F_TAG_INT)
			return CValue((d_value < (double)value.i_value) ? 1 : 0);
		else if (value.type == F_TAG_DOUBLE)
			return CValue((d_value < value.d_value) ? 1 : 0);
		else
			return CValue(0);
	}
	else if (type == F_TAG_STRING) {
		if (value.type == F_TAG_STRING)
			return CValue((wcscmp(s_value.c_str(), value.s_value.c_str()) < 0) ? 1 : 0);
		else
			return CValue(0);
	}

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  operator || (CValue)
 *
 *  �U�͐���0�݂̂ŁA����ȊO�͂��ׂĐ^�ł��B
 * -----------------------------------------------------------------------
 */
CValue CValue::operator ||(const CValue &value) const
{
	return CValue((GetTruth() || value.GetTruth()) ? 1 : 0);
}

/* -----------------------------------------------------------------------
 *  operator && (CValue)
 *
 *  �U�͐���0�݂̂ŁA����ȊO�͂��ׂĐ^�ł��B
 * -----------------------------------------------------------------------
 */
CValue CValue::operator && (const CValue &value) const
{
	return CValue((GetTruth() && value.GetTruth()) ? 1 : 0);
}
