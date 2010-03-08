#include "stdafx.h"
#include <afx.h>
#include <Tlhelp32.h>
#include "detours.h"
#include <iostream>


#pragma comment(lib, "Ws2_32.lib")

#include <Winsock2.h>


#ifdef _DEBUG
	LPCSTR sHookPath = "c:\\MAG\\Detours\\Bin\\WSHookD.dll";
#else
	LPCSTR sHookPath = "c:\\MAG\\Detours\\Bin\\WSHook.dll";
#endif

CString Readline(SOCKET *client);

int _tmain(int argc, _TCHAR* argv[])
{
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hTool32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    BOOL bProcess = Process32First(hTool32, &pe32);
    if(bProcess == TRUE)
    {
		while((Process32Next(hTool32, &pe32)) == TRUE)
		{
			CString sProcName = pe32.szExeFile;
			if(sProcName == "notepad.exe")
			{
				LPCSTR sFullPath = "c:\\MAG\\Detours\\Bin\\detoured.dll";
				HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_WRITE, FALSE, pe32.th32ProcessID);
				LPVOID LoadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "LoadLibraryA");
				LPVOID LLParam = (LPVOID)VirtualAllocEx(hProcess, NULL, strlen(sFullPath), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
				WriteProcessMemory(hProcess, LLParam, sFullPath, strlen(sFullPath), NULL);
				CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, LLParam, NULL, NULL);

				LLParam = (LPVOID)VirtualAllocEx(hProcess, NULL, strlen(sHookPath), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
				WriteProcessMemory(hProcess, LLParam, sHookPath, strlen(sHookPath), NULL);
				CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, LLParam, NULL, NULL);

				CloseHandle(hProcess);
			}
		}
    }
    CloseHandle(hTool32);

	WORD sockVersion;
	WSADATA wsaData;
	int rVal;

	sockVersion = MAKEWORD(2,2);
	WSAStartup(sockVersion, &wsaData);

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(s == INVALID_SOCKET)
	{
		closesocket(s);
		WSACleanup();
		return 0;
	}

	SOCKADDR_IN sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");

	rVal = bind(s, (LPSOCKADDR)&sin, sizeof(sin));
	if(rVal == SOCKET_ERROR)
	{
		closesocket(s);
		WSACleanup();
		return 0;
	}

	rVal = listen(s, 2);
	if(rVal == SOCKET_ERROR)
	{
		closesocket(s);
		WSACleanup();
		return 0;
	}

	bool b_Done(false);

	while (!b_Done)
	{
		SOCKET client;
		client = accept(s, NULL, NULL);

		if(client == INVALID_SOCKET)
		{
			closesocket(s);
			WSACleanup();
			return 0;
		}

		CString sMessage;
		sMessage = Readline(&client);
		std::cout << sMessage;
	}

	closesocket(s);

	WSACleanup();


	system("pause");

	return 0;
}

CString Readline(SOCKET *client)
{
	CString sMess;
	char buffer;
	int rVal;

	while(true)
	{
		rVal = recv(*(client), &buffer, 1, 0);
		if(rVal == SOCKET_ERROR)
		{
			return "";
			WSACleanup();
		}
		
		if(buffer != '\n')
		{
			sMess += buffer;
		}
		else
		{
			break;
		}
	}

	return sMess;
}


