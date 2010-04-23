#include "stdafx.h"
#include "hook.h"
#include "detours.h"
#include <iostream>

//#include "DCSanner\DCSanner.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace hook_utils
{
	namespace internal
	{
		static BOOL CALLBACK ExportCallback(PVOID pContext,
											ULONG nOrdinal,
											PCHAR pszSymbol,
											PVOID pbTarget)
		{
			(void)pContext;
			(void)pbTarget;
			(void)pszSymbol;

			if (nOrdinal == 1)
			{
				*((BOOL *)pContext) = TRUE;
			}

			return TRUE;
		}

		BOOL DoesDllExportOrdinal1(LPCSTR pszDllPath)
		{
			HMODULE hDll = LoadLibraryEx(pszDllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
			if (hDll == NULL)
			{
				//printf("ERROR: LoadLibraryEx(%s) failed with error %d.", pszDllPath, GetLastError());
				return FALSE;
			}

			BOOL validFlag = FALSE;
			DetourEnumerateExports(hDll, &validFlag, ExportCallback);
			FreeLibrary(hDll);
			return validFlag;
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
		sHookDllPath.Format("%s\\%s", sCurrDir, "SystemHook.dll");
		//std::string sHookDllPath = sCurrDir + " \\" + "SystemHook.dll";
		
		if (!internal::DoesDllExportOrdinal1(sHookDllPath))
		{
			//printf("ERROR: %s does not export function with ordinal #1.", sHookDllPath.c_str());
			return;
		}

		CString sDetourPath;
		sDetourPath.Format("%s\\%s", sCurrDir, "detoured.dll");

		if(!DetourCreateProcessWithDll(sRunExe,
									   NULL,
									   NULL,
									   NULL,
									   TRUE,
									   CREATE_DEFAULT_ERROR_MODE,
									   NULL,
									   NULL,
									   &si,
									   &pi,
									   sDetourPath,
									   sHookDllPath,
									   NULL))
		{
			//printf("ERROR: DetourCreateProcessWithDll failed: %d", GetLastError());
		}
//		else
//		{
//			::WaitForSingleObject(pi.hProcess, INFINITE);
//			CloseHandle(pi.hProcess);
//			CloseHandle(pi.hThread);
//		}
	}
}