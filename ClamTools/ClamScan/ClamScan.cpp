#include "stdafx.h"
#include "ClamScan.h"
#include "openssl/evp.h"
#include <strsafe.h>

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CWinApp theApp;
using namespace std;

bool running = true;

UINT OnScan(LPVOID pParam)
{
	OpenSSL_add_all_digests();

	std::string sCipherName[3];
	sCipherName[0] = "md5";
	sCipherName[1] = "sha256";
	sCipherName[2] = "ripemd160";

	std::string sFileName[4];
	
	//sFileName[3] = "c:\\strobist8-1.mkv";
	sFileName[2] = "c:\\avg_free_stf_en_90_730a1834.exe";
	sFileName[1] = "c:\\WINDOWS\\system32\\shell32.dll";
	sFileName[0] = "c:\\Sondering\\Sondering\\Bin\\stlportd.5.1.dll";

	int nCounts[3];
	nCounts[2] = 10;
	nCounts[1] = 100;
	nCounts[0] = 400;

	for(int ci = 0; ci < 3; ++ci)
	{
		const EVP_MD *md = EVP_get_digestbyname(sCipherName[ci].c_str());
		printf("%s\n", sCipherName[ci].c_str());

		for(int fi = 2; fi < 3; ++fi)
		{
			EVP_MD_CTX mdctx;

			unsigned char md_value[EVP_MAX_MD_SIZE];
			unsigned int md_len;

			FILE *pFile;
			fopen_s(&pFile, sFileName[fi].c_str(), "rb");
			fseek(pFile, 0L, SEEK_END);
			long lSize = ftell(pFile);
			fseek(pFile, 0L, SEEK_SET);
			
			char *data = (char *)malloc(lSize);
			fread(data, sizeof(char), lSize, pFile);
			fclose(pFile);

			int nCount = nCounts[fi];
			time_t start, stop;
			time(&start);
			for(int i = 0; i < nCount; ++i)
			{
				EVP_MD_CTX_init(&mdctx);
				EVP_DigestInit_ex(&mdctx, md, NULL);
				EVP_DigestUpdate(&mdctx, data, lSize);
				EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
				EVP_MD_CTX_cleanup(&mdctx);
			}
			time(&stop);

			free((void *)data);

			double dDiff = difftime(stop, start) / nCount;
			double dSize = (double)lSize / (1024*1024);

			printf("Laikas: %.10f | Failo dydis: %.3f\n", dDiff, dSize);
		}
	}

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

#define strPipeName "\\\\.\\pipe\\HelloWorld"
//Pipe name format - \\.\pipe\pipename


#define BUFFER_SIZE 1024 //1k
#define ACK_MESG_RECV "Message received successfully"



int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		//CWinThread *pThread = AfxBeginThread(OnScan, (LPVOID)nRetCode);
		//pThread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);
		//WaitForSingleObject(pThread->m_hThread, INFINITE);

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

		SOCKET client;
		client = accept(s, NULL, NULL);

		if(client == INVALID_SOCKET)
		{
			closesocket(s);
			WSACleanup();
			return 0;
		}

		while (!b_Done)
		{
			CString sMessage;
			sMessage = Readline(&client);
			char szScanBuffer[3];
			sprintf(szScanBuffer, "%d\n", 1);
			int nSize = strlen(szScanBuffer);
			int nGetSize = send(client, szScanBuffer, nSize, 0);
		}

		closesocket(s);

		WSACleanup();		

	//SECURITY_ATTRIBUTES sa;
	//sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR)malloc(
	//	SECURITY_DESCRIPTOR_MIN_LENGTH);
	//InitializeSecurityDescriptor(sa.lpSecurityDescriptor, 
	//	SECURITY_DESCRIPTOR_REVISION);
	//// ACL is set as NULL in order to allow all access to the object.
	//SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, NULL, FALSE);
	//sa.nLength = sizeof(sa);
	//sa.bInheritHandle = TRUE;

	//// Create the named pipe.
	//HANDLE hPipe = CreateNamedPipe(
	//	strPipeName,				// The unique pipe name. This string must 
	//								// have the form of \\.\pipe\pipename
	//	PIPE_ACCESS_DUPLEX,			// The pipe is bi-directional; both  
	//								// server and client processes can read 
	//								// from and write to the pipe
	//	PIPE_TYPE_MESSAGE |			// Message type pipe 
	//	PIPE_READMODE_MESSAGE |		// Message-read mode 
	//	PIPE_WAIT,					// Blocking mode is enabled
	//	PIPE_UNLIMITED_INSTANCES,	// Max. instances

	//	// These two buffer sizes have nothing to do with the buffers that 
	//	// are used to read from or write to the messages. The input and 
	//	// output buffer sizes are advisory. The actual buffer size reserved 
	//	// for each end of the named pipe is either the system default, the 
	//	// system minimum or maximum, or the specified size rounded up to the 
	//	// next allocation boundary. The buffer size specified should be 
	//	// small enough that your process will not run out of nonpaged pool, 
	//	// but large enough to accommodate typical requests.
	//	BUFFER_SIZE,				// Output buffer size in bytes
	//	BUFFER_SIZE,				// Input buffer size in bytes

	//	NMPWAIT_USE_DEFAULT_WAIT,	// Time-out interval
	//	&sa							// Security attributes
	//	);

	//if (hPipe == INVALID_HANDLE_VALUE)
	//{
	//	_tprintf(_T("Unable to create named pipe %s w/err 0x%08lx\n"), 
	//		strPipeName, GetLastError());
	//	return 1;
	//}
	//_tprintf(_T("The named pipe, %s, is created.\n"), strPipeName);


	///////////////////////////////////////////////////////////////////////////
	//// Wait for the client to connect.
	//// 
	//_putts(_T("Waiting for the client's connection..."));

	//BOOL bConnected = ConnectNamedPipe(hPipe, NULL) ? 
	//	TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

	//if (!bConnected)
	//{
	//	_tprintf(_T(
	//		"Error occurred while connecting to the client: 0x%08lx\n"
	//		), GetLastError()); 
	//	CloseHandle(hPipe);
	//	return 1;
	//}


	///////////////////////////////////////////////////////////////////////////
	//// Read client requests from the pipe and write the response.
	//// 
	//
	//// A char buffer of BUFFER_SIZE chars, aka BUFFER_SIZE * sizeof(TCHAR) 
	//// bytes. The buffer should be big enough for ONE request from a client.

	//TCHAR chRequest[BUFFER_SIZE];	// Client -> Server
	//DWORD cbBytesRead, cbRequestBytes;
	//TCHAR chReply[BUFFER_SIZE];		// Server -> Client
	//DWORD cbBytesWritten, cbReplyBytes;

	//BOOL bResult;

	//while (TRUE)
	//{
	//	// Receive one message from the pipe.

	//	cbRequestBytes = sizeof(TCHAR) * BUFFER_SIZE;
	//	bResult = ReadFile(			// Read from the pipe.
	//		hPipe,					// Handle of the pipe
	//		chRequest,				// Buffer to receive data
	//		cbRequestBytes,			// Size of buffer in bytes
	//		&cbBytesRead,			// Number of bytes read
	//		NULL);					// Not overlapped I/O

	//	if (!bResult/*Failed*/ || cbBytesRead == 0/*Finished*/) 
	//		break;
	//	
	//	//_tprintf(_T("Receives %ld bytes; Message: \"%s\"\n"), 
	//	//	cbBytesRead, chRequest);

	//	// Prepare the response.

	//	StringCchCopy(
	//		chReply, BUFFER_SIZE, _T("Default response from server"));
	//	cbReplyBytes = sizeof(TCHAR) * (lstrlen(chReply) + 1);

	//	// Write the response to the pipe.

	//	bResult = WriteFile(		// Write to the pipe.
	//		hPipe,					// Handle of the pipe
	//		chReply,				// Buffer to write to 
	//		cbReplyBytes,			// Number of bytes to write 
	//		&cbBytesWritten,		// Number of bytes written 
	//		NULL);					// Not overlapped I/O 

	//	if (!bResult/*Failed*/ || cbReplyBytes != cbBytesWritten/*Failed*/) 
	//	{
	//		_tprintf(_T("WriteFile failed w/err 0x%08lx\n"), GetLastError());
	//		break;
	//	}

	//	//_tprintf(_T("Replies %ld bytes; Message: \"%s\"\n"), 
	//	//	cbBytesWritten, chReply);
	//}


	///////////////////////////////////////////////////////////////////////////
	//// Flush the pipe to allow the client to read the pipe's contents before
	//// disconnecting. Then disconnect the pipe, and close the handle to this 
	//// pipe instance. 
	//// 

	//FlushFileBuffers(hPipe); 
	//DisconnectNamedPipe(hPipe); 
	//CloseHandle(hPipe);

	}

	return nRetCode;
}
