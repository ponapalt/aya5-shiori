// 
// AYA version 5
//
// �V�X�e���֐������s����N���X�@CSystemFunction
// written by umeici. 2004
// 
// ���K�\���n�̊֐��ł�boost::regex���g�p���Ă��܂��B
// http://www.boost.org/
// http://www.boost.org/libs/regex/doc/index.html
// �M��
// http://boost.cppll.jp/HEAD/libs/regex/index.htm
// boost::regex�ɂ����鐳�K�\���\���ɂ��Ă͈ȉ����Q��
// http://boost.cppll.jp/HEAD/libs/regex/syntax.htm
//

#ifndef	SYSFUNCH
#define	SYSFUNCH

//----

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "value.h"
#include "cell.h"
#include "variable.h"
#include "function.h"

#include <boost/regex.hpp>	/* boost::regex���g�p���܂� */
#define	BOOST_REGEX_STATIC_LINK

#define	I_PERF_DATA_SIZE			32768
#define	D_PERF_DATA_SIZE			1024

#define	SYSFUNC_NUM					91
#define	SYSFUNC_HIS					61

const wchar_t	sysfunc[SYSFUNC_NUM][32] = {
	// �^�擾/�ϊ�
	L"TOINT",
	L"TOREAL",
	L"TOSTR",
	L"GETTYPE",
	L"ISFUNC",
	L"ISVAR",
	// �f�o�b�O
	L"LOGGING",
	L"GETLASTERROR",
	// �O�����C�u����
	L"LOADLIB",
	L"UNLOADLIB",
	L"REQUESTLIB",
	L"CHARSETLIB",
	// ���l
	L"RAND",
	L"FLOOR",
	L"CEIL",
	L"ROUND",
	L"SIN",
	L"COS",
	L"TAN",
	L"LOG",
	L"LOG10",
	L"POW",
	L"SQRT",
	// �����񑀍�
	L"STRSTR",
	L"STRLEN",
	L"REPLACE",
	L"SUBSTR",
	L"ERASE",
	L"INSERT",
	L"TOUPPER",
	L"TOLOWER",
	L"CUTSPACE",
	L"TOBINSTR",
	L"TOHEXSTR",
	L"BINSTRTOI",
	L"HEXSTRTOI",
	L"CHR",
	// �t�@�C������
	L"FOPEN",
	L"FCLOSE",
	L"FREAD",
	L"FWRITE",
	L"FWRITE2",
	L"FCOPY",
	L"FMOVE",
	L"MKDIR",
	L"RMDIR",
	L"FDEL",
	L"FRENAME",
	L"FSIZE",
	L"FENUM",
	L"FCHARSET",
	// �z��
	L"ARRAYSIZE",
	L"SETDELIM",
	// ����
	L"EVAL",
	L"ERASEVAR",
	// �V�X�e������/���������
	L"GETTIME",
	L"GETTICKCOUNT",
	L"GETMEMINFO",
	// ���K�\��
	L"RE_SEARCH",
	L"RE_MATCH",
	L"RE_GREP",
	// �V�X�e���Ŏg�p
	L"EmBeD_HiStOrY",	// %[n]�i�u���ς̒l�̍ė��p�j�����p
	// �f�o�b�O�p(2)
	L"SETLASTERROR",
	// ���K�\��(2)
	L"RE_REPLACE",
	L"RE_SPLIT",
	L"RE_GETSTR",
	L"RE_GETPOS",
	L"RE_GETLEN",
	// �����񑀍�(2)
	L"CHRCODE",
	L"ISINTSTR",
	L"ISREALSTR",
	// �z��(2)
	L"IARRAY",
	// �����񑀍�(3)
	L"SPLITPATH",
	// �^�擾/�ϊ�(2)
	L"CVINT",
	L"CVSTR",
	L"CVREAL",
	// ����(2)
	L"LETTONAME",
	L"LSO",
	// �����񑀍�(4)
	L"STRFORM",
	L"ANY",
	// ����(3)
	L"SAVEVAR",
	// �����񑀍�(5)
	L"GETSTRBYTES",
	// �z��(3)
	L"ASEARCH",
	L"ASEARCHEX",
	// �z��(2)
	L"GETDELIM",
	// ����(4)
	L"GETSETTING",
	// ���l(2)
	L"ASIN",
	L"ACOS",
	L"ATAN",
	// �����񑀍�(6)
	L"SPLIT",
	// �t�@�C������(2)
	L"FATTRIB"
//	// ���K�\��(3)
//	L"RE_REPLACEEX"
};

//----

class	CSystemFunction
{
protected:
	int		sysfunc_len[SYSFUNC_NUM];	// �e�֐����̒���

	int		lasterror;					// �Ŋ��ɔ��������G���[�̔ԍ�
	int		lso;						// LSO���Ԃ��l

	CValue	re_str;						// ���K�\���������ʂ̏ڍ׏��i��v����������j
	CValue	re_pos;						// ���K�\���������ʂ̏ڍ׏��i��v�����ʒu�j
	CValue	re_len;						// ���K�\���������ʂ̏ڍ׏��i��v���������j

public:
	CSystemFunction(void)
		: re_str(F_TAG_ARRAY, 0/*dmy*/), re_pos(F_TAG_ARRAY, 0/*dmy*/), re_len(F_TAG_ARRAY, 0/*dmy*/)
	{
		lasterror   = 0;
		lso         = -1;

		for(int i = 0; i < SYSFUNC_NUM; i++)
			sysfunc_len[i] = ::wcslen(sysfunc[i]);
	}

