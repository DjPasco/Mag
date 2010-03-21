#include "StdAfx.h"
#include "SocketSender.h"

#pragma comment(lib, "Ws2_32.lib")


CSender *CSender::inst = NULL;

CSender *CSender::instance()
{
	if (NULL == inst)
	{
		inst = new CSender();
	}

	return inst;
};

bool CSender::Initialize()
{
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR)
	{
		m_SocketListen = NULL;
		return false;
	}

	struct sockaddr_in saServer;

	// Create a listening socket
	m_SocketListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(NULL == m_SocketListen)
	{
		return false;
	}

	// Set up the sockaddr structure
	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = inet_addr("127.0.0.1");
	saServer.sin_port = htons(8888);

	int nConnectReturn = connect(m_SocketListen, (struct sockaddr *) &saServer, sizeof(struct sockaddr_in));

	if(0 != nConnectReturn)
	{
		m_SocketListen = NULL;
		return false;
	}

	//SendFile("Soketas sukurtas");

	return true;
}

void CSender::DropConection()
{
	shutdown(m_SocketListen, 0);
	closesocket(m_SocketListen);
	WSACleanup();
}

bool CSender::SendFile(LPCSTR sFile)
{
	if(!Initialize())
	{
		return false;
	}

	char szScanBuffer[MAX_PATH];

	sprintf_s(szScanBuffer, "%s\n", sFile);
	int nSize = strlen(szScanBuffer);
	int nGetSize = send(m_SocketListen, szScanBuffer, nSize, 0);


	DropConection();

	return true;
}