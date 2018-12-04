// 
// AYA version 5
//
// stl::wstring‚ğchar*•—‚Ég‚¤‚½‚ß‚ÌŠÖ”‚È‚Ç
// written by umeici. 2004
// 

#ifndef	WSEXH
#define	WSEXH

#ifndef POSIX
#  include "stdafx.h"
#else
#  include <string>
#  include <stdio.h>
using namespace std;
#endif

//----

#define	WS_MAXLEN	1024
#define	WS_EOF		-1

int		ws_atoi(const wstring &str, int base);
double	ws_atof(const wstring &str);
void	ws_itoa(wstring &str, int num, int base);
void	ws_ftoa(wstring &str, double num);
void	ws_eraseend(wstring &str, wchar_t c);
void	ws_replace(wstring &str, const wstring &before, const wstring &after);

FILE	*w_fopen(const wchar_t *fname, const wchar_t *mode); 
//void	write_utf8bom(FILE *fp);
int		ws_fgets(wstring &str, FILE *stream, int charset, int ayc, int lc);
void	cutbom(wstring &wstr);
inline void	decodecipher(int &c);

//----

#endif
