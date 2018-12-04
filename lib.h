// 
// AYA version 5
//
// 外部ライブラリを扱うクラス　CLib/CLib1
// written by umeici. 2004
// 
// CLibが全体を管理します。個々のDLLはCLib1で管理します。
//

#ifndef	LIBH
#define	LIBH

//----

#ifndef POSIX
#  include "stdafx.h"
#else
#  include <list>
#  include <string>
using namespace std;
#endif

class	CLib1
{
protected:
	wstring	name;
#ifndef POSIX
	HMODULE	hDLL;
#else
	void*   hDLL;
#endif
	int		charset;

public:
	CLib1(const wstring &n, int cs)
	{
		name    = n;
		charset = cs;
		hDLL    = NULL;
	}

	CLib1(void) { hDLL = NULL; }
	~CLib1(void) { Unload(); Release(); }

	wstring	GetName(void) { return name; }

	int		LoadLib(void);
	int		Load(void);
	int		Unload(void);
	void	Release(void);

	int		Request(const wstring &istr, wstring &ostr);
};

//----

class	CLib
{
protected:
	list<CLib1>	liblist;
	int			charset;

public:
	~CLib(void) { DeleteAll(); }

	void	SetCharset(int cs) { charset = cs; }

	int		Add(const wstring &name);
	int		Delete(const wstring &name);
	void	DeleteAll(void);

	int		Request(const wstring &name, const wstring &istr, wstring &ostr);
};

//----

#endif
