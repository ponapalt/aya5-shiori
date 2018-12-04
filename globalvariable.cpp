// 
// AYA version 5
//
// グローバル変数を扱うクラス　CGlobalVariable
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "basis.h"
#include "variable.h"
#include "value.h"
#include "misc.h"
#include "log.h"
#include "wsex.h"
#include "ccct.h"

extern CBasis	basis;

/* -----------------------------------------------------------------------
 *  関数名  ：  CGlobalVariable::Make
 *  機能概要：  変数を作成します
 *  引数　　：  erased 0/1=有効状態/消去状態
 *
 *  返値　　：  追加した位置
 *
 *  指定された名前の変数が既に存在していた場合は追加は行わず、既存の位置を返します
 * -----------------------------------------------------------------------
 */
int	CGlobalVariable::Make(const wstring &name, char erased)
{
	// 既に存在するか調べ、存在していたらそれを返す
	int	i = 0;
	for(vector<CVariable>::iterator it = var.begin(); it != var.end(); it++, i++)
		if (!name.compare(it->name)) {
			if (!basis.IsRun()) {
				if (erased)
					it->Erase();
				else
					it->Enable();
			}
		return i;
		}

	// 作成
	CVariable	addvariable(name);
	if (erased)
		addvariable.Erase();
	else
		addvariable.Enable();
	var.push_back(addvariable);

	return var.size() - 1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CGlobalVariable::GetMacthedLongestNameLength
 *  機能概要：  指定された文字列にマッチする名前を持つ変数を探し、マッチした長さを返します
 *
 *  複数見つかった場合は最長のものを返します。見つからなかった場合は0を返します
 * -----------------------------------------------------------------------
 */
int	CGlobalVariable::GetMacthedLongestNameLength(const wstring &name)
{
	int	max_len = 0;

	for(vector<CVariable>::iterator it = var.begin(); it != var.end(); it++) {
		int	len = it->name.size();
		if (!it->IsErased() && max_len < len && !it->name.compare(name.substr(0, len)))
			max_len = len;
	}

	return max_len;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CGlobalVariable::GetIndex
 *  機能概要：  指定された名前の変数の位置を返します
 *
 *  見つからなかった場合は-1を返します
 * -----------------------------------------------------------------------
 */
int		CGlobalVariable::GetIndex(const wstring &name)
{
	int	i = 0;
	for(vector<CVariable>::iterator it = var.begin(); it != var.end(); it++, i++)
		if (!name.compare(it->name))
			return (it->IsErased()) ? -1 : i;

	return -1;
}
