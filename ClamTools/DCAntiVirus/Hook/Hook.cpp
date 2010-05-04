#include "stdafx.h"
#include "hook.h"
#include "detours.h"

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
		void EnableDebugPriv()
		{
			HANDLE hToken;
			LUID luid;
			TOKEN_PRIVILEGES tkp;

			OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);

			LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &luid );

			tkp.PrivilegeCount = 1;
			tkp.Privileges[0].Luid = luid;
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			AdjustTokenPrivileges( hToken, false, &tkp, sizeof( tkp ), NULL, NULL );

			CloseHandle( hToken ); 
		}

		void Inject(PROCESSENTRY32 &entry)
		{
			char dirPath[MAX_PATH];
			char sHookPath[MAX_PATH];

			GetCurrentDirectory(MAX_PATH, dirPath);

		#ifdef _DEBUG
			sprintf(sHookPath, "%s\\SystemHookD.dll", dirPath);
		#else
			sprintf(sHookPath, "%s\\SystemHook.dll", dirPath);
		#endif

			char sFullDetoursPath[MAX_PATH];
			sprintf(sFullDetoursPath, "%s\\detoured.dll", dirPath);

			HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_WRITE, FALSE, entry.th32ProcessID);
			LPVOID LoadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "LoadLibraryA");
			LPVOID LLParam = (LPVOID)VirtualAllocEx(hProcess, NULL, strlen(sFullDetoursPath), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			WriteProcessMemory(hProcess, LLParam, sFullDetoursPath, strlen(sFullDetoursPath), NULL);
			CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, LLParam, NULL, NULL);

			LLParam = (LPVOID)VirtualAllocEx(hProcess, NULL, strlen(sHookPath), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			WriteProcessMemory(hProcess, LLParam, sHookPath, strlen(sHookPath), NULL);
			CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, LLParam, NULL, NULL);
			CloseHandle( hProcess );
		}

		bool NeedHook(LPCSTR sExeName)
		{
			if(NULL != strstr(sExeName, "exe") || NULL != strstr(sExeName, "EXE"))
			{
				if(0 == stricmp(sExeName, "notepad.exe"))
				{
					return true;
				}
				else if(0 == stricmp(sExeName, "TOTALCMD.EXE"))
				{
					return true;
				}
			}

			return false;
		}

		BOOL EjectDLL (DWORD WorProcessId, CONST CHAR * ChaDLL)
		{
			HANDLE HanProcess = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_WRITE, FALSE, WorProcessId);
			CHAR ChaDLLFilePath[(MAX_PATH + 16)] = { 0 };

			strcpy(ChaDLLFilePath, ChaDLL);

			HMODULE ModDLLHandle = NULL;
			BYTE * BytDLLBaseAdress = 0;
			MODULEENTRY32 MOEModuleInformation = { 0 };
			MOEModuleInformation.dwSize = sizeof ( MODULEENTRY32 );

			HANDLE HanModuleSnapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPMODULE, WorProcessId );

			Module32First(HanModuleSnapshot, &MOEModuleInformation);

			do
			{
				if(!strcmp(MOEModuleInformation.szExePath, ChaDLLFilePath))
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

		char sCurrDir[MAX_PATH];

		GetCurrentDirectory(MAX_PATH, sCurrDir);

		CString sHookDllPath;

#ifdef _DEBUG
		sHookDllPath.Format("%s\\%s", sCurrDir, "SystemHookD.dll");
#else
		sHookDllPath.Format("%s\\%s", sCurrDir, "SystemHook.dll");
#endif

		CString sDetourPath;
		sDetourPath.Format("%s\\%s", sCurrDir, "detoured.dll");

		DetourCreateProcessWithDll(sRunExe, NULL, NULL,
								   NULL, TRUE, CREATE_DEFAULT_ERROR_MODE,
								   NULL, NULL, &si, &pi,
								   sDetourPath, sHookDllPath, NULL);
	}

	void GlobalHook()
	{
		PROCESSENTRY32 entry;
		entry.dwFlags = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if(TRUE == Process32First(snapshot, &entry))
		{
			internal::EnableDebugPriv();

			while(TRUE == Process32Next(snapshot, &entry))
			{
				if(GetCurrentProcessId() != entry.th32ProcessID)
				{
					if(internal::NeedHook(entry.szExeFile))
					{
						internal::Inject(entry);
					}
				}
			}
		}

		CloseHandle( snapshot );
	}

	void GlobalUnHook()
	{
		PROCESSENTRY32 entry;
		entry.dwFlags = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if(TRUE == Process32First(snapshot, &entry))
		{
			internal::EnableDebugPriv();
			char sCurrDir[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, sCurrDir);
			char sFullDetoursPath[MAX_PATH];
			sprintf(sFullDetoursPath, "%s\\detoured.dll", sCurrDir);

			char sHookPath[MAX_PATH];
		#ifdef _DEBUG
			sprintf(sHookPath, "%s\\SystemHookD.dll", sCurrDir);
		#else
			sprintf(sHookPath, "%s\\SystemHook.dll", sCurrDir);
		#endif

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