// 
// AYA version 5
//
// ファイルを扱うクラス　CFile
// written by umeici. 2004
// 
// write/readの度にlistから対象を検索していますが、一度に取り扱うファイルは
// 多くても数個だと思うので、これでも実行速度に問題はないと考えています。
//

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "file.h"
#include "misc.h"

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile::Add
 *  機能概要：  指定されたファイルをオープンします
 *
 *  返値　　：　0/1/2=失敗/成功/既にオープンしている
 * -----------------------------------------------------------------------
 */
int	CFile::Add(const wstring &name, const wstring &mode)
{
        list<CFile1>::iterator it;
	for(it = filelist.begin(); it != filelist.end(); it++)
		if (!name.compare(it->GetName()))
			return 2;

	wstring	t_mode = mode;
	if (!t_mode.compare(L"read"))
		t_mode = L"r";
	else if (!t_mode.compare(L"write"))
		t_mode = L"w";
	else if (!t_mode.compare(L"append"))
		t_mode = L"a";

	if (t_mode.compare(L"r") &&
		t_mode.compare(L"w") &&
		t_mode.compare(L"a"))
		return 0;

	filelist.push_back(CFile1(name, charset, t_mode));
	it = filelist.end();
	it--;
	if (!it->Open()) {
		filelist.erase(it);
		return 0;
	}

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile::Delete
 *  機能概要：  指定されたファイルをクローズします
 *
 *  返値　　：　1/2=成功/オープンされていない、もしくは既にfcloseされている
 * -----------------------------------------------------------------------
 */
int	CFile::Delete(const wstring &name)
{
	for(list<CFile1>::iterator it = filelist.begin(); it != filelist.end(); it++)
		if (!name.compare(it->GetName())) {
			int	result = it->Close();
			filelist.erase(it);
			return result;
		}

	return 2;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile::DeleteAll
 *  機能概要：  すべてのファイルをクローズします
 * -----------------------------------------------------------------------
 */
void	CFile::DeleteAll(void)
{
	filelist.clear();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile::Write
 *  機能概要：  ファイルに文字列を書き込みます
 *
 *  返値　　：　0/1=失敗/成功
 * -----------------------------------------------------------------------
 */
int	CFile::Write(const wstring &name, const wstring &istr)
{
	for(list<CFile1>::iterator it = filelist.begin(); it != filelist.end(); it++)
		if (!name.compare(it->GetName()))
			return it->Write(istr);

	return 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile::Read
 *  機能概要：  ファイルから文字列を1行読み取ります
 *
 *  返値　　：　-1/0/1=EOF/失敗/成功
 * -----------------------------------------------------------------------
 */
int	CFile::Read(const wstring &name, wstring &ostr)
{
	for(list<CFile1>::iterator it = filelist.begin(); it != filelist.end(); it++)
		if (!name.compare(it->GetName())) {
			int	result = it->Read(ostr);
			return result;
		}

	return 0;
}
