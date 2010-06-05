/////////////////////////////////////////////////////////////////////////
// RemoteLib.cpp
//-----------------------------------------------------------------------
// Implementation for NT-only RemoteLib APIs.
//-----------------------------------------------------------------------
// Author:
//
// Abin (abinn32@yahoo.com)
// Homepage: http://www.wxjindu.com/abin/
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RemoteLib.h"

BOOL IsWindowsNT();
HANDLE OpenProcessForRemoteExecute(DWORD dwProcessID);
BOOL WriteProcessBytes(HANDLE hProcess, LPVOID lpBaseAddr, LPCVOID lpData, DWORD dwSize);
BOOL RemoteExecute(DWORD dwRemoteProcID, LPTHREAD_START_ROUTINE lpfn, LPCVOID lpszParamString, DWORD dwLen, DWORD& rExitCode, DWORD& rErrorCode);
BOOL RemoteExecute(HANDLE hRemoteProc, LPTHREAD_START_ROUTINE lpfn, LPVOID lpParam, DWORD& rExitCode, DWORD& rErrorCode);

/////////////////////////////////////////////////////////////////////////////////
// Functions Below This Line are for Windows NT Plateform Only!
/////////////////////////////////////////////////////////////////////////////////

BOOL IsWindowsNT()
{
	OSVERSIONINFO osi = { 0 };
	osi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&osi);
	return osi.dwPlatformId == VER_PLATFORM_WIN32_NT;
}

HANDLE OpenProcessForRemoteExecute(DWORD dwProcessID)
{
	return ::OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, dwProcessID);
}

BOOL WriteProcessBytes(HANDLE hProcess, LPVOID lpBaseAddr, LPCVOID lpData, DWORD dwSize)
{
	if (hProcess == NULL || lpBaseAddr == NULL || lpData == NULL || dwSize == 0)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	DWORD dwOld;
	if (!::VirtualProtectEx(hProcess, lpBaseAddr, dwSize, PAGE_READWRITE, &dwOld))
		return FALSE;

	DWORD dwDummy;
	BOOL bOK = ::WriteProcessMemory(hProcess, lpBaseAddr, (LPVOID)lpData, dwSize, &dwDummy);
	DWORD dwError = ::GetLastError();
	::VirtualProtectEx(hProcess, lpBaseAddr, dwSize, dwOld, &dwDummy);
	::SetLastError(dwError);
	return bOK;	
}

BOOL RemoteExecute(DWORD dwRemoteProcID, LPTHREAD_START_ROUTINE lpfn, LPCVOID lpszParamString, DWORD dwLen, DWORD& rExitCode, DWORD& rErrorCode)
{
	rExitCode = 0;
	rErrorCode = ERROR_SUCCESS;

	if (lpfn == NULL)
	{
		rErrorCode = ERROR_INVALID_PARAMETER;
		return FALSE;
	}

	HANDLE hRemoteProc = OpenProcessForRemoteExecute(dwRemoteProcID);
	if (hRemoteProc == NULL)
	{
		rErrorCode = ::GetLastError();
		return FALSE;
	}

	BOOL bOK = TRUE;
	LPVOID lpParam = NULL;

	if (lpszParamString && dwLen)
	{
		lpParam = ::VirtualAllocEx(hRemoteProc, NULL, dwLen, MEM_COMMIT, PAGE_READWRITE);
		if (lpParam == NULL)
		{
			rErrorCode = ::GetLastError();
			::CloseHandle(hRemoteProc);
			return FALSE;		
		}

		bOK = WriteProcessBytes(hRemoteProc, lpParam, lpszParamString, dwLen);
	}
	

	if (bOK)
		bOK = RemoteExecute(hRemoteProc, lpfn, lpParam, rExitCode, rErrorCode);
	else
		rErrorCode = ::GetLastError();

	if (lpszParamString)
	{
		::VirtualFreeEx(hRemoteProc, lpParam, 0, MEM_RELEASE);
		::CloseHandle(hRemoteProc);
	}
	
	return bOK;
}

