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

	void LoadNotepadWithHookDll(std::wstring sRunExe)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		si.cb = sizeof(STARTUPINFO);

		std::wstring sHookDllPath = _T("C:\\MAG_REPO\\Bin\\SystemHook");
		
#ifdef _DEBUG
		sHookDllPath += _T("D");
#endif
		
		sHookDllPath += _T(".dll");

		if (!internal::DoesDllExportOrdinal1(sHookDllPath.c_str()))
		{
			printf("ERROR: %s does not export function with ordinal #1.", sHookDllPath.c_str());
			return;
		}

		std::string sDetourPath = "C:\\MAG_REPO\\Bin\\detoured.dll";
		std::string sHookDllPathWide = "C:\\MAG_REPO\\Bin\\SystemHook";

		std::string s = GetDllPath<std::string>("SystemHook");
		
#ifdef _DEBUG
		sHookDllPathWide += "D";
#endif
		sHookDllPathWide += ".dll";

		std::wstring sExe = GetExePath(sRunExe);

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
									   sHookDllPathWide.c_str(),
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

	std::wstring GetExePath(std::wstring sExeName)
	{
		std::wstring sPath = _T("C:\\MAG_REPO\\Bin\\");
		sPath += sExeName;

#ifdef _DEBUG
		sPath += _T("D");
#endif
		
		sPath += _T(".exe");

		return sPath;
	}

	template<class string_type>
	string_type GetDllPath(string_type sExeName)
	{
		string_type sPath = "C:\\MAG_REPO\\Bin\\";
		sPath += sExeName;

#ifdef _DEBUG
		sPath += "D";
#endif
		
		sPath += ".dll";

		return sPath;
	}

}