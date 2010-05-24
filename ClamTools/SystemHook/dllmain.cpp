#include "stdafx.h"
#include "../../detours/include/detours.h"
#include "detourapis.h"

BOOL APIENTRY DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
	{
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)pTrueCreateFileW, TransCreateFileW);
        DetourAttach(&(PVOID&)pTrueCreateFileA, TransCreateFileA);
        DetourTransactionCommit();
   }
    else if (dwReason == DLL_PROCESS_DETACH)
	{
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)pTrueCreateFileW, TransCreateFileW);
	    DetourDetach(&(PVOID&)pTrueCreateFileA, TransCreateFileA);
        DetourTransactionCommit();
    }

    return TRUE;
}