BOOL RemoteExecute(HANDLE hRemoteProc, LPTHREAD_START_ROUTINE lpfn, LPVOID lpParam, DWORD& rExitCode, DWORD& rErrorCode)
{
	rExitCode = 0;
	rErrorCode = ERROR_SUCCESS;

	if (hRemoteProc == NULL || lpfn == NULL)
	{
		rErrorCode = ERROR_INVALID_PARAMETER;
		return FALSE;	
	}

	HANDLE hThread = ::CreateRemoteThread(hRemoteProc, NULL, NULL, lpfn, lpParam, NULL, NULL);
	if (hThread == NULL)
	{
		rErrorCode = ::GetLastError();
		return FALSE;
	}

	::WaitForSingleObject(hThread, INFINITE);
	::GetExitCodeThread(hThread, &rExitCode);	
	::CloseHandle(hThread);
	return TRUE;
}

HMODULE RemoteLoadLibraryNTA(DWORD dwTargetProcessID, LPCSTR lpszDllPath)
{	
	if (lpszDllPath == NULL || lpszDllPath[0] == 0)
	{
		::SetLastError(ERROR_MOD_NOT_FOUND);
		return NULL;
	}

	if (!IsWindowsNT())
	{
		::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return NULL;
	}

	LPTHREAD_START_ROUTINE lpfn = (LPTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	if (lpfn == NULL)
		return NULL;

	DWORD dwExitCode = 0;
	DWORD dwErrorCode = ERROR_SUCCESS;
	BOOL bOK = RemoteExecute(dwTargetProcessID, lpfn, (LPCVOID)lpszDllPath, (::strlen(lpszDllPath) + 1) * sizeof(char), dwExitCode, dwErrorCode);
	if (!bOK)
	{
		::SetLastError(dwErrorCode);
		return NULL;
	}

	if (dwExitCode == 0)
	{
		::SetLastError(ERROR_FILE_NOT_FOUND);
		return NULL;
	}

	return (HMODULE)dwExitCode;
}

HMODULE RemoteLoadLibraryNTW(DWORD dwTargetProcessID, LPCWSTR lpszDllPath)
{
	LPTHREAD_START_ROUTINE lpfn = (LPTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandleA("kernel32.dll"), "LoadLibraryW");
	if (lpfn == NULL)
	{
		return NULL;
	}
	
	if (lpszDllPath == NULL || lpszDllPath[0] == 0)
	{
		::SetLastError(ERROR_MOD_NOT_FOUND);
		return NULL;
	}

	if (!IsWindowsNT())
	{
		::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return NULL;
	}

	DWORD dwExitCode = 0;
	DWORD dwErrorCode = ERROR_SUCCESS;
	BOOL bOK = RemoteExecute(dwTargetProcessID, lpfn, (LPCVOID)lpszDllPath, (::wcslen(lpszDllPath) + 1) * sizeof(wchar_t), dwExitCode, dwErrorCode);
	if (!bOK)
	{
		::SetLastError(dwErrorCode);
		return NULL;
	}

	if (dwExitCode == 0)
	{
		::SetLastError(ERROR_FILE_NOT_FOUND);
		return NULL;
	}

	return (HMODULE)dwExitCode;
}

BOOL RemoteFreeLibraryNT(DWORD dwTargetProcessID, HMODULE hModule)
{
	if (!IsWindowsNT())
	{
		::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
		return FALSE;
	}

	LPTHREAD_START_ROUTINE lpfn = (LPTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandleA("kernel32.dll"), "FreeLibrary");
	if (lpfn == NULL)
		return FALSE;
	
	HANDLE hRemoteProc = OpenProcessForRemoteExecute(dwTargetProcessID);
	if (hRemoteProc == NULL)
		return FALSE;

	DWORD dwExitCode = 0;
	DWORD dwErrorCode = ERROR_SUCCESS;
	BOOL bOK = RemoteExecute(hRemoteProc, lpfn, (LPVOID)hModule, dwExitCode, dwErrorCode);
	::CloseHandle(hRemoteProc);
	if (!bOK)
	{
		::SetLastError(dwErrorCode);
		return FALSE;
	}

	if (dwExitCode == 0)
	{
		::SetLastError(ERROR_MOD_NOT_FOUND);
		return FALSE;
	}

	return TRUE;
}