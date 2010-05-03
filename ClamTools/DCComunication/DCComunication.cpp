#include "stdafx.h"
#include "DCComunication.h"
#include "../Utils/PipeUtils.h"
#include <tchar.h>
#include <strsafe.h>

static HANDLE hPipe = NULL;

namespace pipe_utils
{
	namespace internal
	{
		bool Connect()
		{
			while(TRUE) 
			{
				hPipe = CreateFile( 
						strPipeName,			// Pipe name 
						GENERIC_READ |			// Read and write access 
						GENERIC_WRITE,
						0,						// No sharing 
						NULL,					// Default security attributes
						OPEN_EXISTING,			// Opens existing pipe 
						0,						// Default attributes 
						NULL);					// No template file 

				// Break if the pipe handle is valid. 
				if(hPipe != INVALID_HANDLE_VALUE) 
				{					
					break; 
				}
		 
				// Exit if an error other than ERROR_PIPE_BUSY occurs
				// All pipe instances are busy, so wait for 5 seconds
				if(GetLastError() != ERROR_PIPE_BUSY || !WaitNamedPipe(strPipeName, 500)) 
				{
					return false;
				}
			}

			/////////////////////////////////////////////////////////////////////////
			// Sets the read mode and the blocking mode of the specified named pipe.
			// 

			// Set data to be read from the pipe as a stream of messages
			DWORD dwMode = PIPE_READMODE_MESSAGE;
			BOOL bResult = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
			if(!bResult) 
			{
				return false;
			}

			return true;
		}

		bool CheckFile(LPCSTR sFile)
		{
			// A char buffer of BUFFER_SIZE chars, aka BUFFER_SIZE * sizeof(TCHAR) 
			// bytes. The buffer should be big enough for ONE request to the server.

			char sFilePath[BUFFER_SIZE];	// Client -> Server
			DWORD dwBytesWritten, dwRequestBytes;
			char sReply[BUFFER_SIZE];		// Server -> Client
			DWORD dwBytesRead, dwReplyBytes;

			// Send one message to the pipe.

			StringCchCopy(sFilePath, BUFFER_SIZE, sFile);
			dwRequestBytes = sizeof(char) * (lstrlen(sFilePath) + 1);

			BOOL bResult = WriteFile(hPipe, sFilePath, dwRequestBytes, &dwBytesWritten, NULL);
			if(!bResult || dwRequestBytes != dwBytesWritten) 
			{
				return false; //Error. Assumpt that file is OK. 
			}

			// Receive the response from the server.
			dwReplyBytes = sizeof(char) * BUFFER_SIZE;
			bResult = ReadFile(hPipe, sReply, dwReplyBytes, &dwBytesRead, NULL);
			if(!bResult && GetLastError() != ERROR_MORE_DATA) 
			{
				return false;//Error. Assumpt that file is OK. 
			}	

			return true;
		}
	};

	static bool Execute(LPCWSTR sFile)
	{
		BOOL bResult = FALSE;
		if(NULL == hPipe)
		{
			if(!internal::Connect())
			{
				return true; //Error. Assumpt that file is OK. 
			}
		}

		if(NULL != hPipe)
		{
			char sPath[MAX_PATH];
			WideCharToMultiByte( CP_ACP, 0, sFile, -1, sPath, MAX_PATH,NULL,NULL);
			return internal::CheckFile(sPath);
		}

		return true;
	};
};

bool CDCClient::Execute(LPCWSTR sFile)
{
	return pipe_utils::Execute(sFile);
}

void CDCClient::Close()
{
	CloseHandle(hPipe);
}



