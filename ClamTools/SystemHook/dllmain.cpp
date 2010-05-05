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
        DetourAttach(&(PVOID&)pTrueCreateProcessW, TransCreateProcessW);
		DetourAttach(&(PVOID&)pTrueCreateProcessA, TransCreateProcessA);
        DetourTransactionCommit();
   }
    else if (dwReason == DLL_PROCESS_DETACH)
	{
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)pTrueCreateFileW, TransCreateFileW);
        DetourDetach(&(PVOID&)pTrueCreateProcessW, TransCreateProcessW);
		DetourDetach(&(PVOID&)pTrueCreateProcessA, TransCreateProcessA);
        DetourTransactionCommit();
    }

    return TRUE;
}

