#include "stdafx.h"
//#undef UNICODE
#include <cstdio>
//#include <windows.h>
#include <afx.h>
#include "include/detours.h"

#pragma comment(lib, "detoured.lib")
#pragma comment(lib, "detours.lib")

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

    if(!DetourCreateProcessWithDll(_T("C:\\Windows\\Notepad.exe"), NULL, NULL,
        NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL,
        &si, &pi, sDetourPath, sDllPath, NULL))
	{
		printf("withdll.exe: DetourCreateProcessWithDll failed: %d\n", GetLastError());
	}

    delete [] DirPath;
    
	return 0;
}

