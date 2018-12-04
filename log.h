// 
// AYA version 5
//
// ロギング用クラス　CLog
// written by umeici. 2004
// 

#ifndef	LOGGERH
#define	LOGGERH

//----

#ifndef POSIX
#  include "stdafx.h"
#endif

#define	CLASSNAME_CHECKTOOL	"TamaWndClass"	/* チェックツールのウィンドウクラス名 */

//----

class	CLog
{
protected:
	wstring		path;		// ログファイルのパス
	int			charset;	// 文字コードセット
	int			msglang;	// メッセージの言語
#ifndef POSIX
	HWND		hWnd;		// チェックツールのHWND
#endif

	char		enable;		// ロギング有効フラグ
	char		fileen;		// ファイルへのロギング有効フラグ
	char		iolog;		// 入出力ロギング有効フラグ

public:
	CLog(void)
	{
		enable = 1;
		fileen = 1;
		iolog  = 1;
	}

#ifndef POSIX
	void	Start(const wstring &p, int cs, int ml, HWND hw, char il);
#else
	void	Start(const wstring &p, int cs, int ml, char il);
#endif
	void	Termination(void);

	void	Write(const wstring &str, int mode = 0);
	void	Write(const wchar_t *str, int mode = 0);

	void	Message(int id);
	void	Filename(const wstring &filename);

	void	Error(int mode, int id, const wchar_t *ref, const wstring &dicfilename, int linecount);
	void	Error(int mode, int id, const wstring &ref, const wstring &dicfilename, int linecount);
	void	Error(int mode, int id, const wchar_t *ref);
	void	Error(int mode, int id, const wstring &ref);
	void	Error(int mode, int id, const wstring &dicfilename, int linecount);
	void	Error(int mode, int id);

	void	Io(char io, const wstring &str);
	void	IoLib(char io, const wstring &str, const wstring &name);

	void	SendLogToWnd(const wchar_t *str, int mode);
	void	SendLogToWnd(const wstring &str, int mode);

protected:
#ifndef POSIX
	HWND	GetCheckerWnd(void);
#endif

};

//----

#endif
