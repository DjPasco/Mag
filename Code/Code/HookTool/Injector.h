#if !defined(_INJECTOR_H_)
#define _INJECTOR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _COMMON_H_
	#include "..\Common\Common.h"
#endif

#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")
#include "..\Common\LockMgr.h"
#include <share.h>
#include <time.h>

class CExeModuleInstance;
class CNtInjectorThread;

class CInjector  
{
public:
	CInjector(BOOL bServerInstance);
	virtual ~CInjector();
	//
	// examines whether a process should be hooked up by the DLL
	//
	BOOL IsProcessForHooking(PSTR pszExaminedProcessName);
	//
	// Inject the DLL into all running processes
	//
	virtual BOOL InjectModuleIntoAllProcesses() = 0;
	//
	// Eject the DLL from all processes if it has been injected before 
	//
	virtual BOOL EjectModuleFromAllProcesses(HANDLE hWaitOn) = 0;
protected:
	//
	// Determines whether the instance is created by the hook server
	//
	BOOL m_bServerInstance;
private:
	//
	// Get the value of [Scope] / HookAll from the INI file
	//
	BOOL GetHookAllEnabled();
	//
	// Return the name of the INI file
	//
	void GetIniFile(char* pszIniFile);
	//
	// A comma separated list with name of processes
	// for hooking
	//
	char m_szProcessesForHooking[MAX_PATH];
	//
	// ... and those that should be protected and not hooked up
	//
	char  m_szProtectedProcesses[MAX_PATH];
	//
	//
	//
	BOOL m_bHookAllEnabledInitialized;
	//
	// Indicates whether all process must be hooked up
	//
	BOOL m_bHookAllEnabled;
};

class CWinHookInjector: public CInjector  
{
public:
	CWinHookInjector(BOOL bServerInstance, HHOOK* pHook);
private:
	//
	// Inject the DLL into all running processes
	//
	virtual BOOL InjectModuleIntoAllProcesses();
	//
	// Eject the DLL from all processes if it has been injected before 
	//
	virtual BOOL EjectModuleFromAllProcesses(HANDLE hWaitOn);
	//
	// Pointer to shared hook handle
	//
	static HHOOK* sm_pHook;
};

class CRemThreadInjector: public CInjector  
{
public:
	CRemThreadInjector(BOOL bServerInstance);
	virtual ~CRemThreadInjector();
	//
	// Inject the DLL into address space of a specific external process
	//
	BOOL InjectModuleInto(DWORD dwProcessId);
	//
	// Eject the DLL from the address space of an external process
	//
	BOOL EjectModuleFrom(DWORD dwProcessId);
private:
	//
	// Inject the DLL into all running processes
	//
	virtual BOOL InjectModuleIntoAllProcesses();
	//
	// Eject the DLL from all processes if it has been injected before 
	//
	virtual BOOL EjectModuleFromAllProcesses(HANDLE hWaitOn);
	//
	// Attempts to enable SeDebugPrivilege. This is required by use of
	// CreateRemoteThread() under NT/2K
	//
	BOOL EnableDebugPrivilege();
	//
	// Execute injection mechanism for NT/2K systems
	//
	virtual BOOL DoInjectModuleInto(CExeModuleInstance *pProcess);
	//
	// Perform actual ejection of the DLL from the address space of an external process
	//
	virtual BOOL DoEjectModuleFrom(CExeModuleInstance& process);
	//
	// Guard used by InjectModuleInto
	//
	static CCSWrapper sm_CritSecInjector;
	//
	// An object responsible for monitoring process creation/termination
	//
	CNtInjectorThread* m_pNtInjectorThread;
};


namespace socket_utils
{
	static int get_a_line(SOCKET &socket, char *buf, size_t len)
	{
		char *ptr;

		for(ptr = buf; ptr < &buf[len]; ptr++) 
		{
			/* FIXME: very inefficient to call recv so many times */
			if(recv(socket, ptr, sizeof(char), 0) <= 0)
				return 0;
			if(*ptr == '\n') {
				*++ptr = '\0';
				return 1;
			}
		}

		return 1;
	}

	static bool dsresult(SOCKET &socket, LPCSTR sFile)
	{
		char buff[BUFSIZ];
		get_a_line(socket, buff, sizeof(buff));

		if(strstr(buff, "FOUND"))
		{
			return false;
		}
				
		return true;
	}

	static bool CheckFile(LPCSTR sFile)
	{
		WSADATA wsaData;
		if(WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR)
		{
			return false;
		}

		struct sockaddr_in saServer;

		// Create a listening socket
		SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if(NULL == ListenSocket)
		{
			return true;
		}

		// Set up the sockaddr structure
		saServer.sin_family = AF_INET;
		saServer.sin_addr.s_addr = inet_addr("127.0.0.1");
		saServer.sin_port = htons(8888);

		int nConnectReturn = connect(ListenSocket, (struct sockaddr *) &saServer, sizeof(struct sockaddr_in));

		bool bRet(true);

		if(0 == nConnectReturn)
		{
			char szScanBuffer[MAX_PATH];

			sprintf(szScanBuffer, "FILE %s\n", sFile);
			int nSize = strlen(szScanBuffer);
			int nGetSize = send(ListenSocket, szScanBuffer, nSize, 0);
			if(nGetSize == nSize)
			{
				bRet = dsresult(ListenSocket, sFile);
			}			
		}

		shutdown(ListenSocket, 0);

		closesocket(ListenSocket);

		WSACleanup();

		return bRet;
	}

}


#endif
