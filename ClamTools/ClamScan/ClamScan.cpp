#include "stdafx.h"
#include "ClamScan.h"
//#include "openssl/evp.h"
#include <strsafe.h>

#include "../Utils/SendObj.h"

#include "../Utils/PipeUtils.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CWinApp theApp;
using namespace std;

bool running = true;

//UINT OnScan(LPVOID pParam)
//{
//	OpenSSL_add_all_digests();
//
//	std::string sCipherName[3];
//	sCipherName[0] = "md5";
//	sCipherName[1] = "sha256";
//	sCipherName[2] = "ripemd160";
//
//	std::string sFileName[4];
//	
//	//sFileName[3] = "c:\\strobist8-1.mkv";
//	sFileName[2] = "c:\\avg_free_stf_en_90_730a1834.exe";
//	sFileName[1] = "c:\\WINDOWS\\system32\\shell32.dll";
//	sFileName[0] = "c:\\Sondering\\Sondering\\Bin\\stlportd.5.1.dll";
//
//	int nCounts[3];
//	nCounts[2] = 10;
//	nCounts[1] = 100;
//	nCounts[0] = 400;
//
//	for(int ci = 0; ci < 3; ++ci)
//	{
//		const EVP_MD *md = EVP_get_digestbyname(sCipherName[ci].c_str());
//		printf("%s\n", sCipherName[ci].c_str());
//
//		for(int fi = 2; fi < 3; ++fi)
//		{
//			EVP_MD_CTX mdctx;
//
//			unsigned char md_value[EVP_MAX_MD_SIZE];
//			unsigned int md_len;
//
//			FILE *pFile;
//			fopen_s(&pFile, sFileName[fi].c_str(), "rb");
//			fseek(pFile, 0L, SEEK_END);
//			long lSize = ftell(pFile);
//			fseek(pFile, 0L, SEEK_SET);
//			
//			char *data = (char *)malloc(lSize);
//			fread(data, sizeof(char), lSize, pFile);
//			fclose(pFile);
//
//			int nCount = nCounts[fi];
//			time_t start, stop;
//			time(&start);
//			for(int i = 0; i < nCount; ++i)
//			{
//				EVP_MD_CTX_init(&mdctx);
//				EVP_DigestInit_ex(&mdctx, md, NULL);
//				EVP_DigestUpdate(&mdctx, data, lSize);
//				EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
//				EVP_MD_CTX_cleanup(&mdctx);
//			}
//			time(&stop);
//
//			free((void *)data);
//
//			double dDiff = difftime(stop, start) / nCount;
//			double dSize = (double)lSize / (1024*1024);
//
//			printf("Laikas: %.10f | Failo dydis: %.3f\n", dDiff, dSize);
//		}
//	}
//
//	return 0;
//}

//CString Readline(SOCKET *client)
//{
//	CString sMess;
//	char buffer;
//	int rVal;
//
//	while(true)
//	{
//		rVal = recv(*(client), &buffer, 1, 0);
//		if(rVal == SOCKET_ERROR)
//		{
//			return "";
//			WSACleanup();
//		}
//		
//		if(buffer != '\n')
//		{
//			sMess += buffer;
//		}
//		else
//		{
//			break;
//		}
//	}
//
//	return sMess;
//}

