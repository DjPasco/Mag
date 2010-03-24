#include "stdafx.h"
#include "../detours/include/detours.h"
#include "SystemHook.h"
#include "detourapis.h"
#include <tchar.h>

#ifndef SOCKET_SENDER
	#include "../../Socket/SocketSender.h"
#endif


BOOL APIENTRY DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
	{
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)pTrueCreateFileW, TransCreateFileW);
        DetourTransactionCommit();

		//CSender::instance()->Initialize();
		//CSender::instance()->SendFile("Prikabinom");

   }
    else if (dwReason == DLL_PROCESS_DETACH)
	{
		CSender::instance()->SendFile("Nukabinam");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)pTrueCreateFileW, TransCreateFileW);
        DetourTransactionCommit();
    }

    return TRUE;
}

