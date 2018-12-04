// 
// AYA version 5
//
// 重複回避制御を行なうクラス　CDuplEvInfo
// - 主処理部
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "selecter.h"
#include "sysfunc.h"
#include "value.h"
#include "misc.h"
#include "log.h"
#include "wsex.h"
#include "mt19937ar.h"

extern CSystemFunction	sysfunction;
extern CLog				logger;

/* -----------------------------------------------------------------------
 *  関数名  ：  CDuplEvInfo::Choice
 *  機能概要：  候補から選択して出力します
 * -----------------------------------------------------------------------
 */
CValue	CDuplEvInfo::Choice(int areanum, const vector<CVecValue> &values, int mode)
{
	// 領域毎の候補数と総数を更新　変化があった場合は巡回順序を初期化する
	if (UpdateNums(areanum, values))
		InitRoundOrder(mode);

	// 値の取得と巡回制御
	CValue	result = GetValue(areanum, values);

	// 巡回位置を進める　巡回が完了したら巡回順序を初期化する
	index++;
	if (index >= total)
		InitRoundOrder(mode);

	return result;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CDuplEvInfo::InitRoundOrder
 *  機能概要：  巡回順序を初期化します
 * -----------------------------------------------------------------------
 */
void	CDuplEvInfo::InitRoundOrder(int mode)
{
	// 初期化
	index = 0;
	roundorder.clear();
	int	t_total = total;

	for(int i = 0; i < t_total; i++)
		roundorder.push_back(i);

	// nonoverlapの場合は巡回順序をかき混ぜる
	// ランダムに選んでスワップ、を(キー数/2)回繰り返すという途方もなく原始的な方法
	if (mode == CHOICETYPE_NONOVERLAP) {
		int	shufflenum = (total + 1) >> 1;
		for(int i = 0; i < shufflenum; i++) {
			int i0 = (int)(genrand_real2()*(double)t_total);
			int i1 = (int)(genrand_real2()*(double)t_total);
		    
			if (i0 != i1)
				exchange(roundorder[i0], roundorder[i1]);
		}
	}

}

/* -----------------------------------------------------------------------
 *  関数名  ：  CDuplEvInfo::UpdateNums
 *  機能概要：  領域毎の候補数と総数を更新します
 *  返値　　　  0/1=変化なし/あり
 * -----------------------------------------------------------------------
 */
char	CDuplEvInfo::UpdateNums(int areanum, const vector<CVecValue> &values)
{
	// 元の候補数を保存しておく
	vector<int>	bef_num(num.begin(), num.end());
	int	bef_numlenm1 = bef_num.size() - 1;

	// 領域毎の候補数と組み合わせ総数を更新
	// 候補数に変化があった場合はフラグに記録する
	num.clear();
	total = 1;
	char	changed = (areanum != bef_numlenm1) ? 1 : 0;
	for(int i = 0; i <= areanum; i++) {
		int	t_num = values[i].array.size();
		num.push_back(t_num);
		total *= t_num;
		if (i <= bef_numlenm1)
			if (bef_num[i] != t_num)
				changed = 1;
	}

	return changed;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CDuplEvInfo::GetValue
 *  機能概要：  現在の巡回位置から値を取得します
 *
 *  格納領域が一つしかない場合はそれをそのまま出すので値の型が保護されます。
 *  領域が複数ある場合はそれらは文字列として結合されますので、文字列型での出力となります。
 * -----------------------------------------------------------------------
 */
CValue	CDuplEvInfo::GetValue(int areanum, const vector<CVecValue> &values)
{
	int	t_index = roundorder[index];

	sysfunction.SetLso(t_index);

	if (areanum) {
		wstring	result;
		for(int i = 0; i <= areanum; i++) {
			int	next = t_index/num[i];
			result += values[i].array[t_index - next*(num[i])].GetValueString();
			t_index = next;
		}
		return CValue(result);
	}
	else
		return values[0].array[t_index];
}
