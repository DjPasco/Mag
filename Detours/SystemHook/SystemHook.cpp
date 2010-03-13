#include "stdafx.h"
#include "../detours/include/detours.h"
#include <tchar.h>
#include <stdio.h>
#include "SystemHook.h"
#include <Winsock2.h>

extern HANDLE (WINAPI * pTrueCreateFileW)(LPCWSTR lpFileName,
										  DWORD dwDesiredAccess,
										  DWORD dwShareMode,
										  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
										  DWORD dwCreationDisposition,
										  DWORD dwFlagsAndAttributes,
										  HANDLE hTemplateFile);

HANDLE WINAPI TransCreateFileW(LPCWSTR lpFileName,
							   DWORD dwDesiredAccess,
							   DWORD dwShareMode,
							   LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							   DWORD dwCreationDisposition,
							   DWORD dwFlagsAndAttributes,
							   HANDLE hTemplateFile)
{
	//MessageBox(NULL, (LPCWSTR)"lala", (LPCWSTR)"lala", MB_OK);

 	//socket_utils::SendFile(lpFileName);

    return pTrueCreateFileW(lpFileName,
					   dwDesiredAccess,
					   dwShareMode,
					   lpSecurityAttributes,
					   dwCreationDisposition,
					   dwFlagsAndAttributes,
					   hTemplateFile);
};

extern HANDLE (WINAPI * pTrueCreateFileA)(LPCSTR lpFileName,
										  DWORD dwDesiredAccess,
										  DWORD dwShareMode,
										  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
										  DWORD dwCreationDisposition,
										  DWORD dwFlagsAndAttributes,
										  HANDLE hTemplateFile);

HANDLE WINAPI TransCreateFileA(LPCSTR lpFileName,
							   DWORD dwDesiredAccess,
							   DWORD dwShareMode,
							   LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							   DWORD dwCreationDisposition,
							   DWORD dwFlagsAndAttributes,
							   HANDLE hTemplateFile)
{
	MessageBox(NULL, (LPCWSTR)"lala", (LPCWSTR)"lala", MB_OK);

    return CreateFileA(lpFileName,
					   dwDesiredAccess,
					   dwShareMode,
					   lpSecurityAttributes,
					   dwCreationDisposition,
					   dwFlagsAndAttributes,
					   hTemplateFile);
};

SYSTEM_HOOK_API void DoNothing()
{
	//
}

SYSTEM_HOOK_API void DoSomething()
{
	//
}