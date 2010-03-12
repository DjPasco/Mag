// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "../../detours/include/detours.h"
#include "ForceVersionDLL.h"
#include "detourapis.h"

// DllMain function attaches and detaches the TimedSleep detour to the
// Sleep target function.  The Sleep target function is referred to
// through the TrueSleep target pointer.
BOOL APIENTRY DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    if (dwReason == DLL_PROCESS_ATTACH) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)pfuncGetVersionEx, ForceGetVersionEx);
        DetourTransactionCommit();
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)pfuncGetVersionEx, ForceGetVersionEx);
        DetourTransactionCommit();
    }
    return TRUE;
}

