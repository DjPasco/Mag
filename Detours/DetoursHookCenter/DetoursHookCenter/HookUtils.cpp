#include "stdafx.h"
#include "HookUtils.h"
#include "detours.h"
#include <iostream>
#include "Log.h"

#ifdef _DEBUG
	#define FILE_USAGE "FileUsageD.exe"
#else
	#define FILE_USAGE "FileUsage.exe"
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

		BOOL DoesDllExportOrdinal1(CLog &log, LPCWSTR pszDllPath)
		{
			HMODULE hDll = LoadLibraryEx(pszDllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
			if (hDll == NULL)
			{
				CString sError;
				sError.Format(_T("ERROR: LoadLibraryEx(%s) failed with error %d."), pszDllPath, GetLastError());
				log.AddRichText(sError);
				return FALSE;
			}

			BOOL validFlag = FALSE;
			DetourEnumerateExports(hDll, &validFlag, ExportCallback);
			FreeLibrary(hDll);
			return validFlag;
		}
	}

	void LoadNotepadWithHookDll(CLog &log)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
		si.cb = sizeof(STARTUPINFO);

		LPCWSTR sDllPathW    = _T("C:\\MAG_REPO\\Bin\\SystemHookD.dll");

		if (!internal::DoesDllExportOrdinal1(log, sDllPathW))
		{
			CString sError;
			sError.Format(_T("ERROR: %s does not export function with ordinal #1."), sDllPathW);
			log.AddRichText(sError);
			return;
		}

		LPCSTR sDetourPath = "C:\\MAG_REPO\\Bin\\detoured.dll";
		LPCSTR sDllPath = "C:\\MAG_REPO\\Bin\\SystemHookD.dll";

		std::wstring sFileUsage = _T("C:\\MAG_REPO\\Bin\\");
		sFileUsage += _T(FILE_USAGE);

		if(!DetourCreateProcessWithDll(sFileUsage.c_str(),
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
									   sDllPath,
									   NULL))
		{
			CString sError;
			sError.Format(_T("ERROR: DetourCreateProcessWithDll failed: %d"), GetLastError());
			log.AddRichText(sError);
		}
	}
}