#include "stdafx.h"
#include "SystemHook.h"

#include "../Utils/SendObj.h"

#include <stdio.h>
#include <tchar.h>

namespace wnd_utils
{
	static bool Execute(LPCSTR sFile)
	{
		HWND hwnd = NULL;
		hwnd = FindWindow(NULL, "DCAntiVirusScan");

		if(NULL != hwnd)
		{
			return true;
		}
		CSendObj obj;
		strcpy_s(obj.m_sPath, MAX_PATH, sFile);
		obj.m_bReQuestData = false;

		COPYDATASTRUCT copy;
		copy.dwData = 1;
		copy.cbData = sizeof(obj);
		copy.lpData = &obj;

		LRESULT result = SendMessage(hwnd,
									 WM_COPYDATA,
									 0,
									 (LPARAM) (LPVOID) &copy);

		return true;
	}
};

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
		//MessageBox(NULL, sPath, "Dydis", MB_OKCANCEL);	
		bool bFileOK = wnd_utils::Execute(sPath);
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
	BOOL bRet =  pTrueCreateProcessW(lpszImageName, lpszCmdLine, lpsaProcess,
							   lpsaThread, fInheritHandles, fdwCreate,
							   lpvEnvironment, lpszCurDir, lpsiStartInfo, lppiProcInfo);

	//if(bRet)
	//{
	//	char sDetoursPath[MAX_PATH];
	//	path_utils::GetDetourDllPath(sDetoursPath);

	//	//hook_utils_main::DebugMessage(sDetoursPath);

	//	char sHookPath[MAX_PATH];
	//	path_utils::GetHookDllPath(sHookPath);

	//	LPVOID LoadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), _T("LoadLibraryW"));

	//	hook_utils_main::RunLoadLibraryInProcess(lppiProcInfo->hProcess, LoadLibraryAddr, sDetoursPath);
	//	//hook_utils_main::DebugMessage("Deours");

	//	hook_utils_main::RunLoadLibraryInProcess(lppiProcInfo->hProcess, LoadLibraryAddr, sHookPath);
	//	//hook_utils_main::DebugMessage("hook");
	//}

	return bRet;

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
	return pTrueCreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes,
							   lpThreadAttributes, bInheritHandles, dwCreate,
							   lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
}


SYSTEM_HOOK_API void DoMagic(){ }