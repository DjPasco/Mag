#include "stdafx.h"
#include "SystemHook.h"

#include "../Utils/SendObj.h"
#include "../Utils/HookUtils.h"

#include "../../detours/include/detours.h"

#include <stdio.h>
#include <tchar.h>

namespace wnd_utils
{
	static bool Execute(LPCSTR sFile)
	{
		HWND hwnd = NULL;
		hwnd = FindWindow(NULL, "DCAntiVirusScan");

		if(NULL == hwnd)
		{
			return true;
		}

		CSendObj obj;
		strcpy_s(obj.m_sPath, MAX_PATH, sFile);
		obj.m_nType = EScan;
		obj.m_bUseInternalDB = true;

		COPYDATASTRUCT copy;
		copy.dwData = 1;
		copy.cbData = sizeof(obj);
		copy.lpData = &obj;

		LRESULT result = SendMessage(hwnd,
									 WM_COPYDATA,
									 0,
									 (LPARAM) (LPVOID) &copy);

		if(2 == result)
		{
			return false;
		}

		return true;
	}

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
};



extern HANDLE (WINAPI * pTrueCreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
										  DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

HANDLE WINAPI TransCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							   DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	if(0 != dwDesiredAccess)
	{
		char sPath[MAX_PATH];
		WideCharToMultiByte( CP_ACP, 0, lpFileName, -1, sPath, MAX_PATH,NULL,NULL);

		if(NULL == strstr(sPath, "\\\\.\\"))//Named Pipe
		{
			if(!wnd_utils::Execute(sPath))
			{
				SetLastError(ERROR_ACCESS_DENIED);
				return INVALID_HANDLE_VALUE;
			}
		}
	}

	return pTrueCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
					   dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
};

extern HANDLE (WINAPI * pTrueCreateFileA)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
										  DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

HANDLE WINAPI TransCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							   DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	if(0 != dwDesiredAccess)
	{
		if(NULL == strstr(lpFileName, "\\\\.\\"))//Named Pipe
		{
			if(!wnd_utils::Execute(lpFileName))
			{
				SetLastError(ERROR_ACCESS_DENIED);
				return INVALID_HANDLE_VALUE;
			}
		}
	}

	return pTrueCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
						    dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
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
	char sHookPath[MAX_PATH];
	wnd_utils::GetHookDllPath(sHookPath);

	char sFullDetoursPath[MAX_PATH];
	wnd_utils::GetDetourDllPath(sFullDetoursPath);
	
	return DetourCreateProcessWithDllW(lpszImageName, lpszCmdLine, lpsaProcess,
								   lpsaThread, fInheritHandles, fdwCreate,
								   lpvEnvironment, lpszCurDir, lpsiStartInfo, lppiProcInfo,
								   sFullDetoursPath, sHookPath, pTrueCreateProcessW);
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

BOOL WINAPI TransCreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
								   LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreate,
								   LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	char sHookPath[MAX_PATH];
	wnd_utils::GetHookDllPath(sHookPath);

	char sFullDetoursPath[MAX_PATH];
	wnd_utils::GetDetourDllPath(sFullDetoursPath);

	return DetourCreateProcessWithDllA(lpApplicationName, lpCommandLine, lpProcessAttributes,
								   lpThreadAttributes, bInheritHandles, dwCreate,
								   lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation,
								   sFullDetoursPath, sHookPath, pTrueCreateProcessA);
}

SYSTEM_HOOK_API void DoMagic(){ }