// 
// AYA version 5
//
// コメントアウト処理クラス　CComment
// written by umeici. 2004
// 

#ifndef	CCOMMENTH
#define	CCOMMENTH

//----

#ifndef POSIX
#  include "stdafx.h"
#else
#  include <string>
using namespace std;
#endif

class	CComment
{
protected:
	char	flag;		// 領域フラグ 0/1=コメントエリア外/内
public:
	CComment(void) { flag = 0; }
	~CComment(void) {}

	void	Execute(wstring &str);

	void	ExecuteSS_Top(wstring &str);
	void	ExecuteSS_Tail(wstring &str);
};

//----

#endif
