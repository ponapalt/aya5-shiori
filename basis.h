// 
// AYA version 5
//
// ��Ȑ�����s�Ȃ��N���X�@CBasis
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
	wstring	path;					// ��ʃ��W���[������load���ɓn�����p�X

protected:
	char	msglang;				// ���O�ɋL�^���郁�b�Z�[�W�̌���i��/�p�j
	char	charset;				// �����R�[�h�Z�b�g�iShift_JIS/UTF-8/OS�f�t�H���g�j
#ifndef POSIX
	HWND	hlogrcvWnd;				// �`�F�b�N�c�[����hWnd
	HMODULE	hmodule;				// ���W���[���̃n���h��
#endif
	wstring	logpath;				// ���O�t�@�C���̃p�X
	wstring	modulename;				// ���W���[���̎�t�@�C�����i�Ⴆ��a.dll�̏ꍇ��"a"�j
	char	surpress;				// ��������}�~�i�\����͂ŃG���[���N�����ۂɓ�����}�X�N���邽�߂Ɏg�p���܂��j
	char	checkparser;			// �\����͌��ʂ̃��O�ւ̋L�^���w������t���O
	char	iolog;					// ���o�͂̃��O�ւ̋L�^���w������t���O

	int	loadindex;					// �֐� load �̈ʒu
	int	unloadindex;				// �֐� unload �̈ʒu
	int	requestindex;				// �֐� request �̈ʒu

	char	run;					// load������0��1��

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
