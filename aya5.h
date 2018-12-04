// 
// AYA version 5
//
// written by umeici. 2004
// 

#ifndef	AYA5H
#define	AYA5H

//----

#ifndef POSIX

extern "C" __declspec(dllexport) BOOL __cdecl load(HGLOBAL h, long len);
extern "C" __declspec(dllexport) BOOL __cdecl unload();
extern "C" __declspec(dllexport) HGLOBAL __cdecl request(HGLOBAL h, long *len);

extern "C" __declspec(dllexport) BOOL __cdecl logsend(long hwnd);

#else

extern "C" int load(char* h, long len);
extern "C" int unload();
extern "C" char* request(char* h, long *len);
// logsendはとりあえず無し。

#endif

//----

#endif
