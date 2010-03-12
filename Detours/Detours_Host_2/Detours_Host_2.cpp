#include "stdafx.h"
//#undef UNICODE
#include <cstdio>
//#include <windows.h>
#include <afx.h>
#include "include/detours.h"

#pragma comment(lib, "detoured.lib")
#pragma comment(lib, "detours.lib")

static BOOL CALLBACK ExportCallback(PVOID pContext,
                                    ULONG nOrdinal,
                                    PCHAR pszSymbol,
                                    PVOID pbTarget)
{
    (void)pContext;
    (void)pbTarget;
    (void)pszSymbol;

    if (nOrdinal == 1) {
        *((BOOL *)pContext) = TRUE;
    }
    return TRUE;
}

BOOL DoesDllExportOrdinal1(PCHAR pszDllPath)
{
    HMODULE hDll = LoadLibraryEx(pszDllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (hDll == NULL) {
        printf("withdll.exe: LoadLibraryEx(%s) failed with error %d.\n",
               pszDllPath,
               GetLastError());
        return FALSE;
    }

    BOOL validFlag = FALSE;
    DetourEnumerateExports(hDll, &validFlag, ExportCallback);
    FreeLibrary(hDll);
    return validFlag;
}

int main(int argc, char* argv[])
{
	STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    si.cb = sizeof(STARTUPINFO);
    char* DirPath = new char[MAX_PATH];
    
	GetCurrentDirectory(MAX_PATH, DirPath);

	CString sDllPath;
	sDllPath.Format(_T("%s\\WSHookD.dll"), DirPath);

	CString sDetourPath;
	sDetourPath.Format(_T("%s\\detoured.dll"), DirPath);

	TCHAR sDetourPath_[MAX_PATH] = _T("C:\\MAG_REPO\\Detours\\Bin\\detoured.dll");
	TCHAR sDllPath_[MAX_PATH] = _T("C:\\MAG_REPO\\Detours\\Bin\\SystemHookD.dll");

	if (!DoesDllExportOrdinal1(sDllPath_)) {
        printf("withdll.exe: Error: %s does not export function with ordinal #1.\n",
               sDllPath_);
        return 9003;
    }


    if(!DetourCreateProcessWithDll(_T("C:\\Windows\\Notepad.exe"),
								   NULL,
								   NULL,
								   NULL,
								   TRUE,
								   CREATE_DEFAULT_ERROR_MODE,// | CREATE_SUSPENDED,
								   NULL,
								   NULL,
								   &si,
								   &pi,
								   sDetourPath_,
								   sDllPath_,
								   NULL))
	{
		printf("withdll.exe: DetourCreateProcessWithDll failed: %d\n", GetLastError());
	}

    delete [] DirPath;

//	// Resume thread and wait on the process..
//	ResumeThread(pi.hThread);
//
//	WaitForSingleObject(pi.hProcess, INFINITE);

//    DWORD dwResult = 0;
//    if (!GetExitCodeProcess(pi.hProcess, &dwResult))
//	{
//        printf("withdll.exe: GetExitCodeProcess failed: %d\n", GetLastError());
//        return 9008;
//    }

	return 0;
}

