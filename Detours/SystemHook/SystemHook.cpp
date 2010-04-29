#include "stdafx.h"
#include "../detours/include/detours.h"
#include <tchar.h>
#include <stdio.h>
#include "SystemHook.h"
#include <iostream>
#include "../Utils/SendObj.h"
#include "client.h"

#include "ipcsetup.h"

//static HWND g_Hwnd;
static int nTime = 0;
static MyClient m_Client;

extern HANDLE (WINAPI * pTrueCreateFileW)(LPCWSTR lpFileName,
										  DWORD dwDesiredAccess,
										  DWORD dwShareMode,
										  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
										  DWORD dwCreationDisposition,
										  DWORD dwFlagsAndAttributes,
										  HANDLE hTemplateFile);

HANDLE WINAPI TransCreateFileW(LPCWSTR lpFileName,
							   DWORD dwDesiredAccess,
							   DWORD dwShareMode,
							   LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							   DWORD dwCreationDisposition,
							   DWORD dwFlagsAndAttributes,
							   HANDLE hTemplateFile)
{
	//CSendObj obj;
	char sh[MAX_PATH];
	WideCharToMultiByte( CP_ACP, 0, lpFileName, -1, sh, MAX_PATH,NULL,NULL); 

	//COPYDATASTRUCT copy;

	//copy.dwData = 1;          // function identifier
	//copy.cbData = sizeof( obj );  // size of data
	//copy.lpData = &obj;           // data structure

	//LRESULT result = SendMessage(g_Hwnd,
	//							 WM_COPYDATA,
	//							 0,
	//	
	if(nTime == 0 )	
	{
		wxString servername = IPC_SERVICE;
	    wxString hostname = IPC_HOST;
		wxString topic = IPC_TOPIC;

		if(m_Client.Connect(hostname, servername, topic))
		{
			//m_Client.GetConnection()->Execute(_T("Date"));
		}
		nTime = 1;
	}
		
    if (m_Client.IsConnected())
	{
		wxString s = sh;
		m_Client.GetConnection()->Execute(_T("Date"));
	}


    return pTrueCreateFileW(lpFileName,
					   dwDesiredAccess,
					   dwShareMode,
					   lpSecurityAttributes,
					   dwCreationDisposition,
					   dwFlagsAndAttributes,
					   hTemplateFile);
};

extern HANDLE (WINAPI * pTrueCreateFileA)(LPCSTR lpFileName,
										  DWORD dwDesiredAccess,
										  DWORD dwShareMode,
										  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
										  DWORD dwCreationDisposition,
										  DWORD dwFlagsAndAttributes,
										  HANDLE hTemplateFile);

HANDLE WINAPI TransCreateFileA(LPCSTR lpFileName,
							   DWORD dwDesiredAccess,
							   DWORD dwShareMode,
							   LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							   DWORD dwCreationDisposition,
							   DWORD dwFlagsAndAttributes,
							   HANDLE hTemplateFile)
{
	return CreateFileA(lpFileName,
					   dwDesiredAccess,
					   dwShareMode,
					   lpSecurityAttributes,
					   dwCreationDisposition,
					   dwFlagsAndAttributes,
					   hTemplateFile);
};

SYSTEM_HOOK_API void DoNothing()
{
	//
}

namespace utils
{
	void SetHwnd(HWND Hwnd)
	{
		//g_Hwnd = Hwnd;
	}
}
