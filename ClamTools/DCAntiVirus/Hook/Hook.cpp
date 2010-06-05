#include "stdafx.h"
#include "hook.h"
#include "detours.h"
#include "Psapi.h"
#include <tlhelp32.h>
#include <iostream>
#include "RemoteLib.h"

#include "../../Utils/Registry.h"
#include "../../Utils/Log.h"


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
			hook_log_utils::LogString("Ejecting", sDllPath);

			HMODULE hDllModule = RemoteGetModuleHandleNT(WorProcessId, sDllPath); 
			if(NULL == hDllModule)
			{
				hook_log_utils::WriteLine("Dll not found.");
				return FALSE;
			}

			hook_log_utils::WriteLine("Dll found.");

			if(!RemoteFreeLibraryNT(WorProcessId, hDllModule))
			{
				hook_log_utils::WriteLine("UnHooking error.");
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
				hook_log_utils::LogString("Reason", lpMsgBuf);
				return FALSE;
			}

			hook_log_utils::WriteLine("Dll ejected.");

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
		if(bInitial)
		{
			hook_log_utils::LogHeader("Hook System", GetCurrentProcessId());
		}

		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if(INVALID_HANDLE_VALUE == snapshot)
		{
			hook_log_utils::WriteLine("INVALID_HANDLE_VALUE.");
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
						hook_log_utils::LogString("Working on", entry.szExeFile);
						char sDetoursPath[MAX_PATH];
						path_utils::GetDetourDllPath(sDetoursPath);

						if(NULL == RemoteLoadLibraryNT(entry.th32ProcessID, sDetoursPath))
						{
							hook_log_utils::LogString("Hook failed on", sDetoursPath);
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
							hook_log_utils::LogString("Reason", lpMsgBuf);
						}
						else
						{
							hook_log_utils::LogString("Hook OK on", sDetoursPath);
						}

						if(NULL == RemoteLoadLibraryNT(entry.th32ProcessID, sHookPath))
						{
							hook_log_utils::LogString("Hook failed on", sHookPath);
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
							hook_log_utils::LogString("Reason", lpMsgBuf);
						}
						else
						{
							hook_log_utils::LogString("Hook OK on", sHookPath);
						}
					}
				}
			}
		}

		CloseHandle( snapshot );
	}

	void GlobalUnHook()
	{
		hook_log_utils::LogHeader("UnHook System", GetCurrentProcessId());

		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if(INVALID_HANDLE_VALUE == snapshot)
		{
			hook_log_utils::WriteLine("INVALID_HANDLE_VALUE.");
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
						hook_log_utils::LogString("Working on", entry.szExeFile);
						internal::EjectDLL(entry.th32ProcessID, sHookPath, entry.szExeFile);
						internal::EjectDLL(entry.th32ProcessID, sFullDetoursPath, entry.szExeFile);
					}
				}
			}
		}

		CloseHandle(snapshot);
	}
}