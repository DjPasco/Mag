#include "stdafx.h"
//#undef UNICODE
#include <cstdio>
//#include <windows.h>
#include <afx.h>
#include "include/detours.h"
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#include <Winsock2.h>


#pragma comment(lib, "detoured.lib")
#pragma comment(lib, "detours.lib")

static BOOL CALLBACK ExportCallback(PVOID pContext,
                                    ULONG nOrdinal,
                                    PCHAR pszSymbol,
                                    PVOID pbTarget)
{
    (void)pContext;
    (void)pbTarget;
    (void)pszSymbol;

    if (nOrdinal == 1) {
        *((BOOL *)pContext) = TRUE;
    }
    return TRUE;
}

BOOL DoesDllExportOrdinal1(PCHAR pszDllPath)
{
    HMODULE hDll = LoadLibraryEx(pszDllPath, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (hDll == NULL) {
        printf("withdll.exe: LoadLibraryEx(%s) failed with error %d.\n",
               pszDllPath,
               GetLastError());
        return FALSE;
    }

    BOOL validFlag = FALSE;
    DetourEnumerateExports(hDll, &validFlag, ExportCallback);
    FreeLibrary(hDll);
    return validFlag;
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

int main(int argc, char* argv[])
{
	STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    si.cb = sizeof(STARTUPINFO);
    char* DirPath = new char[MAX_PATH];
    
	GetCurrentDirectory(MAX_PATH, DirPath);

	CString sDllPath;
	sDllPath.Format(_T("%s\\WSHookD.dll"), DirPath);

	CString sDetourPath;
	sDetourPath.Format(_T("%s\\detoured.dll"), DirPath);

	TCHAR sDetourPath_[MAX_PATH] = _T("C:\\MAG_REPO\\Detours\\Bin\\detoured.dll");
	TCHAR sDllPath_[MAX_PATH] = _T("C:\\MAG_REPO\\Detours\\Bin\\SystemHookD.dll");

	if (!DoesDllExportOrdinal1(sDllPath_)) {
        printf("withdll.exe: Error: %s does not export function with ordinal #1.\n",
               sDllPath_);
        return 9003;
    }


    if(!DetourCreateProcessWithDll(_T("C:\\Windows\\Notepad.exe"),
								   NULL,
								   NULL,
								   NULL,
								   TRUE,
								   CREATE_DEFAULT_ERROR_MODE,// | CREATE_SUSPENDED,
								   NULL,
								   NULL,
								   &si,
								   &pi,
								   sDetourPath_,
								   sDllPath_,
								   NULL))
	{
		printf("withdll.exe: DetourCreateProcessWithDll failed: %d\n", GetLastError());
	}

    delete [] DirPath;

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

//	// Resume thread and wait on the process..
//	ResumeThread(pi.hThread);
//
//	WaitForSingleObject(pi.hProcess, INFINITE);

//    DWORD dwResult = 0;
//    if (!GetExitCodeProcess(pi.hProcess, &dwResult))
//	{
//        printf("withdll.exe: GetExitCodeProcess failed: %d\n", GetLastError());
//        return 9008;
//    }

	return 0;
}

