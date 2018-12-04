// 
// AYA version 5
//
// ファイルを扱うクラス　CFile/CFile1
// written by umeici. 2004
// 
// CFileが全体を管理します。個々のファイルはCFile1で管理します。
//

#ifndef	FILEH
#define	FILEH

//----

#ifndef POSIX
#  include "stdafx.h"
#else
#  include <list>
#  include <string>
using namespace std;
#endif

class	CFile1
{
protected:
	wstring	name;
	FILE	*fp;
	int		charset;
	wstring	mode;

	int	bomcheck;

public:
	CFile1(const wstring &n, int cs, wstring md)
	{
		name     = n;
		charset  = cs;
		mode     = md;
		fp       = NULL;

		bomcheck = 1;
	}

	CFile1(void) { fp = NULL; }
	~CFile1(void) { Close(); }

	wstring	GetName(void) { return name; }

	int	Open(void);
	int	Close(void);

	int	Write(const wstring &istr);
	int	Read(wstring &ostr);
};

//----

class	CFile
{
protected:
	list<CFile1>	filelist;
	int			charset;

public:
	~CFile(void) { DeleteAll(); }

	void	SetCharset(int cs) { charset = cs; }

	int		Add(const wstring &name, const wstring &mode);
	int		Delete(const wstring &name);
	void	DeleteAll(void);

	int		Write(const wstring &name, const wstring &istr);
	int		Read(const wstring &name, wstring &ostr);
};

//----

#endif
