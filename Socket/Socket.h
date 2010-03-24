#ifndef _SOCKET_H_
#define _SOCKET_H_
#pragma once

#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

namespace socket_utils
{
	static bool SendFile(LPCWSTR sFile)
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
			//char szScanBuffer[MAX_PATH];

			//sprintf_s(szScanBuffer, "%s\n", sFile);
			//int nSize = strlen(szScanBuffer);
			//int nGetSize = send(ListenSocket, szScanBuffer, nSize, 0);
		}

		shutdown(ListenSocket, 0);

		closesocket(ListenSocket);

		WSACleanup();

		return bRet;
	}
}
#endif
