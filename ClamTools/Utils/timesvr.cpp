#include <stdio.h>
#include <afxwin.h>
#include <afxext.h>
#include "..\npipe.h"


void main()
{
  _tprintf(_T("Time Server demo program has started...\n"));

  //Create the server pipe   CreateNamedPipe
  CNamedPipe serverPipe;
  if (!serverPipe.Create(_T("TEST_PJTIME"), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_WAIT, 1, 4096, 4096, 1, NULL))
  {
    _tprintf(_T("Failed to create the server pipe, make sure you are running on NT, not 95 or 98\n"));
    return;
  }

  //Test the state accessor functions
  ///*
  BOOL bClocking;
  BOOL bSuccess = serverPipe.IsBlockingPipe(bClocking);
  BOOL bClient;
  bSuccess = serverPipe.IsClientPipe(bClient);
  BOOL bServer;
  bSuccess = serverPipe.IsServerPipe(bServer);
  BOOL bMessage;
  bSuccess = serverPipe.IsMessagePipe(bMessage);
  DWORD dwInstances = serverPipe.GetCurrentInstances();
  //DWORD dwCollectionCount = serverPipe.GetMaxCollectionCount();
  //DWORD dwCollectionTimeout = serverPipe.GetCollectionTimeout();
  DWORD dwOutboundBufferSize = serverPipe.GetOutboundBufferSize();
  DWORD dwInboundBufferSize = serverPipe.GetInboundBufferSize();
  DWORD dwMaxInstances = serverPipe.GetMaxInstances();
  //*/

  //Test the state mutator functions
  //*
  bSuccess = serverPipe.SetMode(FALSE, TRUE);
  bSuccess = serverPipe.IsMessagePipe(bMessage);
  //*/

  while (1)
  {
    if (!serverPipe.ConnectClient())
    {
      _tprintf(_T("Failed in call to connect client\n"));
      return;
    }
    CString sClient = serverPipe.GetClientUserName();
    _tprintf(_T("%s Connected\n"), sClient);

    SYSTEMTIME st;
    ZeroMemory(&st, sizeof(SYSTEMTIME));
    GetLocalTime(&st);

    DWORD dwBytesWritten;
    serverPipe.Write(&st, sizeof(SYSTEMTIME), dwBytesWritten);

    TCHAR sTime[256];
    GetTimeFormat(LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE, &st, NULL, sTime, 256);
    TCHAR sDate[256];
    GetDateFormat(LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE, &st, NULL, sDate, 256);
    _tprintf(_T("  Sending time %s %s\n"), sDate, sTime);

    if (!serverPipe.DisconnectClient())
    {
      _tprintf(_T("Failed in call to disconnect client\n"));
      return;
    }
    _tprintf(_T("  Disconnecting\n"));
  }

}