#include "stdafx.h"
#include "SystemHook.h"
#include "DCComunication/DCComunication.h"

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
	char sPath[MAX_PATH];
	WideCharToMultiByte( CP_ACP, 0, lpFileName, -1, sPath, MAX_PATH,NULL,NULL);

	if(NULL == strstr(sPath, "\\\\.\\"))//Named Pipe
	{
		bool bFileOK = CDCClient::Execute(lpFileName);
	}

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
	return CreateFileA(lpFileName,
					   dwDesiredAccess,
					   dwShareMode,
					   lpSecurityAttributes,
					   dwCreationDisposition,
					   dwFlagsAndAttributes,
					   hTemplateFile);
};

SYSTEM_HOOK_API void DoMagic(){ }