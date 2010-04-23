#include "stdafx.h"
#include "../detours/include/detours.h"
#include "SystemHook.h"
#include "detourapis.h"
#include <tchar.h>

BOOL APIENTRY DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
	{
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)pTrueCreateFileW, TransCreateFileW);
        DetourTransactionCommit();
		
		HWND hwnd = NULL;
		hwnd = FindWindow(NULL, _T("DCAntivirus"));
		if(hwnd)
		{
			utils::SetHwnd(hwnd);
		}
   }
    else if (dwReason == DLL_PROCESS_DETACH)
	{
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)pTrueCreateFileW, TransCreateFileW);
        DetourTransactionCommit();
    }

    return TRUE;
}

