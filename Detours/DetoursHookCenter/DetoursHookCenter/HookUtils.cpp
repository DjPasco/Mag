#include "stdafx.h"
#include "HookUtils.h"
#include "detours.h"
#include <iostream>
#include "Log.h"

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

		LPCWSTR sDllPathW    = _T("C:\\MAG_REPO\\Detours\\Bin\\SystemHookD.dll");

		if (!internal::DoesDllExportOrdinal1(log, sDllPathW))
		{
			CString sError;
			sError.Format(_T("ERROR: %s does not export function with ordinal #1."), sDllPathW);
			log.AddRichText(sError);
			return;
		}

		LPCSTR sDetourPath = "C:\\MAG_REPO\\Detours\\Bin\\detoured.dll";
		LPCSTR sDllPath = "C:\\MAG_REPO\\Detours\\Bin\\SystemHookD.dll";

		if(!DetourCreateProcessWithDll(_T("C:\\Windows\\Notepad.exe"),
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

	void EnumeratePayloads(CLog &log)
	{
		HANDLE hOld = INVALID_HANDLE_VALUE;
		PDETOUR_BINARY pBinary = NULL;

		LPCWSTR sFilePath = _T("C:\\MAG_REPO\\Detours\\Bin\\SystemHookD.dll");

		hOld = CreateFile(sFilePath,
						  GENERIC_READ,
						  FILE_SHARE_READ,
						  NULL,
						  OPEN_EXISTING,
						  FILE_ATTRIBUTE_NORMAL,
						  NULL);

		if (hOld == INVALID_HANDLE_VALUE)
		{
			CString sError;
			sError.Format(_T("ERROR: %s: Failed to open input file with error: %d."), sFilePath, GetLastError());
			log.AddRichText(sError);
			return;
		}

		if ((pBinary = DetourBinaryOpen(hOld)) == NULL)
		{
			CString sError;
			sError.Format(_T("ERROR: %s: DetourBinaryOpen failed: %d."), sFilePath, GetLastError());
			log.AddRichText(sError);
			return;
		}

		if (hOld != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hOld);
			hOld = INVALID_HANDLE_VALUE;
		}

		GUID id;
		DWORD dwData = 0;
		DWORD dwIterator = 0;
		DetourBinaryEnumeratePayloads(pBinary, &id, &dwData, &dwIterator);
	}
}