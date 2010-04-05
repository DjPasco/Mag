#include "stdafx.h"
#include "HookUtils.h"
#include "detours.h"

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

		BOOL DoesDllExportOrdinal1(LPCWSTR pszDllPath)
		{
			HMODULE hDll = LoadLibraryEx(pszDllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
			if (hDll == NULL)
			{
				printf("ERROR: LoadLibraryEx(%s) failed with error %d.", pszDllPath, GetLastError());
				return FALSE;
			}

			BOOL validFlag = FALSE;
			DetourEnumerateExports(hDll, &validFlag, ExportCallback);
			FreeLibrary(hDll);
			return validFlag;
		}
	}

	void StartExeWithHookDll(std::wstring sRunExe)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		si.cb = sizeof(STARTUPINFO);

		std::wstring sHookDllPathW = GetDllPath(_T("SystemHook"));
		
		if (!internal::DoesDllExportOrdinal1(sHookDllPathW.c_str()))
		{
			printf("ERROR: %s does not export function with ordinal #1.", sHookDllPathW.c_str());
			return;
		}

		std::string sDetourPath = "C:\\MAG_REPO\\Bin\\detoured.dll";
		std::string sHookDllPath = GetDllPath("SystemHook");
		std::wstring sExe = GetExePathW(sRunExe);

		if(!DetourCreateProcessWithDll(sExe.c_str(),
									   NULL,
									   NULL,
									   NULL,
									   TRUE,
									   CREATE_DEFAULT_ERROR_MODE,
									   NULL,
									   NULL,
									   &si,
									   &pi,
									   sDetourPath.c_str(),
									   sHookDllPath.c_str(),
									   NULL))
		{
			printf("ERROR: DetourCreateProcessWithDll failed: %d", GetLastError());
		}
		else
		{
			::WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}

	std::wstring GetExePathW(std::wstring sExeName)
	{
		std::wstring sPath = _T("C:\\MAG_REPO\\Bin\\");
		sPath += sExeName;

#ifdef _DEBUG
		sPath += _T("D");
#endif
		
		sPath += _T(".exe");

		return sPath;
	}

	std::string GetDllPath(std::string sExeName)
	{
		std::string sPath = "C:\\MAG_REPO\\Bin\\";
		sPath += sExeName;

#ifdef _DEBUG
		sPath += "D";
#endif
		
		sPath += ".dll";

		return sPath;
	}

	std::wstring GetDllPath(std::wstring sExeName)
	{
		std::wstring sPath = _T("C:\\MAG_REPO\\Bin\\");
		sPath += sExeName;

#ifdef _DEBUG
		sPath += _T("D");
#endif
		
		sPath += _T(".dll");

		return sPath;
	}
}