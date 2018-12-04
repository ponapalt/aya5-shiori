// 
// AYA version 5
//
// システム関数を実行するクラス　CSystemFunction
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#else
#  include "posix_utils.h"
#  include "messages.h"
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <sys/time.h>
#  include <iostream>
#  include <fstream>
#  include <unistd.h>
#  include <stdio.h>
#  include <dirent.h>
#  include <boost/scoped_array.hpp>
using namespace std;
using namespace boost;
#endif
#include "basis.h"
#include "parser0.h"
#include "function.h"
#include "sysfunc.h"
#include "value.h"
#include "misc.h"
#include "log.h"
#include "wsex.h"
#include "lib.h"
#include "file.h"
#include "ccct.h"
#include "mt19937ar.h"

extern CBasis				basis;
extern vector<CFunction>	function;
extern CSystemFunction		sysfunction;
extern CGlobalVariable		variable;
extern CFile				files;
extern CLib					libs;
extern CLog					logger;

#ifndef POSIX
#  define	swprintf	_snwprintf
#endif

#ifndef POSIX
#  define for if(0);else for
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::Execute
 *  機能概要：  システム関数を実行します
 *
 *  thisfuncはこの関数を実行したCFunctionインスタンスです
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::Execute(int index, CValue &arg, vector<CCell *> &pcellarg,
			CLocalVariable &lvar, int l, CFunction *thisfunc)
{
	wstring& d = thisfunc->dicfilename;

	switch(index) {
	case 0:		// TOINT
		return TOINT(arg.array, d, l);
	case 1:		// TOREAL
		return TOREAL(arg.array, d, l);
	case 2:		// TOSTR
		return TOSTR(arg, d, l);
	case 3:		// GETTYPE
		return GETTYPE(arg.array, d, l);
	case 4:		// ISFUNC
		return ISFUNC(arg.array, d, l);
	case 5:		// ISVAR
		return ISVAR(arg.array, lvar, d, l);
	case 6:		// LOGGING
		return LOGGING(arg, d, l);
	case 7:		// GETLASTERROR
		return CValue(lasterror);
	case 8:		// LOADLIB
		return LOADLIB(arg.array, d, l);
	case 9:		// UNLOADLIB
		return UNLOADLIB(arg.array, d, l);
	case 10:	// REQUESTLIB
		return REQUESTLIB(arg.array, d, l);
	case 11:	// CHARSETLIB
		return CHARSETLIB(arg.array, d, l);
	case 12:	// RAND
		return RAND(arg.array, d, l);
	case 13:	// FLOOR
		return FLOOR(arg.array, d, l);
	case 14:	// CEIL
		return CEIL(arg.array, d, l);
	case 15:	// ROUND
		return ROUND(arg.array, d, l);
	case 16:	// SIN
		return SIN(arg.array, d, l);
	case 17:	// COS
		return COS(arg.array, d, l);
	case 18:	// TAN
		return TAN(arg.array, d, l);
	case 19:	// LOG
		return LOG(arg.array, d, l);
	case 20:	// LOG10
		return LOG10(arg.array, d, l);
	case 21:	// POW
		return POW(arg.array, d, l);
	case 22:	// SQRT
		return SQRT(arg.array, d, l);
	case 23:	// STRSTR
		return STRSTR(arg.array, d, l);
	case 24:	// STRLEN
		return STRLEN(arg.array, d, l);
	case 25:	// REPLACE
		return REPLACE(arg.array, d, l);
	case 26:	// SUBSTR
		return SUBSTR(arg.array, d, l);
	case 27:	// ERASE
		return ERASE(arg.array, d, l);
	case 28:	// INSERT
		return INSERT(arg.array, d, l);
	case 29:	// TOUPPER
		return TOUPPER(arg.array, d, l);
	case 30:	// TOLOWER
		return TOLOWER(arg.array, d, l);
	case 31:	// CUTSPACE
		return CUTSPACE(arg.array, d, l);
	case 32:	// TOBINSTR
		return TOBINSTR(arg.array, d, l);
	case 33:	// TOHEXSTR
		return TOHEXSTR(arg.array, d, l);
	case 34:	// BINSTRTOI
		return BINSTRTOI(arg.array, d, l);
	case 35:	// HEXSTRTOI
		return HEXSTRTOI(arg.array, d, l);
	case 36:	// CHR
		return CHR(arg.array, d, l);
	case 37:	// FOPEN
		return FOPEN(arg.array, d, l);
	case 38:	// FCLOSE
		return FCLOSE(arg.array, d, l);
	case 39:	// FREAD
		return FREAD(arg.array, d, l);
	case 40:	// FWRITE
		return FWRITE(arg.array, d, l);
	case 41:	// FWRITE2
		return FWRITE2(arg.array, d, l);
	case 42:	// FCOPY
		return FCOPY(arg.array, d, l);
	case 43:	// FMOVE
		return FMOVE(arg.array, d, l);
	case 44:	// MKDIR
		return MKDIR(arg.array, d, l);
	case 45:	// RMDIR
		return RMDIR(arg.array, d, l);
	case 46:	// FDEL
		return FDEL(arg.array, d, l);
	case 47:	// FRENAME
		return FRENAME(arg.array, d, l);
	case 48:	// FSIZE
		return FSIZE(arg.array, d, l);
	case 49:	// FENUM
		return FENUM(arg.array, d, l);
	case 50:	// FCHARSET
		return FCHARSET(arg.array, d, l);
	case 51:	// ARRAYSIZE
		return ARRAYSIZE(arg.array, pcellarg, lvar, d, l);
	case 52:	// SETDELIM
		return SETDELIM(pcellarg, lvar, d, l);
	case 53:	// EVAL
		return EVAL(arg.array, d, l, lvar, thisfunc);
	case 54:	// ERASEVAR
		return ERASEVAR(arg.array, lvar, d, l);
	case 55:	// GETTIME
		return GETTIME();
	case 56:	// GETTICKCOUNT
		return GETTICKCOUNT(arg.array, lvar, d, l);
	case 57:	// GETMEMINFO
		return GETMEMINFO();
	case 58:	// RE_SEARCH
		return RE_SEARCH(arg.array, d, l);
	case 59:	// RE_MATCH
		return RE_MATCH(arg.array, d, l);
	case 60:	// RE_GREP
		return RE_GREP(arg.array, d, l);
	case 61:	// %[n]（置換済の値の再利用）処理用関数 → これのみCFunctionで処理するのでここへは来ない
		logger.Error(E_E, 49, d, l);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	case 62:	// SETLASTERROR
		return SETLASTERROR(arg.array, d, l);
	case 63:	// RE_REPLACE
		return RE_REPLACE(arg.array, d, l);
	case 64:	// RE_SPLIT
		return RE_SPLIT(arg.array, d, l);
	case 65:	// RE_GETSTR
		return re_str;
	case 66:	// RE_GETPOS
		return re_pos;
	case 67:	// RE_GETLEN
		return re_len;
	case 68:	// CHRCODE
		return CHRCODE(arg.array, d, l);
	case 69:	// ISINTSTR
		return ISINTSTR(arg.array, d, l);
	case 70:	// ISREALSTR
		return ISREALSTR(arg.array, d, l);
	case 71:	// IARRAY
		return CValue(F_TAG_ARRAY, 0/*dmy*/);
	case 72:	// SPLITPATH
		return SPLITPATH(arg.array, d, l);
	case 73:	// CVINT
		return CVINT(arg.array, pcellarg, lvar, d, l);
	case 74:	// CVSTR
		return CVSTR(arg.array, pcellarg, lvar, d, l);
	case 75:	// CVREAL
		return CVREAL(arg.array, pcellarg, lvar, d, l);
	case 76:	// LETTONAME
		return LETTONAME(arg.array, d, l, lvar, thisfunc);
	case 77:	// LSO
		return CValue(lso);
	case 78:	// STRFORM
		return STRFORM(arg.array, d, l);
	case 79:	// ANY
		return ANY(arg.array, pcellarg, lvar, d, l);
	case 80:	// SAVEVAR
		return SAVEVAR();
	case 81:	// GETSTRBYTES
		return GETSTRBYTES(arg.array, d, l);
	case 82:	// ASEARCH
		return ASEARCH(arg.array, d, l);
	case 83:	// ASEARCHEX
		return ASEARCHEX(arg.array, d, l);
	case 84:	// GETDELIM
		return GETDELIM(pcellarg, lvar, d, l);
	case 85:	// GETSETTING
		return GETSETTING(arg.array, d, l);
	case 86:	// ASIN
		return ASIN(arg.array, d, l);
	case 87:	// ACOS
		return ACOS(arg.array, d, l);
	case 88:	// ATAN
		return ATAN(arg.array, d, l);
	case 89:	// SPLIT
		return SPLIT(arg.array, d, l);
	case 90:	// FATTRIB
		return FATTRIB(arg.array, d, l);
//	case 82:	// RE_REPLACEEX
//		return RE_REPLACEEX(arg.array, d, l);
	default:
		logger.Error(E_E, 49, d, l);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	};

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOINT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOINT(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"TOINT", d, l);
		SetError(8);
		return CValue(0);
	}

	return CValue(arg[0].GetValueInt());
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOREAL
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOREAL(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"TOREAL", d, l);
		SetError(8);
		return CValue(0.0);
	}

	return CValue(arg[0].GetValueDouble());
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOSTR(CValue &args, wstring &d, int &l)
{
	int	sz = args.array.size();

	if (!sz) {
		logger.Error(E_W, 8, L"TOSTR", d, l);
		SetError(8);
		return CValue();
	}

	if (sz == 1)
		return CValue(args.array[0].GetValueString());

	return CValue(args.GetValueString());
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETTYPE
 *
 *  返値　　：  0/1/2/3/4=エラー/整数/実数/文字列/配列
 *
 *  GETTYPE(1,2)のように複数の引数を与えた場合、これらは1つの配列値ですから
 *  返値は4になることに注意してください。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETTYPE(vector<CValueSub> &arg, wstring &d, int &l)
{
	int	sz = arg.size();

	if (!sz) {
		logger.Error(E_W, 8, L"GETTYPE", d, l);
		SetError(8);
		return CValue(0);
	}

	if (sz > 1)
		return CValue(4);

	switch(arg[0].GetType()) {
	case F_TAG_INT:
		return CValue(1);
	case F_TAG_DOUBLE:
		return CValue(2);
	case F_TAG_STRING:
		return CValue(3);
	default:
		logger.Error(E_E, 88, L"GETTYPE", d, l);
		return CValue(0);
	};
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ISFUNC
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ISFUNC(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"ISFUNC", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"ISFUNC", d, l);
		SetError(9);
		return CValue(0);
	}

	int	i = GetFunctionIndexFromName(arg[0].s_value);
	if(i != -1)
		return CValue(1);

/*	for(vector<CFunction>::iterator it = function.begin(); it != function.end(); it++)
		if (!arg[0].s_value.compare(it->name))
			return CValue(1);
*/

	for(int i = 0; i < SYSFUNC_NUM; i++)
		if (!arg[0].s_value.compare(sysfunc[i]))
			return CValue(2);

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ISVAR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ISVAR(vector<CValueSub> &arg, CLocalVariable &lvar, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"ISVAR", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"ISVAR", d, l);
		SetError(9);
		return CValue(0);
	}

	int	index = variable.GetIndex(arg[0].s_value);
	if (index >= 0)
		return CValue(1);

	int	depth = -1;
	lvar.GetIndex(arg[0].s_value, index, depth);
	if (index >= 0)
		return CValue(2);

	return CValue(0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LOGGING
 *
 *  文字列の場合はダブルクォートします。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LOGGING(CValue &arg, wstring &d, int &l)
{
	if (arg.array.size())
		logger.Write(arg.GetValueStringForLogging());

	logger.Write(L"\n");

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LOADLIB
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LOADLIB(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"LOADLIB", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"LOADLIB", d, l);
		SetError(9);
		return CValue(0);
	}

	int	excode = libs.Add(ToFullPath(arg[0].s_value));
	if (!excode) {
		logger.Error(E_W, 13, L"LOADLIB", d, l);
		SetError(13);
	}

	return CValue(excode);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::UNLOADLIB
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::UNLOADLIB(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"UNLOADLIB", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"UNLOADLIB", d, l);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	int	result = libs.Delete(ToFullPath(arg[0].s_value));

	if (!result) {
		logger.Error(E_W, 13, L"UNLOADLIB", d, l);
		SetError(13);
	}
	else if (result == 2) {
		logger.Error(E_W, 14, d, l);
		SetError(14);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::REQUESTLIB
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::REQUESTLIB(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (arg.size() < 2) {
		logger.Error(E_W, 8, L"REQUESTLIB", d, l);
		SetError(8);
		return CValue();
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"REQUESTLIB", d, l);
		SetError(9);
		return CValue();
	}

	wstring	result;
	if (!libs.Request(ToFullPath(arg[0].s_value), arg[1].s_value, result)) {
		logger.Error(E_W, 13, L"REQUESTLIB", d, l);
		SetError(13);
	}

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CHARSETLIB
 *
 *  有効な値は、0/1/127=Shift_JIS/UTF-8/OSデフォルト　です。
 *  これ以外の値を与えた場合は無効で、warningとなります。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CHARSETLIB(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"CHARSETLIB", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (arg[0].GetType() != F_TAG_INT) {
		logger.Error(E_W, 9, L"CHARSETLIB", d, l);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	int	charset = arg[0].i_value;
	if (charset != CHARSET_SJIS &&
		charset != CHARSET_UTF8 &&
		charset != CHARSET_DEFAULT) {
		logger.Error(E_W, 12, L"CHARSETLIB", d, l);
		SetError(12);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	libs.SetCharset(charset);

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RAND
 *
 *  引数なしはRAND(100)扱いです。
 *  引数エラーでは0を返します。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RAND(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size())
	        return (int)(genrand_real2()*100.0);

	if (arg[0].GetType() != F_TAG_INT) {
		logger.Error(E_W, 9, L"RAND", d, l);
		SetError(9);
		return CValue(0);
	}

	return (int)(genrand_real2()*(double)arg[0].i_value);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FLOOR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FLOOR(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"FLOOR", d, l);
		SetError(8);
		return CValue(0.0);
	}

	if (arg[0].GetType() == F_TAG_INT ||
		arg[0].GetType() == F_TAG_DOUBLE)
		return CValue(floor(arg[0].GetValueDouble()));

	logger.Error(E_W, 9, L"FLOOR", d, l);
	SetError(9);
	return CValue(0.0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CEIL
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CEIL(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"CEIL", d, l);
		SetError(8);
		return CValue(0.0);
	}

	if (arg[0].GetType() == F_TAG_INT ||
		arg[0].GetType() == F_TAG_DOUBLE)
		return CValue(ceil(arg[0].GetValueDouble()));

	logger.Error(E_W, 9, L"CEIL", d, l);
	SetError(9);
	return CValue(0.0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ROUND
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ROUND(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"ROUND", d, l);
		SetError(8);
		return CValue(0.0);
	}

	if (arg[0].GetType() == F_TAG_INT)
		return CValue((double)arg[0].i_value);
	else if (arg[0].GetType() == F_TAG_DOUBLE) {
		double	value = arg[0].d_value;
		if (value - floor(value) >= 0.5)
			return CValue(ceil(value));
		else
			return CValue(floor(value));
	}

	logger.Error(E_W, 9, L"ROUND", d, l);
	SetError(9);
	return CValue(0.0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SIN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SIN(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"SIN", d, l);
		SetError(8);
		return CValue(0.0);
	}

	if (arg[0].GetType() == F_TAG_INT ||
		arg[0].GetType() == F_TAG_DOUBLE)
		return CValue(sin(arg[0].GetValueDouble()));

	logger.Error(E_W, 9, L"SIN", d, l);
	SetError(9);
	return CValue(0.0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::COS
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::COS(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"COS", d, l);
		SetError(8);
		return CValue(0.0);
	}

	if (arg[0].GetType() == F_TAG_INT ||
		arg[0].GetType() == F_TAG_DOUBLE)
		return CValue(cos(arg[0].GetValueDouble()));

	logger.Error(E_W, 9, L"COS", d, l);
	SetError(9);
	return CValue(0.0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TAN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TAN(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"TAN", d, l);
		SetError(8);
		return CValue(0.0);
	}

	if (arg[0].GetType() == F_TAG_INT ||
		arg[0].GetType() == F_TAG_DOUBLE)
		return CValue(tan(arg[0].GetValueDouble()));

	logger.Error(E_W, 9, L"TAN", d, l);
	SetError(9);
	return CValue(0.0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ASIN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ASIN(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"ASIN", d, l);
		SetError(8);
		return CValue(0.0);
	}

	if (arg[0].GetType() == F_TAG_INT ||
		arg[0].GetType() == F_TAG_DOUBLE)
		return CValue(asin(arg[0].GetValueDouble()));

	logger.Error(E_W, 9, L"ASIN", d, l);
	SetError(9);
	return CValue(0.0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ACOS
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ACOS(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"ACOS", d, l);
		SetError(8);
		return CValue(0.0);
	}

	if (arg[0].GetType() == F_TAG_INT ||
		arg[0].GetType() == F_TAG_DOUBLE)
		return CValue(acos(arg[0].GetValueDouble()));

	logger.Error(E_W, 9, L"ACOS", d, l);
	SetError(9);
	return CValue(0.0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ATAN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ATAN(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"ATAN", d, l);
		SetError(8);
		return CValue(0.0);
	}

	if (arg[0].GetType() == F_TAG_INT ||
		arg[0].GetType() == F_TAG_DOUBLE)
		return CValue(atan(arg[0].GetValueDouble()));

	logger.Error(E_W, 9, L"ATAN", d, l);
	SetError(9);
	return CValue(0.0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LOG
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LOG(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"LOG", d, l);
		SetError(8);
		return CValue(0.0);
	}

	if (arg[0].GetType() == F_TAG_INT ||
		arg[0].GetType() == F_TAG_DOUBLE)
		return CValue(log(arg[0].GetValueDouble()));

	logger.Error(E_W, 9, L"LOG", d, l);
	SetError(9);
	return CValue(0.0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LOG10
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LOG10(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"LOG10", d, l);
		SetError(8);
		return CValue(0.0);
	}

	if (arg[0].GetType() == F_TAG_INT ||
		arg[0].GetType() == F_TAG_DOUBLE)
		return CValue(log10(arg[0].GetValueDouble()));

	logger.Error(E_W, 9, L"LOG10", d, l);
	SetError(9);
	return CValue(0.0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::POW
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::POW(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (arg.size() < 2) {
		logger.Error(E_W, 8, L"POW", d, l);
		SetError(8);
		return CValue(0.0);
	}

	if ((arg[0].GetType() == F_TAG_INT ||
		arg[0].GetType() == F_TAG_DOUBLE) &&
		(arg[1].GetType() == F_TAG_INT ||
		arg[1].GetType() == F_TAG_DOUBLE))
		return CValue(pow(arg[0].GetValueDouble(), arg[1].GetValueDouble()));

	logger.Error(E_W, 9, L"POW", d, l);
	SetError(9);
	return CValue(0.0);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SQRT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SQRT(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"SQRT", d, l);
		SetError(8);
		return CValue(0.0);
	}

	if (arg[0].GetType() != F_TAG_INT &&
		arg[0].GetType() != F_TAG_DOUBLE) {
		logger.Error(E_W, 9, L"SQRT", d, l);
		SetError(9);
		return CValue(0.0);
	}

	double	value = arg[0].GetValueDouble();

	if (value < 0.0)
		return CValue(-1);

	return CValue(sqrt(arg[0].GetValueDouble()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::STRSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::STRSTR(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (arg.size() < 3) {
		logger.Error(E_W, 8, L"STRSTR", d, l);
		SetError(8);
		return CValue(-1);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING ||
		(arg[2].GetType() != F_TAG_INT &&
		arg[2].GetType() != F_TAG_DOUBLE)) {
		logger.Error(E_W, 9, L"STRSTR", d, l);
		SetError(9);
		return CValue(-1);
	}

	return CValue((int)arg[0].s_value.find(arg[1].s_value, arg[2].GetValueInt()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::STRLEN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::STRLEN(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"STRLEN", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"STRLEN", d, l);
		SetError(9);
		return CValue(0);
	}

	return CValue((int)arg[0].s_value.size());
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::REPLACE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::REPLACE(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (arg.size() < 3) {
		logger.Error(E_W, 8, L"REPLACE", d, l);
		SetError(8);
		return CValue();
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING ||
		arg[2].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"REPLACE", d, l);
		SetError(9);
		return CValue(arg[0]);
	}

	wstring	result  = arg[0].s_value;
	wstring	*before = &(arg[1].s_value);
	wstring	*after  = &(arg[2].s_value);
	int	sz_before = before->size();
	int	sz_after  = after->size();

	if (sz_before)
		ws_replace(result, *before, *after);

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SUBSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SUBSTR(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (arg.size() < 3) {
		logger.Error(E_W, 8, L"SUBSTR", d, l);
		SetError(8);
		return CValue();
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		(arg[1].GetType() != F_TAG_INT &&
		arg[1].GetType() != F_TAG_DOUBLE) ||
		(arg[2].GetType() != F_TAG_INT &&
		arg[2].GetType() != F_TAG_DOUBLE)) {
		logger.Error(E_W, 9, L"SUBSTR", d, l);
		SetError(9);
		return CValue();
	}

	wstring& src = arg[0].s_value;
	int pos = arg[1].GetValueInt();
	int len = arg[2].GetValueInt();

	if (pos >= src.length()) {
	    return CValue(L"");
	}
	if (pos + len >= src.length()) {
	    len = src.length() - pos;
	}

	return CValue(src.substr(pos, len));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ERASE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ERASE(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (arg.size() < 3) {
		logger.Error(E_W, 8, L"ERASE", d, l);
		SetError(8);
		return CValue();
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		(arg[1].GetType() != F_TAG_INT &&
		arg[1].GetType() != F_TAG_DOUBLE) ||
		(arg[2].GetType() != F_TAG_INT &&
		arg[2].GetType() != F_TAG_DOUBLE)) {
		logger.Error(E_W, 9, L"ERASE", d, l);
		SetError(9);
		return CValue(arg[0]);
	}

	return CValue(arg[0].s_value.erase(arg[1].GetValueInt(), arg[2].GetValueInt()));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::INSERT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::INSERT(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (arg.size() < 3) {
		logger.Error(E_W, 8, L"INSERT", d, l);
		SetError(8);
		return CValue();
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		(arg[1].GetType() != F_TAG_INT &&
		arg[1].GetType() != F_TAG_DOUBLE) ||
		arg[2].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"ERASE", d, l);
		SetError(9);
		return CValue(arg[0]);
	}

	return CValue(arg[0].s_value.insert(arg[1].GetValueInt(), arg[2].s_value));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOUPPER
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOUPPER(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"TOUPPER", d, l);
		SetError(8);
		return CValue();
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"TOUPPER", d, l);
		SetError(9);
		return CValue(arg[0]);
	}

	wstring	result = arg[0].s_value;
	int	len = result.size();
	for(int i = 0; i < len; i++)
		if (result[i] >= L'a' && result[i] <= 'z')
			result[i] += (L'A' - L'a');

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOLOWER
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOLOWER(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"TOLOWER", d, l);
		SetError(8);
		return CValue();
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"TOLOWER", d, l);
		SetError(9);
		return CValue(arg[0]);
	}

	wstring	result = arg[0].s_value;
	int	len = result.size();
	for(int i = 0; i < len; i++)
		if (result[i] >= L'A' && result[i] <= 'Z')
			result[i] += (L'a' - L'A');

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CUTSPACE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CUTSPACE(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"CUTSPACE", d, l);
		SetError(8);
		return CValue();
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"CUTSPACE", d, l);
		SetError(9);
		return CValue(arg[0]);
	}

	wstring	result = arg[0].s_value;
	CutSpace(result);

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOBINSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOBINSTR(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"TOBINSTR", d, l);
		SetError(8);
		return CValue();
	}

	if (arg[0].GetType() != F_TAG_INT) {
		logger.Error(E_W, 9, L"TOBINSTR", d, l);
		SetError(9);
		return CValue();
	}

	wstring	result;
	ws_itoa(result, arg[0].i_value, 2);

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::TOHEXSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::TOHEXSTR(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"TOHEXSTR", d, l);
		SetError(8);
		return CValue();
	}

	if (arg[0].GetType() != F_TAG_INT) {
		logger.Error(E_W, 9, L"TOHEXSTR", d, l);
		SetError(9);
		return CValue();
	}

	wstring	result;
	ws_itoa(result, arg[0].i_value, 16);

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::BINSTRTOI
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::BINSTRTOI(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"BINSTRTOI", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"BINSTRTOI", d, l);
		SetError(9);
		return CValue(0);
	}

	if (!IsIntBinString(arg[0].s_value, 0)) {
		logger.Error(E_W, 12, L"BINSTRTOI", d, l);
		SetError(12);
		return CValue(0);
	}

	return CValue(ws_atoi(arg[0].s_value, 2));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::HEXSTRTOI
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::HEXSTRTOI(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"HEXSTRTOI", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"HEXSTRTOI", d, l);
		SetError(9);
		return CValue(0);
	}

	if (!IsIntHexString(arg[0].s_value, 0)) {
		logger.Error(E_W, 12, L"HEXSTRTOI", d, l);
		SetError(12);
		return CValue(0);
	}

	return CValue(ws_atoi(arg[0].s_value, 16));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CHR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CHR(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"CHR", d, l);
		SetError(8);
		return CValue();
	}

	if (arg[0].GetType() != F_TAG_INT) {
		logger.Error(E_W, 9, L"CHR", d, l);
		SetError(9);
		return CValue();
	}

	return CValue(wstring(1, (wchar_t)arg[0].i_value));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FOPEN
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FOPEN(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (arg.size() < 2) {
		logger.Error(E_W, 8, L"FOPEN", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"FOPEN", d, l);
		SetError(9);
		return CValue(0);
	}

	return CValue(files.Add(ToFullPath(arg[0].s_value), arg[1].s_value));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FCLOSE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FCLOSE(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"FCLOSE", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"FCLOSE", d, l);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	int	result = files.Delete(ToFullPath(arg[0].s_value));

	if (!result) {
		logger.Error(E_W, 13, L"FCLOSE", d, l);
		SetError(13);
	}
	else if (result == 2) {
		logger.Error(E_W, 15, d, l);
		SetError(15);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FREAD
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FREAD(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"FREAD", d, l);
		SetError(8);
		return CValue();
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"FREAD", d, l);
		SetError(9);
		return CValue();
	}

	wstring	r_value;
	int	result = files.Read(ToFullPath(arg[0].s_value), r_value);
	CutCrLf(r_value);

	if (!result) {
		logger.Error(E_W, 13, L"FREAD", d, l);
		SetError(13);
	}
	else if (result == -1)
		return CValue(-1);

	return CValue(r_value);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FWRITE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FWRITE(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (arg.size() < 2) {
		logger.Error(E_W, 8, L"FWRITE", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"FWRITE", d, l);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!files.Write(ToFullPath(arg[0].s_value), arg[1].s_value + wstring(L"\n"))) {
		logger.Error(E_W, 13, L"FWRITE", d, l);
		SetError(13);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FWRITE2
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FWRITE2(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (arg.size() < 2) {
		logger.Error(E_W, 8, L"FWRITE", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"FWRITE", d, l);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!files.Write(ToFullPath(arg[0].s_value), arg[1].s_value)) {
		logger.Error(E_W, 13, L"FWRITE", d, l);
		SetError(13);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FCOPY
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
CValue	CSystemFunction::FCOPY(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (arg.size() < 2) {
		logger.Error(E_W, 8, L"FCOPY", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"FCOPY", d, l);
		SetError(9);
		return CValue(0);
	}

	// 絶対パス化
	wchar_t	drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_wsplitpath(arg[0].s_value.c_str(), drive, dir, fname, ext);
	wstring	s_path = ((::wcslen(drive)) ? wstring(L"") : basis.path) + arg[0].s_value;

	wchar_t	fname2[_MAX_FNAME], ext2[_MAX_EXT];
	_wsplitpath(arg[1].s_value.c_str(), drive, dir, fname2, ext2);
	wstring	d_path = ((::wcslen(drive)) ?
						wstring(L"") : basis.path) + arg[1].s_value + L"\\" + fname + ext;

	// パスをMBCSに変換
	char	*s_pstr = Ccct::Ucs2ToMbcs(s_path, CHARSET_DEFAULT);
	if (s_pstr == NULL) {
		logger.Error(E_E, 89, L"FCOPY", d, l);
		return CValue(0);
	}
	char	*d_pstr = Ccct::Ucs2ToMbcs(d_path, CHARSET_DEFAULT);
	if (d_pstr == NULL) {
		free(s_pstr);
		logger.Error(E_E, 89, L"FCOPY", d, l);
		return CValue(0);
	}

	// 実行
	int	result = (CopyFile(s_pstr, d_pstr, FALSE) ? 1 : 0);
	free(s_pstr);
	free(d_pstr);

	return CValue(result);
}
#else
CValue CSystemFunction::FCOPY(vector<CValueSub> &arg, wstring &d, int &l) {
    if (arg.size() < 2) {
	logger.Error(E_W, 8, L"FCOPY", d, l);
	SetError(8);
	return CValue(0);
    }
    
    if (arg[0].GetType() != F_TAG_STRING ||
	arg[1].GetType() != F_TAG_STRING) {
	logger.Error(E_W, 9, L"FCOPY", d, l);
	SetError(9);
	return CValue(0);
    }

    // 絶対パス化
    string src = narrow(ToFullPath(arg[0].s_value));
    string dest = narrow(ToFullPath(arg[1].s_value));
    fix_filepath(src);
    fix_filepath(dest);

    // srcは通常ファイルでなければならない。
    // FCOPY("/dev/zero", "/tmp/hoge") とかやられたら嫌過ぎ。
    struct stat sb;
    if (stat(src.c_str(), &sb) != 0) {
	return CValue(0);
    }
    if ((sb.st_mode & S_IFREG) == 0) {
	return CValue(0);
    }

    // 実行
    unlink(dest.c_str()); // コピー先がシンボリックリンクとかだと嫌。
    ifstream is(src.c_str());
    int result = 0;
    if (is.good()) {
	ofstream os(dest.c_str());
	if (os.good()) {
	    scoped_array<char> buf(new char[512]);
	    while (is.good()) {
		is.read(buf.get(), 512);
		int len = is.gcount();
		if (len == 0) {
		    break;
		}
		os.write(buf.get(), len);
	    }
	    result = 1;
	}
    }
    
    return CValue(result);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FMOVE
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
CValue	CSystemFunction::FMOVE(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (arg.size() < 2) {
		logger.Error(E_W, 8, L"FMOVE", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"FMOVE", d, l);
		SetError(9);
		return CValue(0);
	}

	// 絶対パス化
	wchar_t	drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_wsplitpath(arg[0].s_value.c_str(), drive, dir, fname, ext);
	wstring	s_path = ((::wcslen(drive)) ? wstring(L"") : basis.path) + arg[0].s_value;

	wchar_t	fname2[_MAX_FNAME], ext2[_MAX_EXT];
	_wsplitpath(arg[1].s_value.c_str(), drive, dir, fname2, ext2);
	wstring	d_path = ((::wcslen(drive)) ?
						wstring(L"") : basis.path) + arg[1].s_value + L"\\" + fname + ext;

	// パスをMBCSに変換
	char	*s_pstr = Ccct::Ucs2ToMbcs(s_path, CHARSET_DEFAULT);
	if (s_pstr == NULL) {
		logger.Error(E_E, 89, L"FMOVE", d, l);
		return CValue(0);
	}
	char	*d_pstr = Ccct::Ucs2ToMbcs(d_path, CHARSET_DEFAULT);
	if (d_pstr == NULL) {
		free(s_pstr);
		logger.Error(E_E, 89, L"FMOVE", d, l);
		return CValue(0);
	}

	// 実行
	int	result = (MoveFile(s_pstr, d_pstr) ? 1 : 0);
	free(s_pstr);
	free(d_pstr);

	return CValue(result);
}
#else
CValue CSystemFunction::FMOVE(vector<CValueSub> &arg, wstring &d, int &l) {
    if (arg.size() < 2) {
	logger.Error(E_W, 8, L"FMOVE", d, l);
	SetError(8);
	return CValue(0);
    }
    
    if (arg[0].GetType() != F_TAG_STRING ||
	arg[1].GetType() != F_TAG_STRING) {
	logger.Error(E_W, 9, L"FMOVE", d, l);
	SetError(9);
	return CValue(0);
    }

    // 絶対パス化
    string src = narrow(ToFullPath(arg[0].s_value));
    string dest = narrow(ToFullPath(arg[1].s_value));
    fix_filepath(src);
    fix_filepath(dest);
    
    // 実行
    int result = rename(src.c_str(), dest.c_str()) ? 0 : 1;

    return CValue(result);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::MKDIR
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
CValue	CSystemFunction::MKDIR(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"MKDIR", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"MKDIR", d, l);
		SetError(9);
		return CValue(0);
	}

	// パスをMBCSに変換
	char	*s_dirstr = Ccct::Ucs2ToMbcs(ToFullPath(arg[0].s_value), CHARSET_DEFAULT);
	if (s_dirstr == NULL) {
		logger.Error(E_E, 89, L"MKDIR", d, l);
		return CValue(0);
	}

	// 実行
	int	result = (_mkdir(s_dirstr) ? 0 : 1);
	free(s_dirstr);

	return CValue(result);
}
#else
CValue CSystemFunction::MKDIR(vector<CValueSub> &arg, wstring &d, int &l) {
    if (!arg.size()) {
	logger.Error(E_W, 8, L"MKDIR", d, l);
	SetError(8);
	return CValue(0);
    }
    
    if (arg[0].GetType() != F_TAG_STRING) {
	logger.Error(E_W, 9, L"MKDIR", d, l);
	SetError(9);
	return CValue(0);
    }

    string dirstr = narrow(ToFullPath(arg[0].s_value));
    fix_filepath(dirstr);

    // 実行
    int result = (mkdir(dirstr.c_str(), 0644) ? 0 : 1);

    return CValue(result);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RMDIR
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
CValue	CSystemFunction::RMDIR(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"RMDIR", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"RMDIR", d, l);
		SetError(9);
		return CValue(0);
	}

	// パスをMBCSに変換
	char	*s_dirstr = Ccct::Ucs2ToMbcs(ToFullPath(arg[0].s_value), CHARSET_DEFAULT);
	if (s_dirstr == NULL) {
		logger.Error(E_E, 89, L"RMDIR", d, l);
		return CValue(0);
	}

	// 実行
	int	result = (_rmdir(s_dirstr) ? 0 : 1);
	free(s_dirstr);

	return CValue(result);
}
#else
CValue CSystemFunction::RMDIR(vector<CValueSub> &arg, wstring &d, int &l) {
    if (!arg.size()) {
	logger.Error(E_W, 8, L"RMDIR", d, l);
	SetError(8);
	return CValue(0);
    }
    
    if (arg[0].GetType() != F_TAG_STRING) {
	logger.Error(E_W, 9, L"RMDIR", d, l);
	SetError(9);
	return CValue(0);
    }

    string dirstr = narrow(ToFullPath(arg[0].s_value));
    fix_filepath(dirstr);

    // 実行。
    int result = (rmdir(dirstr.c_str()) ? 0 : 1);

    return CValue(result);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FDEL
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
CValue	CSystemFunction::FDEL(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"FDEL", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"FDEL", d, l);
		SetError(9);
		return CValue(0);
	}

	// パスをMBCSに変換
	char	*s_filestr = Ccct::Ucs2ToMbcs(ToFullPath(arg[0].s_value), CHARSET_DEFAULT);
	if (s_filestr == NULL) {
		logger.Error(E_E, 89, L"FDEL", d, l);
		return CValue(0);
	}

	// 実行
	int	result = (DeleteFile(s_filestr) ? 1 : 0);
	free(s_filestr);

	return CValue(result);
}
#else
CValue CSystemFunction::FDEL(vector<CValueSub> &arg, wstring &d, int &l) {
    if (!arg.size()) {
	logger.Error(E_W, 8, L"FDEL", d, l);
	SetError(8);
	return CValue(0);
    }
    
    if (arg[0].GetType() != F_TAG_STRING) {
	logger.Error(E_W, 9, L"FDEL", d, l);
	SetError(9);
	return CValue(0);
    }

    string filestr = narrow(ToFullPath(arg[0].s_value));
    fix_filepath(filestr);

    // 実行
    int result = (unlink(filestr.c_str()) ? 0 : 1);

    return CValue(result);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FRENAME
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
CValue	CSystemFunction::FRENAME(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (arg.size() < 2) {
		logger.Error(E_W, 8, L"FRENAME", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"FRENAME", d, l);
		SetError(9);
		return CValue(0);
	}

	// パスをMBCSに変換
	char	*s_filestr = Ccct::Ucs2ToMbcs(ToFullPath(arg[0].s_value), CHARSET_DEFAULT);
	if (s_filestr == NULL) {
		logger.Error(E_E, 89, L"FRENAME", d, l);
		return CValue(0);
	}
	char	*d_filestr = Ccct::Ucs2ToMbcs(ToFullPath(arg[1].s_value), CHARSET_DEFAULT);
	if (d_filestr == NULL) {
		free(s_filestr);
		logger.Error(E_E, 89, L"FRENAME", d, l);
		return CValue(0);
	}

	// 実行
	int	result = (MoveFile(s_filestr, d_filestr) ? 1 : 0);
	free(s_filestr);
	free(d_filestr);

	return CValue(result);
}
#else
CValue CSystemFunction::FRENAME(vector<CValueSub> &arg, wstring &d, int &l) {
    if (arg.size() < 2) {
	logger.Error(E_W, 8, L"FRENAME", d, l);
	SetError(8);
	return CValue(0);
    }
    
    if (arg[0].GetType() != F_TAG_STRING ||
	arg[1].GetType() != F_TAG_STRING) {
	logger.Error(E_W, 9, L"FRENAME", d, l);
	SetError(9);
	return CValue(0);
    }

    // 絶対パス化
    string src = narrow(ToFullPath(arg[0].s_value));
    string dest = narrow(ToFullPath(arg[1].s_value));
    fix_filepath(src);
    fix_filepath(dest);
    
    // 実行
    int result = rename(src.c_str(), dest.c_str()) ? 0 : 1;

    return CValue(result);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FSIZE
 *
 *  4GB以上のサイズは取得できません
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
CValue	CSystemFunction::FSIZE(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"FSIZE", d, l);
		SetError(8);
		return CValue(-1);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"FSIZE", d, l);
		SetError(9);
		return CValue(-1);
	}

	// パスをMBCSに変換
	char	*s_filestr = Ccct::Ucs2ToMbcs(ToFullPath(arg[0].s_value), CHARSET_DEFAULT);
	if (s_filestr == NULL) {
		logger.Error(E_E, 89, L"FSIZE", d, l);
		return CValue(-1);
	}

	// 実行
	HANDLE	hFile = CreateFile(s_filestr, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	free(s_filestr);
	if (hFile == INVALID_HANDLE_VALUE)
		return CValue(-1);
	unsigned long	result = GetFileSize(hFile, NULL);
	CloseHandle(hFile);

	if (result == 0xFFFFFFFF)
		return CValue(-1);

	return CValue((int)result);
}
#else
CValue CSystemFunction::FSIZE(vector<CValueSub> &arg, wstring &d, int &l) {
    if (!arg.size()) {
	logger.Error(E_W, 8, L"FSIZE", d, l);
	SetError(8);
	return CValue(-1);
    }
    
    if (arg[0].GetType() != F_TAG_STRING) {
	logger.Error(E_W, 9, L"FSIZE", d, l);
	SetError(9);
	return CValue(-1);
    }

    string path = narrow(ToFullPath(arg[0].s_value));
    fix_filepath(path);

    struct stat sb;
    if (stat(path.c_str(), &sb) != 0) {
	return CValue(-1);
    }
    return CValue(static_cast<int>(sb.st_size));
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FENUM
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
CValue	CSystemFunction::FENUM(vector<CValueSub> &arg, wstring &d, int &l)
{
	int	sz = arg.size();

	if (!sz) {
		logger.Error(E_W, 8, L"FENUM", d, l);
		SetError(8);
		return CValue();
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"FENUM", d, l);
		SetError(9);
		return CValue();
	}

	// デリミタ取得
	wstring	delimiter = VAR_DELIMITER;
	if (sz >= 2) {
		if (arg[1].GetType() == F_TAG_STRING &&
			arg[1].s_value.size())
			delimiter = arg[1].s_value;
		else {
			logger.Error(E_W, 9, L"FENUM", d, l);
			SetError(9);
			return CValue();
		}
	}

	// パスをMBCSに変換
	char	*s_filestr = Ccct::Ucs2ToMbcs(ToFullPath(arg[0].s_value + L"\\*.*"), CHARSET_DEFAULT);
	if (s_filestr == NULL) {
		logger.Error(E_E, 89, L"FENUM", d, l);
		return CValue();
	}

	// 実行
	CValue			result;
	HANDLE			hFile;
	WIN32_FIND_DATA	w32FindData;
	hFile = FindFirstFile(s_filestr, &w32FindData);
	free(s_filestr);
	if(hFile != INVALID_HANDLE_VALUE) {
		int i = 0;
		do {
			// 1つ取得
			string	t_file =w32FindData.cFileName;
			// もし"."or".."なら飛ばす
			if (!t_file.compare(".") || !t_file.compare(".."))
				continue;
			// UCS2へ変換
			wchar_t	*t_wfile = Ccct::MbcsToUcs2(t_file, CHARSET_DEFAULT);
			if (t_wfile == NULL)
				continue;
			// 追加
			if (i)
				result.s_value += delimiter;
			if (w32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				result.s_value +=  L"\\";
			result.s_value +=  t_wfile;
			free(t_wfile);
			i++;
		}
		while(FindNextFile(hFile, &w32FindData));

		FindClose(hFile);
	}

	return CValue(result);
}
#else
CValue CSystemFunction::FENUM(vector<CValueSub> &arg, wstring &d, int &l) {
    int	sz = arg.size();
    
    if (!sz) {
	logger.Error(E_W, 8, L"FENUM", d, l);
	SetError(8);
	return CValue();
    }
    
    if (arg[0].GetType() != F_TAG_STRING) {
	logger.Error(E_W, 9, L"FENUM", d, l);
	SetError(9);
	return CValue();
    }
    
    // デリミタ取得
    wstring delimiter = VAR_DELIMITER;
    if (sz >= 2) {
	if (arg[1].GetType() == F_TAG_STRING &&
	    arg[1].s_value.size()) {
	    delimiter = arg[1].s_value;
	}
	else {
	    logger.Error(E_W, 9, L"FENUM", d, l);
	    SetError(9);
	    return CValue();
	}
    }

    string path = narrow(ToFullPath(arg[0].s_value));
    fix_filepath(path);

    // 実行
    CValue result;
    DIR* dh = opendir(path.c_str());
    if (dh == NULL) {
	return result;
    }
    bool first_entry = true;
    while (true) {
	struct dirent* ent = readdir(dh);
	if (ent == NULL) {
	    break; // もう無い。
	}
	
	string name(ent->d_name, strlen(ent->d_name)/*ent->d_namlen*/);	// by umeici. 2005/1/16 5.6.0.232
	if (name == "." || name == "..") {
	    continue; // .と..は飛ばす
	}
	if (!first_entry) {
	    result.s_value += delimiter;
	}
	first_entry = false;

	struct stat sb;
	if (stat((path+'/'+name).c_str(), &sb) == 0) {
	    if (sb.st_mode & S_IFDIR) {
		name.insert(name.begin(), '\\');
	    }
	}
	result.s_value += widen(name);
    }
    closedir(dh);

    return CValue(result);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FCHARSET
 *
 *  有効な値は、0/1/127=Shift_JIS/UTF-8/OSデフォルト　です。
 *  これ以外の値を与えた場合は無効で、warningとなります。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::FCHARSET(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"FCHARSET", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (arg[0].GetType() != F_TAG_INT) {
		logger.Error(E_W, 9, L"FCHARSET", d, l);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	int	charset = arg[0].i_value;
	if (charset != CHARSET_SJIS &&
		charset != CHARSET_UTF8 &&
		charset != CHARSET_DEFAULT) {
		logger.Error(E_W, 12, L"FCHARSET", d, l);
		SetError(12);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	files.SetCharset(charset);

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ARRAYSIZE
 *
 *  文字列の場合は簡易配列の、汎用配列の場合はその要素数を返します。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ARRAYSIZE(vector<CValueSub> &arg, vector<CCell *> &pcellarg, CLocalVariable &lvar,
			wstring &d, int &l)
{
	// 引数無しなら0
	int	sz = arg.size();
	if (!sz)
		return CValue(0);

	if (sz == 1) {
		// 引数1つで文字列でないなら1
		if (arg[0].GetType() != F_TAG_STRING)
			return CValue(1);

		// 引数1つで文字列なら簡易配列の要素数を返す　変数の場合はそのデリミタで分割する
		wstring	delimiter = VAR_DELIMITER;
		if (pcellarg[0]->v.GetType() == F_TAG_VARIABLE)
			delimiter = variable.GetDelimiter(pcellarg[0]->index);
		else if (pcellarg[0]->v.GetType() == F_TAG_LOCALVARIABLE)
			delimiter = lvar.GetDelimiter(pcellarg[0]->name);

		vector<wstring>	s_array;
		return CValue((int)SplitToMultiString(arg[0].s_value, s_array, delimiter));
	}

	// 汎用配列なら要素数をそのまま返す
	return CValue((int)sz);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SETDELIM
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SETDELIM(vector<CCell *> &pcellarg, CLocalVariable &lvar, wstring &d, int &l)
{
	if (pcellarg.size() < 2) {
		logger.Error(E_W, 8, L"SETDELIM", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	CValue	*delimiter;
	if (pcellarg[1]->v.GetType() >= F_TAG_ORIGIN_VALUE &&
		pcellarg[1]->v.GetType() <= F_TAG_STRING)
		delimiter = &(pcellarg[1]->v);
	else
		delimiter = &(pcellarg[1]->ansv);

	if (delimiter->GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"SETDELIM", d, l);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!delimiter->s_value.size()) {
		logger.Error(E_W, 10, L"SETDELIM", d, l);
		SetError(10);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (pcellarg[0]->v.GetType() == F_TAG_VARIABLE)
		variable.SetDelimiter(pcellarg[0]->index, delimiter->s_value);
	else if (pcellarg[0]->v.GetType() == F_TAG_LOCALVARIABLE)
		lvar.SetDelimiter(pcellarg[0]->name, delimiter->s_value);
	else {
		logger.Error(E_W, 11, L"SETDELIM", d, l);
		SetError(11);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::EVAL
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::EVAL(vector<CValueSub> &arg, wstring &d, int &l, CLocalVariable &lvar,
			CFunction *thisfunc)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"EVAL", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"EVAL", d, l);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	// 数式へ展開
	wstring	str = arg[0].s_value;
	CStatement	t_state(ST_FORMULA, l);
	if (CParser0::ParseEmbedString(str, t_state, d, l))
		return CValue(arg[0].s_value);

	// 実行して結果を返す
	CValue	result = thisfunc->GetFormulaAnswer(lvar, t_state);
	if (t_state.type == ST_FORMULA_SUBST)
		return CValue(F_TAG_NOP, 0/*dmy*/);
	else
		return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ERASEVAR
 *
 *  ローカル変数では空文字列を代入するだけです。
 *  グローバル変数では空文字列を代入し、さらにunload時にファイルへ値を保存しなくなります。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ERASEVAR(vector<CValueSub> &arg, CLocalVariable &lvar, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"ERASEVAR", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"ERASEVAR", d, l);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (!arg[0].s_value.size())
		return CValue();

	if (arg[0].s_value[0] == L'_')
		lvar.Erase(arg[0].s_value);
	else
		variable.Erase(arg[0].s_value);

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETTIME
 *
 *  返値　　：  year,month,day,week(0-6),hour,minute,secondの汎用配列
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETTIME(void)
{
	time_t	ltime;
	time(&ltime);
	tm	*today = localtime(&ltime);
	wchar_t	t_timestr[16];
	wcsftime(t_timestr, STRMAX, L"%Y%m%d%w%H%M%S", today);
	wstring	timestr = t_timestr;

	CValue	result(F_TAG_ARRAY, 0/*dmy*/);

	result.array.push_back(CValueSub(ws_atoi(timestr.substr( 0, 4), 10)));
	result.array.push_back(CValueSub(ws_atoi(timestr.substr( 4, 2), 10)));
	result.array.push_back(CValueSub(ws_atoi(timestr.substr( 6, 2), 10)));
	result.array.push_back(CValueSub(ws_atoi(timestr.substr( 8, 1), 10)));
	result.array.push_back(CValueSub(ws_atoi(timestr.substr( 9, 2), 10)));
	result.array.push_back(CValueSub(ws_atoi(timestr.substr(11, 2), 10)));
	result.array.push_back(CValueSub(ws_atoi(timestr.substr(13, 2), 10)));

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETTICKCOUNT
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
CValue	CSystemFunction::GETTICKCOUNT(vector<CValueSub> &arg, CLocalVariable &lvar, wstring &d, int &l)
{
	DWORD	tickcount = GetTickCount();
	int	highc = (int)(tickcount >> 31);
	int	lowc  = (int)(tickcount & 0x7fffffff);

	if (!arg.size())
		return lowc;

	if (arg[0].GetType() == F_TAG_INT && arg[0].i_value == 0)
		return lowc;

	return highc;
}
#else
CValue CSystemFunction::GETTICKCOUNT(vector<CValueSub> &arg, CLocalVariable &lvar, wstring &d, int &l) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    return static_cast<int>(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETMEMINFO
 *
 *  返値　　：  memoryload,memorytotalphys,memoryavailphys,memorytotalvirtual,memoryavailvirtual
 *  　　　　　  の汎用配列
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
CValue	CSystemFunction::GETMEMINFO(void)
{
	MEMORYSTATUS	meminfo;
	GlobalMemoryStatus(&meminfo);

	CValue	result(F_TAG_ARRAY, 0/*dmy*/);

	result.array.push_back(CValueSub((int)meminfo.dwMemoryLoad)  );
	result.array.push_back(CValueSub((int)meminfo.dwTotalPhys)   );
	result.array.push_back(CValueSub((int)meminfo.dwAvailPhys)   );
	result.array.push_back(CValueSub((int)meminfo.dwTotalVirtual));
	result.array.push_back(CValueSub((int)meminfo.dwAvailVirtual));

	return result;
}
#else
CValue CSystemFunction::GETMEMINFO(void) {
    // メモリの状態を取得するポータブルな方法は無いので…
    CValue result(F_TAG_ARRAY, 0/*dmy*/);
    result.array.push_back(CValueSub(0)); // dwMemoryLoad
    result.array.push_back(CValueSub(0)); // dwTotalPhys
    result.array.push_back(CValueSub(0)); // dwAvailPhys
    result.array.push_back(CValueSub(0)); // dwTotalVirtual
    result.array.push_back(CValueSub(0)); // dwAvailVirtual
    return result;
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_SEARCH
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_SEARCH(vector<CValueSub> &arg, wstring &d, int &l)
{
	ClearReResultDetails();

	// 引数の数/型チェック
	if (arg.size() < 2) {
		logger.Error(E_W, 8, L"RE_SEARCH", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"RE_SEARCH", d, l);
		SetError(9);
		return CValue(0);
	}

	if (!arg[0].s_value.size() ||
		!arg[1].s_value.size())
		return CValue(0);

	// 実行
	wchar_t	*str = (wchar_t *)malloc(sizeof(wchar_t)*(arg[0].s_value.size() + 1));
	::wcscpy(str, arg[0].s_value.c_str());
	int	t_result;
	try {
		boost::reg_expression<wchar_t> regex(arg[1].s_value.c_str());
		boost::match_results<const wchar_t*>	result;
		t_result = (int)boost::regex_search(str, result, regex);
		if (t_result)
			StoreReResultDetails(result);
	}
	catch(const boost::bad_expression &e) {
		t_result = 0;
		logger.Error(E_W, 16, L"RE_SEARCH", d, l);
		SetError(16);
	}
	catch(const runtime_error &e) {
		t_result = 0;
		logger.Error(E_W, 16, L"RE_SEARCH", d, l);
		SetError(16);
	}
	catch(...) {
		t_result = 0;
		logger.Error(E_W, 17, L"RE_SEARCH", d, l);
		SetError(17);
	}

	free(str);

	return CValue(t_result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_MATCH
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_MATCH(vector<CValueSub> &arg, wstring &d, int &l)
{
	ClearReResultDetails();

	// 引数の数/型チェック
	if (arg.size() < 2) {
		logger.Error(E_W, 8, L"RE_MATCH", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"RE_MATCH", d, l);
		SetError(9);
		return CValue(0);
	}

	if (!arg[0].s_value.size() ||
		!arg[1].s_value.size())
		return CValue(0);

	// 実行
	wchar_t	*str = (wchar_t *)malloc(sizeof(wchar_t)*(arg[0].s_value.size() + 1));
	::wcscpy(str, arg[0].s_value.c_str());
	int	t_result;
	try {
		boost::reg_expression<wchar_t> regex(arg[1].s_value.c_str());
		boost::match_results<const wchar_t*>	result;
		t_result = (int)boost::regex_match(str, result, regex);
		if (t_result)
			StoreReResultDetails(result);
	}
	catch(const boost::bad_expression &e) {
		t_result = 0;
		logger.Error(E_W, 16, L"RE_MATCH", d, l);
		SetError(16);
	}
	catch(const runtime_error &e) {
		t_result = 0;
		logger.Error(E_W, 16, L"RE_MATCH", d, l);
		SetError(16);
	}
	catch(...) {
		t_result = 0;
		logger.Error(E_W, 17, L"RE_MATCH", d, l);
		SetError(17);
	}

	free(str);

	return CValue(t_result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_GREP
 *
 *  regex_grepは使用せず、regex_searchを繰り返し実行することで同等の機能としています。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_GREP(vector<CValueSub> &arg, wstring &d, int &l)
{
	ClearReResultDetails();

	// 引数の数/型チェック
	if (arg.size() < 2) {
		logger.Error(E_W, 8, L"RE_GREP", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"RE_GREP", d, l);
		SetError(9);
		return CValue(0);
	}

	if (!arg[0].s_value.size() ||
		!arg[1].s_value.size())
		return CValue(0);

	// 実行
	wchar_t	*str = (wchar_t *)malloc(sizeof(wchar_t)*(arg[0].s_value.size() + 1));
	::wcscpy(str, arg[0].s_value.c_str());
	wchar_t	*search_point = str;
	int	t_pos    = 0;
	int	t_result = 0;

	try {
		boost::reg_expression<wchar_t> regex(arg[1].s_value.c_str());
		boost::match_results<const wchar_t*>	result;
		for( ; ; ) {
			if (!boost::regex_search(search_point, result, regex))
				break;
			t_result++;
			AppendReResultDetail(result.str(0), result.position(0) + t_pos, result.length(0));
			search_point = str + (t_pos += (result.position(0) + result.length(0)));
		}
	}
	catch(const boost::bad_expression &e) {
		t_result = 0;
		logger.Error(E_W, 16, L"RE_GREP", d, l);
		SetError(16);
	}
	catch(const runtime_error &e) {
		t_result = 0;
		logger.Error(E_W, 16, L"RE_GREP", d, l);
		SetError(16);
	}
	catch(...) {
		t_result = 0;
		logger.Error(E_W, 17, L"RE_GREP", d, l);
		SetError(17);
	}

	free(str);

	return CValue(t_result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SETLASTERROR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SETLASTERROR(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"SETLASTERROR", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (arg[0].GetType() == F_TAG_INT ||
		arg[0].GetType() == F_TAG_DOUBLE) {
		lasterror = arg[0].GetValueInt();
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	logger.Error(E_W, 9, L"SETLASTERROR", d, l);
	SetError(9);
	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_SPLIT
 *
 *  regex_splitは使用せず、regex_searchを繰り返し実行することで同等の機能としています。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_SPLIT(vector<CValueSub> &arg, wstring &d, int &l)
{
	ClearReResultDetails();

	// 引数の数/型チェック
	if (arg.size() < 2) {
		logger.Error(E_W, 8, L"RE_SPLIT", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"RE_SPLIT", d, l);
		SetError(9);
		return CValue(0);
	}

	if (!arg[0].s_value.size() ||
		!arg[1].s_value.size())
		return CValue(0);

	return CValue(RE_SPLIT_CORE(arg, d, l, wstring(L"RE_SPLIT")));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_REPLACE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_REPLACE(vector<CValueSub> &arg, wstring &d, int &l)
{
	ClearReResultDetails();

	// 引数の数/型チェック
	if (arg.size() < 3) {
		logger.Error(E_W, 8, L"RE_REPLACE", d, l);
		SetError(8);
		return CValue(arg[0].s_value);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING ||
		arg[2].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"RE_REPLACE", d, l);
		SetError(9);
		return CValue(arg[0].s_value);
	}

	if (!arg[0].s_value.size() ||
		!arg[1].s_value.size())
		return CValue(arg[0].s_value);

	// まずsplitする
	CValue	splits = RE_SPLIT_CORE(arg, d, l, wstring(L"RE_REPLACE"));
	int	num = splits.array.size();
	if (!num || num == 1)
		return CValue(arg[0].s_value);

	// 置換後文字列の作成
	wstring	result;
	int	i = 0;
	for(i = 0; i < num; i++) {
		if (i)
			result += arg[2].s_value;
		result += splits.array[i].s_value;
	}

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_SPLIT_CORE
 *
 *  RE_SPLITの主処理部分です。RE_REPLACEでも使用します。
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::RE_SPLIT_CORE(vector<CValueSub> &arg, wstring &d, int &l, const wstring &fncname)
{
	wchar_t	*str = (wchar_t *)malloc(sizeof(wchar_t)*(arg[0].s_value.size() + 1));
	::wcscpy(str, arg[0].s_value.c_str());
	wchar_t	*search_point = str;
	int	t_pos = 0;
	CValue	splits(F_TAG_ARRAY, 0/*dmy*/);

	try {
		boost::reg_expression<wchar_t> regex(arg[1].s_value.c_str());
		boost::match_results<const wchar_t*>	result;
		for( ; ; ) {
			if (!boost::regex_search(search_point, result, regex))
				break;

			splits.array.push_back(arg[0].s_value.substr(t_pos, result.position(0)));
			AppendReResultDetail(result.str(0), result.position(0) + t_pos, result.length(0));
			search_point = str + (t_pos += (result.position(0) + result.length(0)));
		}
		int	len = ::wcslen(search_point);
//		if (len)
			splits.array.push_back(arg[0].s_value.substr(t_pos, len));
	}
	catch(const boost::bad_expression &e) {
		splits = CValue(F_TAG_ARRAY, 0/*dmy*/);
		logger.Error(E_W, 16, fncname, d, l);
		SetError(16);
	}
	catch(const runtime_error &e) {
		splits = CValue(F_TAG_ARRAY, 0/*dmy*/);
		logger.Error(E_W, 16, fncname, d, l);
		SetError(16);
	}
	catch(...) {
		splits = CValue(F_TAG_ARRAY, 0/*dmy*/);
		logger.Error(E_W, 17, fncname, d, l);
		SetError(17);
	}

	free(str);

	return splits;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CHRCODE
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CHRCODE(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"CHRCODE", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"CHRCODE", d, l);
		SetError(9);
		return CValue(0);
	}

	if (!arg[0].s_value.size()) {
		logger.Error(E_W, 10, L"CHRCODE", d, l);
		SetError(10);
		return CValue(0);
	}

	return CValue((int)arg[0].s_value[0]);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ISINTSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ISINTSTR(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"ISINTSTR", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"ISINTSTR", d, l);
		SetError(9);
		return CValue(0);
	}

	return CValue((int)IsIntString(arg.at(0).s_value));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ISREALSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ISREALSTR(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"ISREALSTR", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"ISREALSTR", d, l);
		SetError(9);
		return CValue(0);
	}

	return CValue((int)IsDoubleString(arg.at(0).s_value));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SPLITPATH
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
CValue	CSystemFunction::SPLITPATH(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"SPLITPATH", d, l);
		SetError(8);
		return CValue(0);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"SPLITPATH", d, l);
		SetError(9);
		return CValue(0);
	}

	wchar_t	drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_wsplitpath(arg[0].s_value.c_str(), drive, dir, fname, ext);

	CValue	result(F_TAG_ARRAY, 0/*dmy*/);
	result.array.push_back(drive);
	result.array.push_back(dir);
	result.array.push_back(fname);
	result.array.push_back(ext);

	return CValue(result);
}
#else
CValue CSystemFunction::SPLITPATH(vector<CValueSub> &arg, wstring &d, int &l) {
    if (!arg.size()) {
	logger.Error(E_W, 8, L"SPLITPATH", d, l);
	SetError(8);
	return CValue(0);
    }
    
    if (arg[0].GetType() != F_TAG_STRING) {
	logger.Error(E_W, 9, L"SPLITPATH", d, l);
	SetError(9);
	return CValue(0);
    }

    wstring path = arg[0].s_value;
    fix_filepath(path);

    CValue result(F_TAG_ARRAY, 0/*dmy*/);
    result.array.push_back(L""); // driveは常に空文字列

    wstring::size_type pos_slash = path.rfind(L'/');
    wstring fname;
    if (pos_slash == wstring::npos) {
	result.array.push_back(L""); // dirも空
	fname = path;
    }
    else {
	result.array.push_back(path.substr(0, pos_slash+1));
	fname = path.substr(pos_slash+1);
    }

    wstring::size_type pos_period = fname.rfind(L'.');
    if (pos_period == wstring::npos) {
	result.array.push_back(fname);
	result.array.push_back(L""); // extは空
    }
    else {
	result.array.push_back(fname.substr(0, pos_period));
	result.array.push_back(fname.substr(pos_period+1));
    }

    return CValue(result);
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CVINT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CVINT(vector<CValueSub> &arg, vector<CCell *> &pcellarg, CLocalVariable &lvar,
			wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"CVINT", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (pcellarg[0]->v.GetType() == F_TAG_VARIABLE)
		variable.SetValue(pcellarg[0]->index, arg.at(0).GetValueInt());
	else if (pcellarg[0]->v.GetType() == F_TAG_LOCALVARIABLE)
		lvar.SetValue(pcellarg[0]->name, CValue(arg.at(0).GetValueInt()));
	else {
		logger.Error(E_W, 11, L"CVINT", d, l);
		SetError(11);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CVSTR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CVSTR(vector<CValueSub> &arg, vector<CCell *> &pcellarg, CLocalVariable &lvar,
			wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"CVSTR", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (pcellarg[0]->v.GetType() == F_TAG_VARIABLE)
		variable.SetValue(pcellarg[0]->index, arg.at(0).GetValueString());
	else if (pcellarg[0]->v.GetType() == F_TAG_LOCALVARIABLE)
		lvar.SetValue(pcellarg[0]->name, CValue(arg.at(0).GetValueString()));
	else {
		logger.Error(E_W, 11, L"CVSTR", d, l);
		SetError(11);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::CVREAL
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::CVREAL(vector<CValueSub> &arg, vector<CCell *> &pcellarg, CLocalVariable &lvar,
			wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"CVREAL", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (pcellarg[0]->v.GetType() == F_TAG_VARIABLE)
		variable.SetValue(pcellarg[0]->index, arg.at(0).GetValueDouble());
	else if (pcellarg[0]->v.GetType() == F_TAG_LOCALVARIABLE)
		lvar.SetValue(pcellarg[0]->name, CValue(arg.at(0).GetValueDouble()));
	else {
		logger.Error(E_W, 11, L"CVREAL", d, l);
		SetError(11);
	}

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::LETTONAME
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::LETTONAME(vector<CValueSub> &arg, wstring &d, int &l, CLocalVariable &lvar,
			CFunction *thisfunc)
{
	int	sz = arg.size();

	if (sz < 2) {
		logger.Error(E_W, 8, L"LETTONAME", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"LETTONAME", d, l);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	// 代入式を構成する　ダブルクォートはCHRでエスケープ(?)する
	wstring	str = arg[0].s_value;
	str += L"=(";
	for(int i = 1; i < sz; i++) {
		if (i > 1)
			str += L",";
		if (arg[i].GetType() == F_TAG_STRING) {
			wstring	vstr = arg[i].s_value;
			ws_replace(vstr, wstring(L"\""), wstring(L"%(CHR(0x22))"));
			str += (L"\"" + vstr + L"\"");
		}
		else
			str += arg[i].GetValueString();
	}
	str += L")";
	// 数式へ展開して実行
	CStatement	t_state(ST_FORMULA, l);
	if (!CParser0::ParseEmbedString(str, t_state, d, l))
		thisfunc->GetFormulaAnswer(lvar, t_state);

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::STRFORM
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::STRFORM(vector<CValueSub> &arg, wstring &d, int &l)
{
	int	sz = arg.size();

	if (!sz)
		return CValue();

	if (sz == 1)
		return CValue(arg[0]);

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"STRFORM", d, l);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	// '$'でsplitする
	vector<wstring>	vargs;
	int	vargs_sz = SplitToMultiString(arg[0].s_value, vargs, wstring(L"$"));
	if (!vargs_sz)
		return CValue();

	// 各要素ごとに_snwprintfで書式化して結合していく
	wstring	left, right;
	wstring	result = vargs[0];
	wchar_t	t_str[STRMAX + 1];
	for(int i = 1; i < vargs_sz; i++) {
		wstring	format = L"%" + vargs[i];
		if (i < sz) {
			switch(arg[i].GetType()) {
			case F_TAG_INT:
				swprintf(t_str, STRMAX, format.c_str(), arg[i].i_value);
				break;
			case F_TAG_DOUBLE:
				swprintf(t_str, STRMAX, format.c_str(), arg[i].d_value);
				break;
			case F_TAG_STRING:
				swprintf(t_str, STRMAX, format.c_str(), arg[i].s_value.c_str());
				break;
			default:
				logger.Error(E_E, 91, d, l);
				return CValue();
			};
			result += t_str;
		}
		else
			result += L"$" + vargs[i];
	}

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ANY
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ANY(vector<CValueSub> &arg, vector<CCell *> &pcellarg, CLocalVariable &lvar,
			wstring &d, int &l)
{
	// 引数無しなら空文字列
	int	sz = arg.size();
	if (!sz) {
		SetLso(-1);
		return CValue();
	}

	if (sz == 1) {
		// 引数1つで文字列でないなら引数がそのまま返る
		if (arg[0].GetType() != F_TAG_STRING) {
			SetLso(0);
			return CValue(arg[0]);
		}

		// 引数1つで文字列なら簡易配列として処理　変数の場合はそのデリミタで分割する
		wstring	delimiter = VAR_DELIMITER;
		if (pcellarg[0]->v.GetType() == F_TAG_VARIABLE)
			delimiter = variable.GetDelimiter(pcellarg[0]->index);
		else if (pcellarg[0]->v.GetType() == F_TAG_LOCALVARIABLE)
			delimiter = lvar.GetDelimiter(pcellarg[0]->name);

		vector<wstring>	s_array;
		int	a_sz = SplitToMultiString(arg[0].s_value, s_array, delimiter);
		if (!a_sz) {
			SetLso(-1);
			return CValue();
		}

		int s_pos = (int)(genrand_real2()*(double)a_sz);
		SetLso(s_pos);
		return CValue(s_array[s_pos]);
	}

	// 引数が複数なら汎用配列として処理
	int s_pos = (int)(genrand_real2()*(double)sz);
	SetLso(s_pos);
	return CValue(arg[s_pos]);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SAVEVAR
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SAVEVAR(void)
{
	basis.SaveVariable();

	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETSTRBYTES
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETSTRBYTES(vector<CValueSub> &arg, wstring &d, int &l)
{
	int	sz = arg.size();

	if (!sz) {
		logger.Error(E_W, 8, L"GETSTRBYTES", d, l);
		SetError(8);
		return CValue(0);
	}

	// 文字コード取得
	int	charset = CHARSET_SJIS;
	if (sz > 1) {
		if (arg[1].GetType() != F_TAG_INT &&
			arg[1].GetType() != F_TAG_DOUBLE) {
			logger.Error(E_W, 9, L"GETSTRBYTES", d, l);
			SetError(9);
			return CValue(0);
		}
		charset = arg[1].GetValueInt();
		if (charset != CHARSET_SJIS &&
			charset != CHARSET_UTF8 &&
			charset != CHARSET_DEFAULT) {
			logger.Error(E_W, 12, L"GETSTRBYTES", d, l);
			SetError(12);
			return CValue(0);
		}
	}
	
	// 主処理
	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"STRFORM", d, l);
		SetError(9);
		return CValue(0);
	}
	char	*t_str = Ccct::Ucs2ToMbcs(arg[0].s_value, charset);
	if (t_str == NULL) {
		logger.Error(E_E, 89, L"STRFORM", d, l);
		return CValue(0);
	}
	int	result = ::strlen(t_str);
	free(t_str);

	return CValue(result);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ASEARCH
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ASEARCH(vector<CValueSub> &arg, wstring &d, int &l)
{
	int	sz = arg.size();

	if (sz < 2) {
		logger.Error(E_W, 8, L"ASEARCH", d, l);
		SetError(8);
		return CValue(-1);
	}

	CValueSub	&key = arg[0];
	for(int i = 1; i < sz; i++)
		if ((key == arg[i]).GetValueInt())
			return CValue(i - 1);

	return CValue(-1);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ASEARCHEX
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::ASEARCHEX(vector<CValueSub> &arg, wstring &d, int &l)
{
	int	sz = arg.size();

	if (sz < 2) {
		logger.Error(E_W, 8, L"ASEARCHEX", d, l);
		SetError(8);
		return CValue(F_TAG_ARRAY, 0/*dmy*/);
	}

	CValue	result(F_TAG_ARRAY, 0/*dmy*/);
	CValueSub	&key = arg[0];
	for(int i = 1; i < sz; i++)
		if ((key == arg[i]).GetValueInt())
			result.array.push_back(CValueSub(i - 1));

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETDELIM
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETDELIM(vector<CCell *> &pcellarg, CLocalVariable &lvar, wstring &d, int &l)
{
	if (!pcellarg.size()) {
		logger.Error(E_W, 8, L"GETDELIM", d, l);
		SetError(8);
		return CValue(L"");
	}

	CValue	delimiter(L"");
	if (pcellarg[0]->v.GetType() == F_TAG_VARIABLE)
		delimiter = variable.GetDelimiter(pcellarg[0]->index);
	else if (pcellarg[0]->v.GetType() == F_TAG_LOCALVARIABLE)
		delimiter = lvar.GetDelimiter(pcellarg[0]->name);
	else {
		logger.Error(E_W, 18, L"GETDELIM", d, l);
		SetError(18);
	}

	return delimiter;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::GETSETTING
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::GETSETTING(vector<CValueSub> &arg, wstring &d, int &l)
{
	int	sz = arg.size();

	if (!sz) {
		logger.Error(E_W, 8, L"GETSETTING", d, l);
		SetError(8);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	if (arg[0].GetType() != F_TAG_INT &&
		arg[0].GetType() != F_TAG_DOUBLE) {
		logger.Error(E_W, 9, L"GETSETTING", d, l);
		SetError(9);
		return CValue(F_TAG_NOP, 0/*dmy*/);
	}

	switch(arg[0].GetValueInt()) {
	case 0:	// AYAINFO_VERSION
		return CValue(wstring(AYA_VERSION));
	case 1:	// AYAINFO_CHARSET
		return CValue(static_cast<int>(basis.GetCharset()));
	case 2:	// AYAINFO_PATH
		return CValue(basis.GetRootPath());
	default:
		break;
	};

	logger.Error(E_W, 12, L"GETSETTING", d, l);
	SetError(12);
	return CValue(F_TAG_NOP, 0/*dmy*/);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SPLIT
 * -----------------------------------------------------------------------
 */
CValue	CSystemFunction::SPLIT(vector<CValueSub> &arg, wstring &d, int &l)
{
	int	sz = arg.size();

	if (sz < 2) {
		logger.Error(E_W, 8, L"SPLIT", d, l);
		SetError(8);
		return CValue(F_TAG_ARRAY, 0/*dmy*/);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"SPLIT", d, l);
		SetError(9);
		return CValue(F_TAG_ARRAY, 0/*dmy*/);
	}

	int	nums = 0;
	if (sz > 2) {
		if (arg[2].GetType() != F_TAG_INT &&
			arg[2].GetType() != F_TAG_DOUBLE) {
			logger.Error(E_W, 9, L"SPLIT", d, l);
			SetError(9);
			return CValue(F_TAG_ARRAY, 0/*dmy*/);
		}
		nums = arg[2].GetValueInt();
	}

	CValue	result(F_TAG_ARRAY, 0/*dmy*/);

	if (nums == 1) {
		result.array.push_back(CValueSub(arg[0].GetValueString()));
		return result;
	}

	wstring	tgt_str(arg[0].GetValueString());
	wstring	sep_str(arg[1].GetValueString());
	int	sep_strlen = sep_str.size();
	for(int i = 1; ; i++) {
		int	spoint = tgt_str.find(sep_str);
		if (spoint == -1 || i == nums) {
			result.array.push_back(CValueSub(tgt_str));
			break;
		}
		result.array.push_back(CValueSub(tgt_str.substr(0, spoint)));
		spoint += sep_strlen;
		tgt_str = tgt_str.substr(spoint, tgt_str.size() - spoint);
	}

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::FATTRIB
 *
 *  4GB以上のサイズは取得できません
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
CValue	CSystemFunction::FATTRIB(vector<CValueSub> &arg, wstring &d, int &l)
{
	if (!arg.size()) {
		logger.Error(E_W, 8, L"FATTRIB", d, l);
		SetError(8);
		return CValue(-1);
	}

	if (arg[0].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"FATTRIB", d, l);
		SetError(9);
		return CValue(-1);
	}

	// パスをMBCSに変換
	char	*s_filestr = Ccct::Ucs2ToMbcs(ToFullPath(arg[0].s_value), CHARSET_DEFAULT);
	if (s_filestr == NULL) {
		logger.Error(E_E, 89, L"FATTRIB", d, l);
		return CValue(-1);
	}

	// 取得
	DWORD	attrib = GetFileAttributes(s_filestr);
	free(s_filestr);

	if (attrib == 0xFFFFFFFF)
		return CValue(-1);

	// 返値生成
	CValue	result(F_TAG_ARRAY, 0/*dmy*/);
	result.array.push_back(CValueSub((attrib & FILE_ATTRIBUTE_ARCHIVE   ) ? 1 : 0));
	result.array.push_back(CValueSub((attrib & FILE_ATTRIBUTE_COMPRESSED) ? 1 : 0));
	result.array.push_back(CValueSub((attrib & FILE_ATTRIBUTE_DIRECTORY ) ? 1 : 0));
	result.array.push_back(CValueSub((attrib & FILE_ATTRIBUTE_HIDDEN    ) ? 1 : 0));
	result.array.push_back(CValueSub((attrib & FILE_ATTRIBUTE_NORMAL    ) ? 1 : 0));
	result.array.push_back(CValueSub((attrib & FILE_ATTRIBUTE_OFFLINE   ) ? 1 : 0));
	result.array.push_back(CValueSub((attrib & FILE_ATTRIBUTE_READONLY  ) ? 1 : 0));
	result.array.push_back(CValueSub((attrib & FILE_ATTRIBUTE_SYSTEM    ) ? 1 : 0));
	result.array.push_back(CValueSub((attrib & FILE_ATTRIBUTE_TEMPORARY ) ? 1 : 0));

	return result;
}
#else
CValue CSystemFunction::FATTRIB(vector<CValueSub> &arg, wstring &d, int &l) {
    if (!arg.size()) {
	logger.Error(E_W, 8, L"FATTRIB", d, l);
	SetError(8);
	return CValue(-1);
    }
    
    if (arg[0].GetType() != F_TAG_STRING) {
	logger.Error(E_W, 9, L"FATTRIB", d, l);
	SetError(9);
	return CValue(-1);
    }

	CValue	result(F_TAG_ARRAY, 0/*dmy*/);
	result.array.push_back(CValueSub(0));
	result.array.push_back(CValueSub(0));
	result.array.push_back(CValueSub(0));
	result.array.push_back(CValueSub(0));
	result.array.push_back(CValueSub(0));
	result.array.push_back(CValueSub(0));
	result.array.push_back(CValueSub(0));
	result.array.push_back(CValueSub(0));
	result.array.push_back(CValueSub(0));

	return result;
}
#endif

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::RE_REPLACEEX
 * -----------------------------------------------------------------------
 */
/*CValue	CSystemFunction::RE_REPLACEEX(vector<CValueSub> &arg, wstring &d, int &l)
{
	ClearReResultDetails();

	// 引数の数/型チェック
	if (arg.size() < 3) {
		logger.Error(E_W, 8, L"RE_REPLACEEX", d, l);
		SetError(8);
		return CValue(arg[0].s_value);
	}

	if (arg[0].GetType() != F_TAG_STRING ||
		arg[1].GetType() != F_TAG_STRING ||
		arg[2].GetType() != F_TAG_STRING) {
		logger.Error(E_W, 9, L"RE_REPLACEEX", d, l);
		SetError(9);
		return CValue(arg[0].s_value);
	}

	if (!arg[0].s_value.size() ||
		!arg[1].s_value.size())
		return CValue(arg[0].s_value);

	// 実行
	wstring	t_result;
	try {
		boost::reg_expression<wchar_t> regex(arg[1].s_value.c_str());
		boost::match_results<const wchar_t*>	result;
		t_result = boost::regex_replace(arg[0].s_value, regex, arg[2].s_value);
	}
	catch(const boost::bad_expression &e) {
		t_result = L"";
		logger.Error(E_W, 16, L"RE_REPLACEEX", d, l);
		SetError(16);
	}
	catch(const runtime_error &e) {
		t_result = L"";
		logger.Error(E_W, 16, L"RE_REPLACEEX", d, l);
		SetError(16);
	}
	catch(...) {
		t_result = L"";
		logger.Error(E_W, 17, L"RE_REPLACEEX", d, l);
		SetError(17);
	}

	return CValue(t_result);
}*/

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::MakeReResultDetails
 *  機能概要：  正規表現系関数の処理結果詳細を蓄積します
 * -----------------------------------------------------------------------
 */
void	CSystemFunction::StoreReResultDetails(boost::match_results<const wchar_t*> &result)
{
	int	sz = result.size();
	for(int i = 0; i < sz; i++)
		AppendReResultDetail(result.str(i), result.position(i), result.length(i));
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::ClearReResultDetails
 *  機能概要：  正規表現系関数の処理結果詳細をクリアします
 * -----------------------------------------------------------------------
 */
void	CSystemFunction::ClearReResultDetails(void)
{
	re_str.array.clear();
	re_pos.array.clear();
	re_len.array.clear();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::AppendReResultDetail
 *  機能概要：  正規表現系関数の処理結果詳細を1つ蓄積します
 * -----------------------------------------------------------------------
 */
void	CSystemFunction::AppendReResultDetail(const wstring &str, int pos, int len)
{
	re_str.array.push_back(str);
	re_pos.array.push_back(pos);
	re_len.array.push_back(len);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CSystemFunction::SetError
 *  機能概要：  GETLASTERRORが返す値を設定します
 * -----------------------------------------------------------------------
 */
void	CSystemFunction::SetError(int code)
{
	lasterror = code;
}