	int	GetNameLen(int index)
	{
		return (index >= 0 && index < SYSFUNC_NUM) ? sysfunc_len[index] : 0;
	}

	void	SetLso(int order) { lso = order; }

	CValue	*GetReStrPtr(void) { return &re_str; }
	CValue	*GetRePosPtr(void) { return &re_pos; }
	CValue	*GetReLenPtr(void) { return &re_len; }

	CValue	Execute(int index, CValue &arg, vector<CCell *> &pcellarg,
				CLocalVariable &lvar, int l, CFunction *thisfunc);

protected:
	CValue	TOINT(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	TOREAL(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	TOSTR(CValue &args, wstring &d, int &l);
	CValue	GETTYPE(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	ISFUNC(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	ISVAR(vector<CValueSub> &arg, CLocalVariable &lvar, wstring &d, int &l);
	CValue	LOGGING(CValue &arg, wstring &d, int &l);
	CValue	LOADLIB(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	UNLOADLIB(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	REQUESTLIB(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	CHARSETLIB(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	RAND(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	FLOOR(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	CEIL(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	ROUND(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	SIN(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	COS(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	TAN(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	ASIN(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	ACOS(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	ATAN(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	LOG(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	LOG10(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	POW(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	SQRT(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	STRSTR(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	STRLEN(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	REPLACE(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	SUBSTR(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	ERASE(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	INSERT(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	TOUPPER(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	TOLOWER(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	TOBINSTR(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	TOHEXSTR(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	BINSTRTOI(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	HEXSTRTOI(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	CUTSPACE(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	CHR(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	FOPEN(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	FCLOSE(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	FREAD(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	FWRITE(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	FWRITE2(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	FCOPY(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	FMOVE(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	MKDIR(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	RMDIR(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	FDEL(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	FRENAME(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	FSIZE(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	FENUM(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	FCHARSET(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	ARRAYSIZE(vector<CValueSub> &arg, vector<CCell *> &pcellarg, CLocalVariable &lvar,
				wstring &d, int &l);
	CValue	SETDELIM(vector<CCell *> &pcellarg, CLocalVariable &lvar, wstring &d, int &l);
	CValue	EVAL(vector<CValueSub> &arg, wstring &d, int &l, CLocalVariable &lvar,
				CFunction *thisfunc);
	CValue	ERASEVAR(vector<CValueSub> &arg, CLocalVariable &lvar, wstring &d, int &l);
	CValue	GETTIME(void);
	CValue	GETTICKCOUNT(vector<CValueSub> &arg, CLocalVariable &lvar, wstring &d, int &l);
	CValue	GETMEMINFO(void);
	CValue	RE_SEARCH(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	RE_MATCH(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	RE_GREP(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	SETLASTERROR(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	RE_REPLACE(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	RE_SPLIT(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	CHRCODE(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	ISINTSTR(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	ISREALSTR(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	SPLITPATH(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	CVINT(vector<CValueSub> &arg, vector<CCell *> &pcellarg, CLocalVariable &lvar,
				wstring &d, int &l);
	CValue	CVSTR(vector<CValueSub> &arg, vector<CCell *> &pcellarg, CLocalVariable &lvar,
				wstring &d, int &l);
	CValue	CVREAL(vector<CValueSub> &arg, vector<CCell *> &pcellarg, CLocalVariable &lvar,
				wstring &d, int &l);
	CValue	LETTONAME(vector<CValueSub> &arg, wstring &d, int &l, CLocalVariable &lvar,
				CFunction *thisfunc);
	CValue	STRFORM(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	ANY(vector<CValueSub> &arg, vector<CCell *> &pcellarg, CLocalVariable &lvar,
				wstring &d, int &l);
	CValue	SAVEVAR(void);
	CValue	GETSTRBYTES(vector<CValueSub> &arg, wstring &d, int &l);
//	CValue	RE_REPLACEEX(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	ASEARCH(vector<CValueSub> &arg, wstring &d, int &l);
	CValue	ASEARCHEX(vector<CValueSub> &arg, wstring &d, int &l);

	CValue	GETDELIM(vector<CCell *> &pcellarg, CLocalVariable &lvar, wstring &d, int &l);
	CValue	GETSETTING(vector<CValueSub> &arg, wstring &d, int &l);

	CValue	SPLIT(vector<CValueSub> &arg, wstring &d, int &l);

	CValue	FATTRIB(vector<CValueSub> &arg, wstring &d, int &l);

	CValue	RE_SPLIT_CORE(vector<CValueSub> &arg, wstring &d, int &l, const wstring &fncname);

	void	StoreReResultDetails(boost::match_results<const wchar_t*> &result);
	void	ClearReResultDetails(void);
	void	AppendReResultDetail(const wstring &str, int pos, int len);
	void	SetError(int code);
};

//----

#endif
