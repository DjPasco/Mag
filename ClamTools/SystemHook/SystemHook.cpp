#include "stdafx.h"
#include "SystemHook.h"

#include "../Utils/SendObj.h"
#include "../Utils/HookUtils.h"
#include "../Utils/Registry.h"

#include "detours.h"

#include <stdio.h>
#include <tchar.h>

namespace wnd_utils
{
	//Send file to scan server
	static bool Execute(LPCSTR sFile)
	{
		HWND hwnd = NULL;
		hwnd = FindWindow(NULL, sgServerName);

		if(NULL == hwnd)
		{
			return true;
		}

		CSendObj obj;
		strcpy_s(obj.m_sPath, MAX_PATH, sFile);
		obj.m_nType = EScan;
		obj.m_bUseInternalDB = true;
		obj.m_PID = GetCurrentProcessId();

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

		if(NULL == strstr(sPath, "\\\\"))//Named Pipe
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
		if(NULL == strstr(lpFileName, "\\\\"))//Pipe, disc or device. No scan required.
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

SYSTEM_HOOK_API void DoMagic(){ }