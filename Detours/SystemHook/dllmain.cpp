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
		
		//HWND hwnd = NULL;
		//hwnd = FindWindow(NULL, _T("DCAntivirus"));

		//if(hwnd)
		//{
		//	utils::SetHwnd(hwnd);
		//}

	 //   wxString servername = IPC_SERVICE;
	 //   wxString hostname = IPC_HOST;
		//wxString topic = IPC_TOPIC;

		//MyClient *m_client = new MyClient;
		////if(m_client->Connect(hostname, servername, topic))
		////{
		////	utils::SetClient(m_client);
		////}

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

