// 
// AYA version 5
//
// written by umeici. 2004
// 

#ifndef POSIX
#  include "stdafx.h"
#endif
#include "aya5.h"
#include "basis.h"
#include "sysfunc.h"
#include "lib.h"
#include "file.h"
#include "log.h"
#include "misc.h"
#include "wordmatch.h"
#include "mt19937ar.h"

// ���Z�q�E�֐��̌����}�b�v
CWordMatch			formulatag_wm;
CWordMatch			function_wm;

// �吧��
CBasis				basis;

// �֐�/�V�X�e���֐�/�O���[�o���ϐ�
vector<CFunction>	function;
CCallDepth			calldepth;
CSystemFunction		sysfunction;
CGlobalVariable		variable;

// �t�@�C���ƊO�����C�u����
CFile				files;
CLib				libs;

// ���K�[
CLog				logger;


/* -----------------------------------------------------------------------
 *  DllMain
 * -----------------------------------------------------------------------
 */
void	fundamental_init(void)
{
	init_genrand(static_cast<unsigned long>(time(NULL)));
//	srand((unsigned)time(NULL));
}

#ifndef POSIX
BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		basis.SetModuleHandle(hModule);
		fundamental_init();
	}

	return TRUE;
}
#endif

/* -----------------------------------------------------------------------
 *  load
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
extern "C" __declspec(dllexport) BOOL __cdecl load(HGLOBAL h, long len)
{
	basis.SetPath(h, len);
	basis.Configure();

	return TRUE;
}
#else
extern "C" int load(char* h, long len) {
	init_genrand(static_cast<unsigned long>(time(NULL)));
//    srand((unsigned)time(NULL));
    basis.SetPath(h, len);
    basis.Configure();
    
    return 1;
}
#endif

/* -----------------------------------------------------------------------
 *  unload
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
extern "C" __declspec(dllexport) BOOL __cdecl unload()
{
	basis.Termination();

	return TRUE;
}
#else
extern "C" int unload() {
    basis.Termination();
    return 1;
}
#endif

/* -----------------------------------------------------------------------
 *  request
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
extern "C" __declspec(dllexport) HGLOBAL __cdecl request(HGLOBAL h, long *len)
{
	return basis.ExecuteRequest(h, len);
}
#else
extern "C" char* request(char* h, long* len) {
    return basis.ExecuteRequest(h, len);
}
#endif

/* -----------------------------------------------------------------------
 *  logsend�iAYA�ŗL�@�`�F�b�N�c�[������g�p�j
 * -----------------------------------------------------------------------
 */
#ifndef POSIX
extern "C" __declspec(dllexport) BOOL __cdecl logsend(long hwnd)
{
	basis.SetLogRcvWnd(hwnd);

	return TRUE;
}
#endif
