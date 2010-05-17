#include "stdafx.h"
#include "hook.h"
#include "detours.h"
#include "Psapi.h"
#include <tlhelp32.h>
#include <iostream>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace hook_utils
{
	namespace internal
	{
		void GetHookDllPath(char *sHookPath)
		{
			char dirPath[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, dirPath);

		#ifdef _DEBUG
			sprintf(sHookPath, "%s\\SystemHookD.dll", dirPath);
		#else
			sprintf(sHookPath, "%s\\SystemHook.dll", dirPath);
		#endif
		}

		void GetDetourDllPath(char *sDetourPath)
		{
			char dirPath[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, dirPath);

			sprintf(sDetourPath, "%s\\detoured.dll", dirPath);
		}

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

		void RunLoadLibraryInProcess(HANDLE hProcess, LPVOID LoadLibraryAddr, char *sDLLPath)
		{
			LPVOID LLParam = (LPVOID)VirtualAllocEx(hProcess, NULL, strlen(sDLLPath), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			WriteProcessMemory(hProcess, LLParam, sDLLPath, strlen(sDLLPath), NULL);
			HANDLE hLoadLibrary = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, LLParam, NULL, NULL);
			WaitForSingleObject(hLoadLibrary, INFINITE);
		}

		bool ExistsModule(DWORD dwProcID, char *sDLLPath)
		{
			HMODULE ModDLLHandle = NULL;
			BYTE * BytDLLBaseAdress = 0;
			MODULEENTRY32 MOEModuleInformation = { 0 };
			MOEModuleInformation.dwSize = sizeof(MODULEENTRY32);

			HANDLE HanModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcID);

			Module32First(HanModuleSnapshot, &MOEModuleInformation);

			do
			{
				if(!lstrcmpi(MOEModuleInformation.szExePath, sDLLPath))
				{
					CloseHandle(HanModuleSnapshot);
					return true;
				}
			} while(Module32Next(HanModuleSnapshot, &MOEModuleInformation));

			CloseHandle(HanModuleSnapshot);

			return false;
		}

		bool NeedHook(LPCSTR sExeName)
		{
			if(0 == lstrcmpi(sExeName, "DCService.exe"))
			{
				return false;
			}

			if(NULL != strstr(sExeName, "exe") || NULL != strstr(sExeName, "EXE"))
			{
				return true;
			}

			return false;
		}

		BOOL EjectDLL(DWORD WorProcessId, const char *sDllPath)
		{
			HANDLE HanProcess = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_WRITE, FALSE, WorProcessId);
			char sDLLFilePath[(MAX_PATH + 16)] = { 0 };

			strcpy(sDLLFilePath, sDllPath);

			HMODULE ModDLLHandle = NULL;
			BYTE * BytDLLBaseAdress = 0;
			MODULEENTRY32 MOEModuleInformation = { 0 };
			MOEModuleInformation.dwSize = sizeof(MODULEENTRY32);

			HANDLE HanModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, WorProcessId);

			Module32First(HanModuleSnapshot, &MOEModuleInformation);

			do
			{
				if(!lstrcmpi(MOEModuleInformation.szExePath, sDLLFilePath))
				{
					ModDLLHandle = MOEModuleInformation.hModule;
					BytDLLBaseAdress = MOEModuleInformation.modBaseAddr;
				}
			} while(Module32Next(HanModuleSnapshot, &MOEModuleInformation));

			CloseHandle(HanModuleSnapshot);

			HMODULE ModKernel32 = GetModuleHandle("Kernel32.dll");

			if(ModKernel32 != NULL)
			{
				if(ModDLLHandle != NULL && BytDLLBaseAdress != 0)
				{
					HANDLE HanDLLThread = CreateRemoteThread(HanProcess, NULL, 0, LPTHREAD_START_ROUTINE(GetProcAddress(ModKernel32, "FreeLibrary")), (VOID *)BytDLLBaseAdress, 0, NULL);

					if(HanDLLThread != NULL)
					{
						if(WaitForSingleObject(HanDLLThread, INFINITE) != WAIT_FAILED)
						{
							CloseHandle(HanDLLThread);
							CloseHandle(HanProcess);
							return TRUE;
						}
						CloseHandle(HanDLLThread);
					}
				}
			}

			CloseHandle(HanProcess);
			return FALSE;
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
		internal::GetHookDllPath(sHookPath);

		char sFullDetoursPath[MAX_PATH];
		internal::GetDetourDllPath(sFullDetoursPath);

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

	void GlobalHook(bool bInitial)
	{
		HWND hwnd = NULL;
		hwnd = ::FindWindow(NULL, "DCAntiVirusScan");

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

		if(TRUE == Process32First(snapshot, &entry))
		{
			LPVOID LoadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "LoadLibraryA");
			internal::EnableDebugPriv();
			while(TRUE == Process32Next(snapshot, &entry))
			{
				if(GetCurrentProcessId() != entry.th32ProcessID && internal::NeedHook(entry.szExeFile))
				{
					char sHookPath[MAX_PATH];
					internal::GetHookDllPath(sHookPath);

					if(bInitial || !internal::ExistsModule(entry.th32ProcessID, sHookPath))
					{
						char sFullDetoursPath[MAX_PATH];
						internal::GetDetourDllPath(sFullDetoursPath);

						HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_WRITE, FALSE, entry.th32ProcessID);
						internal::RunLoadLibraryInProcess(hProcess, LoadLibraryAddr, sFullDetoursPath);
						internal::RunLoadLibraryInProcess(hProcess, LoadLibraryAddr, sHookPath);
						CloseHandle(hProcess);
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

		if(TRUE == Process32First(snapshot, &entry))
		{
			internal::EnableDebugPriv();

			char sHookPath[MAX_PATH];
			internal::GetHookDllPath(sHookPath);

			char sFullDetoursPath[MAX_PATH];
			internal::GetDetourDllPath(sFullDetoursPath);

			while(TRUE == Process32Next(snapshot, &entry))
			{
				if(GetCurrentProcessId() != entry.th32ProcessID)
				{
					if(internal::NeedHook(entry.szExeFile))
					{
						internal::EjectDLL(entry.th32ProcessID, sFullDetoursPath);
						internal::EjectDLL(entry.th32ProcessID, sHookPath);
					}
				}
			}
		}

		CloseHandle(snapshot);
	}
}