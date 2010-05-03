#include "stdafx.h"
#include "DCComunication.h"
#include "SendObj.h"
#include "../Utils/Socket.h"
#include <tchar.h>
#include <strsafe.h>

static HWND g_Hwnd = NULL;
namespace wnd_utils
{
	static bool Execute(LPCWSTR sFile)
	{
		if(NULL == g_Hwnd)
		{
			HWND hwnd = NULL;
			hwnd = FindWindow(NULL, "DCAntiVirusScan");

			if(NULL != hwnd)
			{
				g_Hwnd = hwnd;
			}
		}
		else
		{
			CSendObj obj;
			WideCharToMultiByte( CP_ACP, 0, sFile, -1, obj.m_sPath, MAX_PATH,NULL,NULL);
			COPYDATASTRUCT copy;

			copy.dwData = 1;
			copy.cbData = sizeof(obj);
			copy.lpData = &obj;

			LRESULT result = SendMessage(g_Hwnd,
										 WM_COPYDATA,
										 0,
										 (LPARAM) (LPVOID) &copy);

		}

		return true;
	}
};

#define BUFFER_SIZE		1024 // 1K
#define strPipeName "\\\\.\\pipe\\HelloWorld"

static HANDLE hPipe = NULL;

namespace pipe_utils
{
	static bool Execute(LPCWSTR sFile)
	{
		BOOL bResult = FALSE;
	if(NULL == hPipe)
	{
		while (TRUE) 
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
			if (hPipe != INVALID_HANDLE_VALUE) 
			{
				
				break; 
			}
	 
			if (// Exit if an error other than ERROR_PIPE_BUSY occurs
				GetLastError() != ERROR_PIPE_BUSY 
				||
				// All pipe instances are busy, so wait for 5 seconds
				!WaitNamedPipe(strPipeName, 5000)) 
			{
				_tprintf(_T("Unable to open named pipe %s w/err 0x%08lx\n"),
					strPipeName, GetLastError());
				return 1;
			}
		}
			/////////////////////////////////////////////////////////////////////////
		// Sets the read mode and the blocking mode of the specified named pipe.
		// 

		// Set data to be read from the pipe as a stream of messages
		DWORD dwMode = PIPE_READMODE_MESSAGE;
		bResult = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
		if (!bResult) 
		{
			_tprintf(_T("SetNamedPipeHandleState failed w/err 0x%08lx\n"), 
				GetLastError()); 
			return 1;
		}
	}
	//_tprintf(_T("The named pipe, %s, is connected.\n"), strPipeName);
	

	/////////////////////////////////////////////////////////////////////////
	// Send a message to the pipe server and receive its response.
	// 

	// A char buffer of BUFFER_SIZE chars, aka BUFFER_SIZE * sizeof(TCHAR) 
	// bytes. The buffer should be big enough for ONE request to the server.

	TCHAR chRequest[BUFFER_SIZE];	// Client -> Server
	DWORD cbBytesWritten, cbRequestBytes;
	TCHAR chReply[BUFFER_SIZE];		// Server -> Client
	DWORD cbBytesRead, cbReplyBytes;

	// Send one message to the pipe.

	StringCchCopy(chRequest, BUFFER_SIZE, _T("Default request from client"));
	cbRequestBytes = sizeof(TCHAR) * (lstrlen(chRequest) + 1);

	bResult = WriteFile(			// Write to the pipe.
		hPipe,						// Handle of the pipe
		chRequest,					// Message to be written
		cbRequestBytes,				// Number of bytes to write
		&cbBytesWritten,			// Number of bytes written
		NULL);						// Not overlapped 

	if (!bResult/*Failed*/ || cbRequestBytes != cbBytesWritten/*Failed*/) 
	{
		_tprintf(_T("WriteFile failed w/err 0x%08lx\n"), GetLastError());
		return 1;
	}

	//_tprintf(_T("Sends %ld bytes; Message: \"%s\"\n"), 
	//	cbBytesWritten, chRequest);

	// Receive the response from the server.

	cbReplyBytes = sizeof(TCHAR) * BUFFER_SIZE;
//	do
	{
		bResult = ReadFile(			// Read from the pipe.
			hPipe,					// Handle of the pipe
			chReply,				// Buffer to receive the reply
			cbReplyBytes,			// Size of buffer 
			&cbBytesRead,			// Number of bytes read 
			NULL);					// Not overlapped 

		if (!bResult && GetLastError() != ERROR_MORE_DATA) 
		{
			_tprintf(_T("ReadFile failed w/err 0x%08lx\n"), GetLastError());
			//break;
		}

		//_tprintf(_T("Receives %ld bytes; Message: \"%s\"\n"), 
		//	cbBytesRead, chReply);

	}/* while (!bResult);  // Repeat loop if ERROR_MORE_DATA */


	/////////////////////////////////////////////////////////////////////////
	// Close the pipe.
	// 

//	CloseHandle(hPipe); 
	}
};

bool CDCClient::Execute(LPCWSTR sFile)
{
	//return socket_utils::SendFile(sFile);
	return wnd_utils::Execute(sFile);

	//return pipe_utils::Execute(sFile);
}



