// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "../detours/include/detours.h"
#include "SystemHook.h"
#include "detourapis.h"

BOOL APIENTRY DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
	{
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)pTrueCreateFileW, TransCreateFileW);
        DetourTransactionCommit();
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

