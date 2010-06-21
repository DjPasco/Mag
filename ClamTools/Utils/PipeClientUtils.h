#pragma once

#include "../Utils/npipe.h"


//Used send scan infor to pipe server.
namespace pipe_client_utils
{
	static void SendFileToPipeServer(LPCSTR sServerName, CSendObj *pObj, CFileResult &result)
	{
		result.m_bOK = true;

		CNamedPipe clientPipe;
		if (!CNamedPipe::ServerAvailable(".", _T(sServerName), 1000))
		{
			return;
		}

		if (!clientPipe.Open(".", _T(sServerName), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, 0))
		{
			return;
		}

		DWORD dwBytes;
		CSendObj newObj(*pObj);						
		clientPipe.Write(&newObj, sizeof(CSendObj), dwBytes);
		clientPipe.Read(&result, sizeof(CFileResult), dwBytes, NULL);
		clientPipe.Close();
	}
}