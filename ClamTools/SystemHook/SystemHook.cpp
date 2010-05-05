#include "stdafx.h"
#include "SystemHook.h"
#include "DCComunication/DCComunication.h"

#include "../../detours/include/detours.h"

#include <stdio.h>

namespace path_utils
{
	void GetHookDllPath(char *sHookPath)
	{
		char dirPath[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, dirPath);

	#ifdef _DEBUG
		sprintf_s(sHookPath, MAX_PATH, "%s\\SystemHookD.dll", dirPath);
	#else
		sprintf_s(sHookPath, MAX_PATH, "%s\\SystemHook.dll", dirPath);
	#endif
	}

	void GetDetourDllPath(char *sDetourPath)
	{
		char dirPath[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, dirPath);

		sprintf_s(sDetourPath, MAX_PATH, "%s\\detoured.dll", dirPath);
	}
}

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
	return pTrueCreateFileA(lpFileName,
					        dwDesiredAccess,
					   dwShareMode,
					   lpSecurityAttributes,
					   dwCreationDisposition,
					   dwFlagsAndAttributes,
					   hTemplateFile);
};

extern BOOL (WINAPI *pTrueCreateProcessW)(LPCWSTR lpszImageName,
								   LPWSTR lpszCmdLine,
								   LPSECURITY_ATTRIBUTES lpsaProcess,
								   LPSECURITY_ATTRIBUTES lpsaThread,
								   BOOL fInheritHandles,
								   DWORD fdwCreate,
								   LPVOID lpvEnvironment,
								   LPCWSTR lpszCurDir,
								   LPSTARTUPINFOW lpsiStartInfo,
								   LPPROCESS_INFORMATION lppiProcInfo); 

BOOL WINAPI TransCreateProcessW(LPCWSTR lpszImageName,
								LPWSTR lpszCmdLine,
								LPSECURITY_ATTRIBUTES lpsaProcess,
								LPSECURITY_ATTRIBUTES lpsaThread,
								BOOL fInheritHandles,
								DWORD fdwCreate,
								LPVOID lpvEnvironment,
								LPCWSTR lpszCurDir,
								LPSTARTUPINFOW lpsiStartInfo,
								LPPROCESS_INFORMATION lppiProcInfo)
{
	MessageBox(NULL, "W", "w", MB_ICONHAND);

	char sHookPath[MAX_PATH];
	path_utils::GetHookDllPath(sHookPath);

	char sFullDetoursPath[MAX_PATH];
	path_utils::GetDetourDllPath(sFullDetoursPath);

	return DetourCreateProcessWithDllW(lpszImageName, lpszCmdLine, lpsaProcess,
							   lpsaThread, fInheritHandles, fdwCreate,
							   lpvEnvironment, lpszCurDir, lpsiStartInfo, lppiProcInfo,
							   sFullDetoursPath, sHookPath, NULL);

}

extern BOOL (WINAPI *pTrueCreateProcessA)(LPCSTR lpApplicationName,
								   LPSTR lpCommandLine,
								   LPSECURITY_ATTRIBUTES lpProcessAttributes,
								   LPSECURITY_ATTRIBUTES lpThreadAttributes,
								   BOOL bInheritHandles,
								   DWORD dwCreate,
								   LPVOID lpEnvironment,
								   LPCSTR lpCurrentDirectory,
								   LPSTARTUPINFOA lpStartupInfo,
								   LPPROCESS_INFORMATION lpProcessInformation); 

BOOL WINAPI TransCreateProcessA(LPCSTR lpApplicationName,
								   LPSTR lpCommandLine,
								   LPSECURITY_ATTRIBUTES lpProcessAttributes,
								   LPSECURITY_ATTRIBUTES lpThreadAttributes,
								   BOOL bInheritHandles,
								   DWORD dwCreate,
								   LPVOID lpEnvironment,
								   LPCSTR lpCurrentDirectory,
								   LPSTARTUPINFOA lpStartupInfo,
								   LPPROCESS_INFORMATION lpProcessInformation)
{
	MessageBox(NULL, "A", "a", MB_ICONHAND);
	char sHookPath[MAX_PATH];
	path_utils::GetHookDllPath(sHookPath);

	char sFullDetoursPath[MAX_PATH];
	path_utils::GetDetourDllPath(sFullDetoursPath);

	return DetourCreateProcessWithDllA(lpApplicationName, lpCommandLine, lpProcessAttributes,
							   lpThreadAttributes, bInheritHandles, dwCreate,
							   lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation,
							   sFullDetoursPath, sHookPath, NULL);
	//return 
	//return pTrueCreateProcessA(lpApplicationName,
	//							   lpCommandLine,
	//							   lpProcessAttributes,
	//							   lpThreadAttributes,
	//							   bInheritHandles,
	//							   dwCreate,
	//							   lpEnvironment,
	//							   lpCurrentDirectory,
	//							   lpStartupInfo,
	//							   lpProcessInformation);
}


SYSTEM_HOOK_API void DoMagic(){ }