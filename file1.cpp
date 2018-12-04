// 
// AYA version 5
//
// 1つのファイルを扱うクラス　CFile1
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#else
#  include "manifest.h"
#endif
#include "file.h"
#include "misc.h"
#include "ccct.h"
#include "wsex.h"

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::Open
 *  機能概要：  ファイルをオープンします
 *
 *  返値　　：　0/1=失敗/成功(既にロードされている含む)
 * -----------------------------------------------------------------------
 */
int	CFile1::Open(void)
{
	if (fp != NULL)
		return 1;

	char	*filepath = Ccct::Ucs2ToMbcs(name, CHARSET_DEFAULT);
	if (filepath == NULL)
		return 0;

	fp = w_fopen((wchar_t *)name.c_str(), (wchar_t *)mode.c_str());
	free(filepath);
	
	return (fp != NULL) ? 1 : 0;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::Close
 *  機能概要：  ファイルをクローズします
 *
 *  返値　　：　1/2=成功/ロードされていない、もしくは既にunloadされている
 * -----------------------------------------------------------------------
 */
int	CFile1::Close(void)
{
	if (fp == NULL)
		return 2;

	fclose(fp);

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::Write
 *  機能概要：  ファイルに文字列を書き込みます
 *
 *  返値　　：　0/1=失敗/成功
 * -----------------------------------------------------------------------
 */
int	CFile1::Write(const wstring &istr)
{
	if (fp == NULL)
		return 0;

	// 文字列をマルチバイト文字コードに変換
	char	*t_istr = Ccct::Ucs2ToMbcs(istr, charset);
	if (t_istr == NULL)
		return 0;

	long	len = (long)strlen(t_istr);

	// 書き込み
	fwrite(t_istr, sizeof(char), len, fp);
	free(t_istr);

	return 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CFile1::Read
 *  機能概要：  ファイルから文字列を1行読み取ります
 *
 *  返値　　：　-1/0/1=EOF/失敗/成功
 * -----------------------------------------------------------------------
 */
int	CFile1::Read(wstring &ostr)
{
	ostr = L"";

	if (fp == NULL)
		return 0;

	if (ws_fgets(ostr, fp, charset, 0, bomcheck) == WS_EOF)
		return -1;

	bomcheck++;

	return 1;
}
