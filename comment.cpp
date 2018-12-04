// 
// AYA version 5
//
// コメントアウト処理クラス　CComment
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "comment.h"
#include "misc.h"

/* -----------------------------------------------------------------------
 *  関数名  ：  CComment::Execute
 *  機能概要：  コメントアウト処理（/＊～＊/）
 * -----------------------------------------------------------------------
 */
void	CComment::Execute(wstring &str)
{
	wstring	outstr;
	for( ; ; ) {
		wstring dstr0, dstr1, dstr2, dstr3;
		char	res0 = Split_IgnoreDQ(str, dstr0, dstr1, L"/*");
		char	res1 = Split_IgnoreDQ(str, dstr2, dstr3, L"*/");
		if (!res0 && !res1) {
			if (!flag)
				outstr.append(str);
			break;
		}
		if (!res0 || (res0 && res1 && dstr0.size() >= dstr2.size())) {
			if (!flag)
				outstr.append(dstr2);
			flag = 0;
			str = dstr3;
		}
		else {
			if (!flag)
				outstr.append(dstr0);
			flag = 1;
			str = dstr1;
		}
	}
	str = outstr;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CComment::ExecuteSS_Top
 *  機能概要：  コメントアウト処理（先頭の//）
 * -----------------------------------------------------------------------
 */
void	CComment::ExecuteSS_Top(wstring &str)
{
	if (!str.substr(0, 2).compare(L"//"))
		str = L"";
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CComment::ExecuteSS_Tail
 *  機能概要：  コメントアウト処理（中途の//）
 * -----------------------------------------------------------------------
 */
void	CComment::ExecuteSS_Tail(wstring &str)
{
	wstring	dstr0, dstr1;
	if (Split_IgnoreDQ(str, dstr0, dstr1, L"//"))
		str = dstr0;
}
