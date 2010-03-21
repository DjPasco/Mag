#ifndef SOCKET_SENDER
#define SOCKET_SENDER

#pragma once
#include <stdio.h>
#include "Winsock2.h"

class CSender
{
public:
	static CSender *instance(); 

	bool Initialize();
	void DropConection();
	bool SendFile(LPCSTR sFile);

protected:
	SOCKET m_SocketListen;
	static CSender *inst; 
};

#endif
