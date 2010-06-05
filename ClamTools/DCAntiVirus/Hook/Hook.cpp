#include "stdafx.h"
#include "hook.h"
#include "detours.h"
#include "Psapi.h"
#include <tlhelp32.h>
#include <iostream>
#include "RemoteLib.h"

#include "../../Utils/Registry.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace hook_utils
{
	namespace internal
	{
		void EnableDebugPriv()
		{
			HANDLE hToken;
			LUID luid;
			TOKEN_PRIVILEGES tkp;

			OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

			LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

			tkp.PrivilegeCount = 1;
			tkp.Privileges[0].Luid = luid;
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL);

			CloseHandle(hToken); 
		}

		bool NeedHook(LPCSTR sExeName)
		{
			if(0 == lstrcmpi(sExeName, "DCService.exe"))
			{
				return false;
			}

			if(0 == lstrcmpi(sExeName, "DCServiceD.exe"))
			{
				return false;
			}

			if(0 == lstrcmpi(sExeName, "DCAntiVirus.exe"))
			{
				return false;
			}

			if(0 == lstrcmpi(sExeName, "DCAntiVirusD.exe"))
			{
				return false;
			}

			if(NULL != strstr(sExeName, "exe") || NULL != strstr(sExeName, "EXE"))
			{
				return true;
			}

			return false;
		}

		BOOL EjectDLL(DWORD WorProcessId, const char *sDllPath, LPCSTR sProcName)
		{
			TRACE("Ejecting %s.\n", sDllPath);

			HMODULE hDllModule = RemoteGetModuleHandleNT(WorProcessId, sDllPath); 
			if(NULL == hDllModule)
			{
				TRACE("Dll not found.\n");
				return FALSE;
			}

			TRACE("Dll found.\n");

			if(!RemoteFreeLibraryNT(WorProcessId, hDllModule))
			{
				TRACE("UnHooking error.\n");
				LPVOID lpMsgBuf;
				FormatMessage( 
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					0, // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL);
				TRACE("Reason: %s.\n", lpMsgBuf);
				return FALSE;
			}

			TRACE("Dll ejected.\n");

			return TRUE;
		}
	}

	void StartExeWithHookDll(LPCSTR sRunExe)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		si.cb = sizeof(STARTUPINFO);

		char sHookPath[MAX_PATH];
		path_utils::GetHookDllPath(sHookPath);

		char sFullDetoursPath[MAX_PATH];
		path_utils::GetDetourDllPath(sFullDetoursPath);

		DetourCreateProcessWithDll(sRunExe, NULL, NULL,
								   NULL, TRUE, CREATE_DEFAULT_ERROR_MODE,
								   NULL, NULL, &si, &pi,
								   sFullDetoursPath, sHookPath, NULL);
	}

	int GetProcessCount()
	{
		PERFORMACE_INFORMATION pi;
		if(GetPerformanceInfo(&pi, sizeof(PERFORMACE_INFORMATION)))
		{
			return pi.ProcessCount;
		}
		else
		{
			return -1;
		}
	}

	bool ExistsModule(DWORD dwProcID, char *sDLLPath)
	{
		if(NULL == RemoteGetModuleHandleNT(dwProcID, sDLLPath))
		{
			return false;
		}

		return true;
	}

	void GlobalHook(bool bInitial)
	{
		HWND hwnd = NULL;
		hwnd = ::FindWindow(NULL, sgServerName);

		if(NULL == hwnd)
		{
			return;
		}

		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if(INVALID_HANDLE_VALUE == snapshot)
		{
			return;
		}

		internal::EnableDebugPriv();

		if(TRUE == Process32First(snapshot, &entry))
		{
			while(TRUE == Process32Next(snapshot, &entry))
			{
				if(GetCurrentProcessId() != entry.th32ProcessID && internal::NeedHook(entry.szExeFile))
				{
					char sHookPath[MAX_PATH];
					path_utils::GetHookDllPath(sHookPath);

					if(bInitial || !ExistsModule(entry.th32ProcessID, sHookPath))
					{
						TRACE("Working on %s.\n", entry.szExeFile);
						char sDetoursPath[MAX_PATH];
						path_utils::GetDetourDllPath(sDetoursPath);

						if(NULL == RemoteLoadLibraryNT(entry.th32ProcessID, sDetoursPath))
						{
							TRACE("Hook failed on %s.\n", sDetoursPath);
							LPVOID lpMsgBuf;
							FormatMessage( 
								FORMAT_MESSAGE_ALLOCATE_BUFFER | 
								FORMAT_MESSAGE_FROM_SYSTEM | 
								FORMAT_MESSAGE_IGNORE_INSERTS,
								NULL,
								GetLastError(),
								0, // Default language
								(LPTSTR) &lpMsgBuf,
								0,
								NULL);
							TRACE("Reason: %s.\n", lpMsgBuf);
						}
						else
						{
							TRACE("Hook OK on %s.\n", sDetoursPath);
						}

						if(NULL == RemoteLoadLibraryNT(entry.th32ProcessID, sHookPath))
						{
							TRACE("Hook failed on %s.\n", sHookPath);
							LPVOID lpMsgBuf;
							FormatMessage( 
								FORMAT_MESSAGE_ALLOCATE_BUFFER | 
								FORMAT_MESSAGE_FROM_SYSTEM | 
								FORMAT_MESSAGE_IGNORE_INSERTS,
								NULL,
								GetLastError(),
								0, // Default language
								(LPTSTR) &lpMsgBuf,
								0,
								NULL);
							TRACE("Reason: %s.\n", lpMsgBuf);
						}
						else
						{
							TRACE("Hook OK on %s.\n", sHookPath);
						}
					}
				}
			}
		}

		CloseHandle( snapshot );
	}

	void GlobalUnHook()
	{
		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if(INVALID_HANDLE_VALUE == snapshot)
		{
			return;
		}

		internal::EnableDebugPriv();

		if(TRUE == Process32First(snapshot, &entry))
		{
			char sHookPath[MAX_PATH];
			path_utils::GetHookDllPath(sHookPath);

			char sFullDetoursPath[MAX_PATH];
			path_utils::GetDetourDllPath(sFullDetoursPath);

			while(TRUE == Process32Next(snapshot, &entry))
			{
				if(GetCurrentProcessId() != entry.th32ProcessID)
				{
					if(internal::NeedHook(entry.szExeFile))
					{
						TRACE("Working on %s.\n", entry.szExeFile);
						internal::EjectDLL(entry.th32ProcessID, sHookPath, entry.szExeFile);
						internal::EjectDLL(entry.th32ProcessID, sFullDetoursPath, entry.szExeFile);
					}
				}
			}
		}

		CloseHandle(snapshot);
	}
}