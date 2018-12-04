// 
// AYA version 5
//
// ローカル変数を扱うクラス　CLocalVariable
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "variable.h"
#include "value.h"
#include "misc.h"
#include "log.h"
#include "wsex.h"
#include "ccct.h"

extern CLog	logger;

/* -----------------------------------------------------------------------
 *  CLocalVariableコンストラクタ
 * -----------------------------------------------------------------------
 */
CLocalVariable::CLocalVariable(void)
{
	depth = -1;

	AddDepth();
}

/* -----------------------------------------------------------------------
 *  CLocalVariableデストラクタ
 * -----------------------------------------------------------------------
 */
CLocalVariable::~CLocalVariable(void)
{
	stack.clear();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLocalVariable::GetArgvAdr
 *  機能概要：  _argvが格納されている位置のポインタを返します
 *
 *  vectorは再割り当てによってアドレスが変化するので、ここで得られる値は
 *  一時的に利用することしか出来ません。
 * -----------------------------------------------------------------------
 */
CVariable	*CLocalVariable::GetArgvPtr(void)
{
	return &(stack[0].substack[0]);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLocalVariable::AddDepth
 *  機能概要：  一階層深いローカル変数スタックを作成します
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::AddDepth(void)
{
	CLVSubStack	addsubstack;
	stack.push_back(addsubstack);
	depth++;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLocalVariable::DelDepth
 *  機能概要：  最も深いローカル変数スタックを破棄します
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::DelDepth(void)
{
	if (stack.size()) {
		stack.erase(stack.end() - 1);
		depth--;
	}
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLocalVariable::Make
 *  機能概要：  ローカル変数を作成します
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::Make(const wchar_t *name)
{
	CVariable	addlv(name);
	stack[depth].substack.push_back(addlv);
}

//----

void	CLocalVariable::Make(const wstring &name)
{
	CVariable	addlv(name);
	stack[depth].substack.push_back(addlv);
}

//----

void	CLocalVariable::Make(const wchar_t *name, const CValue &value)
{
	CVariable	addlv(name);
	addlv.v = value;
	stack[depth].substack.push_back(addlv);
}

//----

void	CLocalVariable::Make(const wstring &name, const CValue &value)
{
	CVariable	addlv(name);
	addlv.v = value;
	stack[depth].substack.push_back(addlv);
}

//----

void	CLocalVariable::Make(const wstring &name, const CValueSub &value)
{
	CVariable	addlv(name);
	addlv.v = value;
	stack[depth].substack.push_back(addlv);
}

//----

void	CLocalVariable::Make(const wchar_t *name, const wstring &delimiter)
{
	CVariable	addlv(name);
	addlv.delimiter = delimiter;
	stack[depth].substack.push_back(addlv);
}

//----

void	CLocalVariable::Make(const wstring &name, const wstring &delimiter)
{
	CVariable	addlv(name);
	addlv.delimiter = delimiter;
	stack[depth].substack.push_back(addlv);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLocalVariable::GetIndex
 *  機能概要：  指定された名前のローカル変数をスタックから検索します
 *
 *  見つからなかった場合は-1を返します
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::GetIndex(const wchar_t *name, int &id, int &dp)
{
	for(int i = stack.size() - 1; i >= 0; i--)
		for(int j = stack[i].substack.size() - 1; j >= 0; j--)
			if (!stack[i].substack[j].name.compare(name)) {
				if (stack[i].substack[j].IsErased()) {
					id = -1;
					dp = -1;
				}
				else {
					dp = i;
					id = j;
				}
				return;
			}

	id = -1;
	dp = -1;
}

//----

void	CLocalVariable::GetIndex(const wstring &name, int &id, int &dp)
{
	for(int i = stack.size() - 1; i >= 0; i--)
		for(int j = stack[i].substack.size() - 1; j >= 0; j--)
			if (!stack[i].substack[j].name.compare(name)) {
				if (stack[i].substack[j].IsErased()) {
					id = -1;
					dp = -1;
				}
				else {
					dp = i;
					id = j;
				}
				return;
			}

	id = -1;
	dp = -1;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLocalVariable::GetValue
 *  機能概要：  指定されたローカル変数の値を取得します
 *
 *  指定された変数が存在しなかった場合は空文字列が返ります。
 * -----------------------------------------------------------------------
 */

CValue	CLocalVariable::GetValue(const wchar_t *name)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0)
		return stack[dp].substack[id].v;

	return CValue();
}

//----

CValue	CLocalVariable::GetValue(const wstring &name)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0)
		return stack[dp].substack[id].v;

	return CValue();
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLocalVariable::GetDelimiter
 *  機能概要：  指定されたローカル変数のデリミタを取得します
 * -----------------------------------------------------------------------
 */
wstring	CLocalVariable::GetDelimiter(const wchar_t *name)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0)
		return stack[dp].substack[id].delimiter;

	return wstring(L"");
}

//----

wstring	CLocalVariable::GetDelimiter(const wstring &name)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0)
		return stack[dp].substack[id].delimiter;

	return wstring(L"");
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLocalVariable::SetDelimiter
 *  機能概要：  指定されたローカル変数に値を格納します
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::SetDelimiter(const wchar_t *name, const wstring &value)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0) {
		stack[dp].substack[id].delimiter = value;
		return;
	}

	// スタック内に存在しなければ変数を新たに作成して代入
	Make(name, value);
}

//----

void	CLocalVariable::SetDelimiter(const wstring &name, const wstring &value)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0) {
		stack[dp].substack[id].delimiter = value;
		return;
	}

	// スタック内に存在しなければ変数を新たに作成して代入
	Make(name, value);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLocalVariable::SetValue
 *  機能概要：  指定されたローカル変数に値を格納します
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::SetValue(const wchar_t *name, const CValue &value)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0) {
		stack[dp].substack[id].v = value;
		return;
	}

	// スタック内に存在しなければ変数を新たに作成して代入
	Make(name, value);
}

//----

void	CLocalVariable::SetValue(const wstring &name, const CValue &value)
{
	int	dp, id;
	GetIndex(name, id, dp);
	if (id >= 0) {
		stack[dp].substack[id].v = value;
		return;
	}

	// スタック内に存在しなければ変数を新たに作成して代入
	Make(name, value);
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLocalVariable::GetMacthedLongestNameLength
 *  機能概要：  指定された文字列にマッチする名前を持つ変数を探し、マッチした長さを返します
 *
 *  複数見つかった場合は最長のものを返します。見つからなかった場合は0を返します
 * -----------------------------------------------------------------------
 */
int	CLocalVariable::GetMacthedLongestNameLength(const wstring &name)
{
	int	max_len = 0;

	for(int i = stack.size() - 1; i >= 0; i--)
		for(int j = stack[i].substack.size() - 1; j >= 0; j--) {
			int	len = stack[i].substack[j].name.size();
			if (!stack[i].substack[j].IsErased() &&
				max_len < len &&
				!stack[i].substack[j].name.compare(name.substr(0, len)))
				max_len = len;
		}

	return max_len;
}

/* -----------------------------------------------------------------------
 *  関数名  ：  CLocalVariable::Erase
 *  機能概要：  指定された変数を消去します
 *
 *  実際に配列から消すわけではなく、空文字列を代入するだけです
 * -----------------------------------------------------------------------
 */
void	CLocalVariable::Erase(const wstring &name)
{
	int	id, dp;
	GetIndex(name, id, dp);
	if (id >= 0)
		SetValue(name, CValue());
}