//#define strPipeName "\\\\.\\pipe\\HelloWorld"
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
			HWND hwnd = NULL;
			hwnd = FindWindow(NULL, "DCAntiVirusScan");

			CSendObj obj;
			strcpy_s(obj.m_sPath, MAX_PATH, "c:\\WINDOWS\\system32\\shell32.dll");
			COPYDATASTRUCT copy;

			copy.dwData = 1;
			copy.cbData = sizeof(obj);
			copy.lpData = &obj;

			LRESULT result = SendMessage(hwnd,
										 WM_COPYDATA,
										 0,
										 (LPARAM) (LPVOID) &copy);
			int k;
			k++;
		//CWinThread *pThread = AfxBeginThread(OnScan, (LPVOID)nRetCode);
		//pThread->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);
		//WaitForSingleObject(pThread->m_hThread, INFINITE);

		//WORD sockVersion;
		//WSADATA wsaData;
		//int rVal;

		//sockVersion = MAKEWORD(2,2);
		//WSAStartup(sockVersion, &wsaData);

		//SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		//if(s == INVALID_SOCKET)
		//{
		//	closesocket(s);
		//	WSACleanup();
		//	return 0;
		//}

		//SOCKADDR_IN sin;
		//sin.sin_family = AF_INET;
		//sin.sin_port = htons(8888);
		//sin.sin_addr.s_addr = inet_addr("127.0.0.1");

		//rVal = bind(s, (LPSOCKADDR)&sin, sizeof(sin));
		//if(rVal == SOCKET_ERROR)
		//{
		//	closesocket(s);
		//	WSACleanup();
		//	return 0;
		//}

		//rVal = listen(s, 2);
		//if(rVal == SOCKET_ERROR)
		//{
		//	closesocket(s);
		//	WSACleanup();
		//	return 0;
		//}

		//bool b_Done(false);

		//SOCKET client;
		//client = accept(s, NULL, NULL);

		//if(client == INVALID_SOCKET)
		//{
		//	closesocket(s);
		//	WSACleanup();
		//	return 0;
		//}

		//while (!b_Done)
		//{
		//	CString sMessage;
		//	sMessage = Readline(&client);
		//	char szScanBuffer[3];
		//	sprintf(szScanBuffer, "%d\n", 1);
		//	int nSize = strlen(szScanBuffer);
		//	int nGetSize = send(client, szScanBuffer, nSize, 0);
		//}

		//closesocket(s);

		//WSACleanup();		

//   HANDLE hPipe; 
//   BOOL   fSuccess = FALSE; 
//   DWORD  cbRead, cbWritten, dwMode; 
//
//
//   while (1) 
//   { 
//      hPipe = CreateFile( 
//         strPipeName,   // pipe name 
//         GENERIC_READ |  // read and write access 
//         GENERIC_WRITE, 
//         0,              // no sharing 
//         NULL,           // default security attributes
//         OPEN_EXISTING,  // opens existing pipe 
//         0,              // default attributes 
//         NULL);          // no template file 
// 
//      if(hPipe != INVALID_HANDLE_VALUE) 
//	  {
//         break; 
//	  }
// 
// 
//      if(GetLastError() != ERROR_PIPE_BUSY) 
//      {
//         return -1;
//      }
// 
//      // All pipe instances are busy, so wait for 20 seconds. 
// 
//      if(!WaitNamedPipe(strPipeName, 100)) 
//      { 
//         return -1;
//      } 
//   } 
// 
//// The pipe connected; change to message-read mode. 
// 
//   dwMode = PIPE_READMODE_MESSAGE; 
//   fSuccess = SetNamedPipeHandleState( 
//      hPipe,    // pipe handle 
//      &dwMode,  // new pipe mode 
//      NULL,     // don't set maximum bytes 
//      NULL);    // don't set maximum time 
//   if ( ! fSuccess) 
//   {
//      return -1;
//   }
// 
//	
//   char swrite[MAX_PATH];
//   strcpy_s(swrite, MAX_PATH, "Tekstas");
//
//   fSuccess = WriteFile( 
//      hPipe,                  // pipe handle 
//      swrite,             // message 
//      MAX_PATH*sizeof(char),              // message length 
//      &cbWritten,             // bytes written 
//      NULL);                  // not overlapped 
//
//   if ( ! fSuccess) 
//   {
//      return -1;
//   }
//
//   do 
//   { 
//   // Read from the pipe. 
//		char s[MAX_PATH];
//      fSuccess = ReadFile( 
//         hPipe,    // pipe handle 
//         &s,    // buffer to receive reply 
//         MAX_PATH*sizeof(char),  // size of buffer 
//         &cbRead,  // number of bytes read 
//         NULL);    // not overlapped 
// 
//      if ( ! fSuccess && GetLastError() != ERROR_MORE_DATA )
//         break; 
// 
//   } while ( ! fSuccess);  // repeat loop if ERROR_MORE_DATA 
//
//   if ( ! fSuccess)
//   {
//      return -1;
//   }
//
// 
//   CloseHandle(hPipe); 
	}

	return nRetCode;
}
