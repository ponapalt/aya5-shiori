// 
// AYA version 5
//
// �R�����g�A�E�g�����N���X�@CComment
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
	char	flag;		// �̈�t���O 0/1=�R�����g�G���A�O/��
public:
	CComment(void) { flag = 0; }
	~CComment(void) {}

	void	Execute(wstring &str);

	void	ExecuteSS_Top(wstring &str);
	void	ExecuteSS_Tail(wstring &str);
};

//----

#endif
