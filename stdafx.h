// stdafx.h : 標準のシステム インクルード ファイル、
//            または参照回数が多く、かつあまり変更されない
//            プロジェクト専用のインクルード ファイルを記述します。
//

#if !defined(AFX_STDAFX_H__308D3A4E_B685_4328_B501_D274F42D9159__INCLUDED_)
#define AFX_STDAFX_H__308D3A4E_B685_4328_B501_D274F42D9159__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// STL使用時にデバッグビルドで発生する無駄なワーニングを抑止
#pragma warning (disable: 4786)

#define WIN32_LEAN_AND_MEAN		// Windows ヘッダーから殆ど使用されないスタッフを除外します

#include <windows.h>
#include <winreg.h>
#include <Winperf.h>

#include <mbstring.h>
#include <wchar.h>
#include "winbase.h"
#include <stdarg.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include <math.h>
#include "io.h"
#include "direct.h"
#include <locale.h>
#include <winnls.h>

#include <locale>
#include <vector>
#include <list>
#include <string>
#include <map>
using namespace std;

#include "manifest.h"
#include "messages.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_STDAFX_H__308D3A4E_B685_4328_B501_D274F42D9159__INCLUDED_)
