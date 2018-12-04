// 
// AYA version 5
//
// ���M���O�p�N���X�@CLog
// written by umeici. 2004
// 

#ifndef	LOGGERH
#define	LOGGERH

//----

#ifndef POSIX
#  include "stdafx.h"
#endif

#define	CLASSNAME_CHECKTOOL	"TamaWndClass"	/* �`�F�b�N�c�[���̃E�B���h�E�N���X�� */

//----

class	CLog
{
protected:
	wstring		path;		// ���O�t�@�C���̃p�X
	int			charset;	// �����R�[�h�Z�b�g
	int			msglang;	// ���b�Z�[�W�̌���
#ifndef POSIX
	HWND		hWnd;		// �`�F�b�N�c�[����HWND
#endif

	char		enable;		// ���M���O�L���t���O
	char		fileen;		// �t�@�C���ւ̃��M���O�L���t���O
	char		iolog;		// ���o�̓��M���O�L���t���O

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
