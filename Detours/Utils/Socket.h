#ifndef _SOCKET_H_
#define _SOCKET_H_
#pragma once

#include <windows.h>
#include <stdio.h>
#include "string"
#pragma comment(lib, "Ws2_32.lib")

static SOCKET ListenSocket = NULL;

namespace socket_utils
{
	static std::string Readline(SOCKET *client)
	{
		std::string sMess;
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

	static bool SendFile(LPCWSTR sFile)
	{
		if(NULL == ListenSocket)
		{
			struct sockaddr_in saServer;
			// Create a listening socket
			ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			if(NULL == ListenSocket)
			{
				return true;
			}

			// Set up the sockaddr structure
			saServer.sin_family = AF_INET;
			saServer.sin_addr.s_addr = inet_addr("127.0.0.1");
			saServer.sin_port = htons(8888);

			int nConnectReturn = connect(ListenSocket, (struct sockaddr *) &saServer, sizeof(struct sockaddr_in));
			if(0 != nConnectReturn)
			{
				ListenSocket = NULL;
				return true;
			}
		}

		if(NULL != ListenSocket)
		{
			char szScanBuffer[MAX_PATH];
			WideCharToMultiByte( CP_ACP, 0, sFile, -1, szScanBuffer, MAX_PATH,NULL,NULL);

			sprintf(szScanBuffer, "%s\n", szScanBuffer);
			int nSize = strlen(szScanBuffer);
			int nGetSize = send(ListenSocket, szScanBuffer, nSize, 0);

			std::string sMessage = Readline(&ListenSocket);
		}

		return true;
	}

	static void ShutDown()
	{
		if(NULL != ListenSocket)
		{
			shutdown(ListenSocket, 0);
			closesocket(ListenSocket);
		}
	}
}
#endif
