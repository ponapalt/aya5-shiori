// 
// AYA version 5
//
// 主な制御を行なうクラス　CBasis
// written by umeici. 2004
// 

#ifndef	BASISH
#define	BASISH

//----

#ifndef POSIX
#  include "stdafx.h"
#else
#  include <string>
using namespace std;
#endif
#include "variable.h"
#include "function.h"
#include "value.h"

class	CBasis
{
public:
	wstring	path;					// 上位モジュールからload時に渡されるパス

protected:
	char	msglang;				// ログに記録するメッセージの言語（日/英）
	char	charset;				// 文字コードセット（Shift_JIS/UTF-8/OSデフォルト）
#ifndef POSIX
	HWND	hlogrcvWnd;				// チェックツールのhWnd
	HMODULE	hmodule;				// モジュールのハンドル
#endif
	wstring	logpath;				// ログファイルのパス
	wstring	modulename;				// モジュールの主ファイル名（例えばa.dllの場合は"a"）
	char	surpress;				// 自律動作抑止（構文解析でエラーが起きた際に動作をマスクするために使用します）
	char	checkparser;			// 構文解析結果のログへの記録を指示するフラグ
	char	iolog;					// 入出力のログへの記録を指示するフラグ

	int	loadindex;					// 関数 load の位置
	int	unloadindex;				// 関数 unload の位置
	int	requestindex;				// 関数 request の位置

	char	run;					// load完了で0→1へ

public:
	CBasis(void);
	~CBasis(void) {}
	
	void CBasis::CreateWordMatch(void);

	char	IsRun(void) { return run; }

#ifndef POSIX
	void	SetModuleHandle(HANDLE h);
	void	SetPath(HGLOBAL h, int len);
	void	SetLogRcvWnd(long hwnd);
#else
	void    SetPath(char* h, long len);
#endif

	void	Configure(void);
	void	Termination(void);
	
#ifndef POSIX
	HWND	GetLogHWnd(void) { return hlogrcvWnd; }
#endif
	wstring	GetLogPath(void)  { return logpath; }
	char	GetMsgLang(void)  { return msglang; }
	char	GetCharset(void)  { return charset; }
	wstring	GetRootPath(void) { return path;    }

	void	ExecuteLoad(void);
#ifndef POSIX
	HGLOBAL	ExecuteRequest(HGLOBAL h, long *len);
#else
	char*   ExecuteRequest(char* h, long* len);
#endif
	void	ExecuteUnload(void);

	void	SaveVariable(void);

protected:
	char	IsSurpress(void);
	void	SetSurpress(void);
	void	ResetSurpress(void);

	void	LoadBaseConfigureFile(vector<wstring> *dics);
	void	SetParameter(wstring &cmd, wstring &param, vector<wstring> *dics);

	void	RestoreVariable(void);
	void	RestoreArrayVariable(CValue &var, wstring &value);
};

//----

#endif
