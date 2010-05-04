/*
Module : NPIPE.CPP
Purpose: Defines the implementation for an MFC wrapper class
         for Win32 named Pipes
Created: PJN / 2-08-1998
History: None

Copyright (c) 1998 by PJ Naughter.  
All rights reserved.

*/


/////////////////////////////////  Includes  //////////////////////////////////
#include "stdafx.h"
#include "npipe.h"



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



////////////////////////////////// CNamedPipe Implementation //////////////////

IMPLEMENT_DYNAMIC(CNamedPipe, CObject)

CNamedPipe::CNamedPipe()
{
  m_hPipe = INVALID_HANDLE_VALUE;
}

CNamedPipe::~CNamedPipe()
{
  Close();
}

BOOL CNamedPipe::Create(LPCTSTR lpszName, DWORD dwOpenMode, DWORD dwPipeMode, 
                        DWORD dwMaxInstances, DWORD dwOutBufferSize, 
                        DWORD dwInBufferSize, DWORD dwDefaultTimeOut, 
                        LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
  ASSERT(!IsOpen());
  ASSERT(_tcslen(lpszName));

  //the class encapsulates creating the pipe name, all that is required is
  //a simple name for the mailslot e.g. lpName = PJPIPE will create the pipe
  //name \\.\pipe\PJPIPE
  TCHAR pszPipeName[_MAX_PATH];
  _tcscpy(pszPipeName, _T("\\\\.\\PIPE\\"));
  _tcscat(pszPipeName, lpszName);

  m_hPipe = ::CreateNamedPipe(pszPipeName, dwOpenMode, dwPipeMode, dwMaxInstances, 
                            dwOutBufferSize, dwInBufferSize, dwDefaultTimeOut, lpSecurityAttributes);
  if (m_hPipe == INVALID_HANDLE_VALUE)
    TRACE1("CNamedPipe::Create() failed, GetLastError returned %d\n", ::GetLastError());

  return (m_hPipe != INVALID_HANDLE_VALUE);
}

BOOL CNamedPipe::Open(LPCTSTR lpszServerName, LPCTSTR lpszPipeName, DWORD dwDesiredAccess, 
                      DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
                      DWORD dwFlagsAndAttributes)
{
  ASSERT(!IsOpen());
  ASSERT(_tcslen(lpszServerName));
  ASSERT(_tcslen(lpszPipeName));

  //Construct the canonical pipe name
  CString sPipeName;
  sPipeName.Format(_T("\\\\%s\\PIPE\\%s"), lpszServerName, lpszPipeName);

  m_hPipe = CreateFile(sPipeName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
                       OPEN_EXISTING, dwFlagsAndAttributes, NULL);

  if (m_hPipe == INVALID_HANDLE_VALUE)
    TRACE1("CNamedPipe::Open() failed, GetLastError returned %d\n", ::GetLastError());

  return (m_hPipe != INVALID_HANDLE_VALUE);
}

BOOL CNamedPipe::Attach(HANDLE hPipe)
{
  if (m_hPipe != hPipe)
    Close();

  m_hPipe = hPipe;
  return TRUE;
}

HANDLE CNamedPipe::Detach()
{
  HANDLE hReturn = m_hPipe;
  m_hPipe = INVALID_HANDLE_VALUE;
  return hReturn;
}

BOOL CNamedPipe::Close()
{
  BOOL bSuccess = TRUE;
  if (IsOpen())
  {
    bSuccess = ::CloseHandle(m_hPipe);
    if (!bSuccess)
      TRACE1("CNamedPipe::Close() failed, GetLastError returned %d\n", ::GetLastError());
  }

  return bSuccess;
}

BOOL CNamedPipe::ConnectClient(LPOVERLAPPED lpOverlapped)
{
  ASSERT(m_hPipe != NULL); //Pipe must be open
#ifdef _DEBUG
  BOOL bServerPipe;
#endif
  ASSERT(IsServerPipe(bServerPipe));
  ASSERT(bServerPipe);     //Must be called from the server side

  BOOL bSuccess = ::ConnectNamedPipe(m_hPipe, lpOverlapped);
  if (!bSuccess)
    TRACE1("CNamedPipe::ConnectClient() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

BOOL CNamedPipe::DisconnectClient()
{
  ASSERT(m_hPipe != NULL); //Pipe must be open
#ifdef _DEBUG
  BOOL bServerPipe;
#endif
  ASSERT(IsServerPipe(bServerPipe));
  ASSERT(bServerPipe);     //Must be called from the server side

  BOOL bSuccess = ::DisconnectNamedPipe(m_hPipe);
  if (!bSuccess)
    TRACE1("CNamedPipe::DisconnectClient() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

BOOL CNamedPipe::Flush()
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  BOOL bSuccess = ::FlushFileBuffers(m_hPipe);
  if (!bSuccess)
    TRACE1("CNamedPipe::Flush() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

BOOL CNamedPipe::Write(LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite, 
                       DWORD& dwNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  BOOL bSuccess = ::WriteFile(m_hPipe, lpBuffer, dwNumberOfBytesToWrite, 
                              &dwNumberOfBytesWritten, lpOverlapped);
  if (!bSuccess)
    TRACE1("CNamedPipe::Write() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

BOOL CNamedPipe::Write(LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite, 
                       LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  BOOL bSuccess = ::WriteFileEx(m_hPipe, lpBuffer, dwNumberOfBytesToWrite, 
                                lpOverlapped, lpCompletionRoutine);
  if (!bSuccess)
    TRACE1("CNamedPipe::Write() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

BOOL CNamedPipe::Read(LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, 
                      DWORD& dwNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  BOOL bSuccess = ::ReadFile(m_hPipe, lpBuffer, dwNumberOfBytesToRead, 
                             &dwNumberOfBytesRead, lpOverlapped);
  if (!bSuccess)
    TRACE1("CNamedPipe::Read() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

BOOL CNamedPipe::Read(LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, 
                      LPOVERLAPPED lpOverlapped,  LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  BOOL bSuccess = ::ReadFileEx(m_hPipe, lpBuffer, dwNumberOfBytesToRead, 
                               lpOverlapped, lpCompletionRoutine);
  if (!bSuccess)
    TRACE1("CNamedPipe::Read() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

BOOL CNamedPipe::Peek(LPVOID lpBuffer, DWORD dwBufferSize, DWORD& dwBytesRead, 
                      DWORD& dwTotalBytesAvail, DWORD& dwBytesLeftThisMessage)
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  BOOL bSuccess = ::PeekNamedPipe(m_hPipe, lpBuffer, dwBufferSize, &dwBytesRead, 
                                  &dwTotalBytesAvail, &dwBytesLeftThisMessage);
  if (!bSuccess)
    TRACE1("CNamedPipe::Peek() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

BOOL CNamedPipe::Transact(LPVOID lpInBuffer, DWORD dwInBufferSize, LPVOID lpOutBuffer,
                          DWORD dwOutBufferSize, DWORD& dwBytesRead, LPOVERLAPPED lpOverlapped)
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  BOOL bSuccess = ::TransactNamedPipe(m_hPipe, lpInBuffer, dwInBufferSize, lpOutBuffer, 
                                      dwOutBufferSize, &dwBytesRead, lpOverlapped);
  if (!bSuccess)
    TRACE1("CNamedPipe::Transact() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

BOOL CNamedPipe::IsBlockingPipe(BOOL& bIsBlocking) const
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  DWORD dwState;
  BOOL bSuccess = ::GetNamedPipeHandleState(m_hPipe, &dwState, NULL, NULL, NULL, NULL, 0);
                                      
  if (!bSuccess)
    TRACE1("CNamedPipe::IsBlockingPipe() failed, GetLastError returned %d\n", ::GetLastError());
  else
    bIsBlocking = ((dwState & PIPE_NOWAIT) == 0);

  return bSuccess;
}

BOOL CNamedPipe::IsClientPipe(BOOL& bClientPipe) const
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  DWORD dwFlags;
  BOOL bSuccess = ::GetNamedPipeInfo(m_hPipe, &dwFlags, NULL, NULL, NULL);
                                      
  if (!bSuccess)
    TRACE1("CNamedPipe::IsClientPipe() failed, GetLastError returned %d\n", ::GetLastError());
  else
    bClientPipe = ((dwFlags & PIPE_CLIENT_END) != 0);

  return bSuccess;
}

BOOL CNamedPipe::IsServerPipe(BOOL& bServerPipe) const
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  DWORD dwFlags;
  BOOL bSuccess = ::GetNamedPipeInfo(m_hPipe, &dwFlags, NULL, NULL, NULL);
                                      
  if (!bSuccess)
    TRACE1("CNamedPipe::IsServerPipe() failed, GetLastError returned %d\n", ::GetLastError());
  else
    bServerPipe = ((dwFlags & PIPE_SERVER_END) != 0);

  return bSuccess;
}

BOOL CNamedPipe::IsMessagePipe(BOOL& bMessagePipe) const
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  DWORD dwState;
  BOOL bSuccess = ::GetNamedPipeHandleState(m_hPipe, &dwState, NULL, NULL, NULL, NULL, 0);
                                      
  if (!bSuccess)
    TRACE1("CNamedPipe::IsMessagePipe() failed, GetLastError returned %d\n", ::GetLastError());
  else
    bMessagePipe = ((dwState & PIPE_READMODE_MESSAGE) != 0);

  return bSuccess;
}

DWORD CNamedPipe::GetCurrentInstances() const
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  DWORD dwCurInstances = 0;
  BOOL bSuccess = ::GetNamedPipeHandleState(m_hPipe, NULL, &dwCurInstances, NULL, NULL, NULL, 0);
                                      
  if (!bSuccess)
    TRACE1("CNamedPipe::GetCurrentInstances() failed, GetLastError returned %d\n", ::GetLastError());

  return dwCurInstances;
}

DWORD CNamedPipe::GetMaxCollectionCount() const
{
  ASSERT(m_hPipe != NULL); //Pipe must be open
#ifdef _DEBUG
  BOOL bClientPipe;
#endif
  ASSERT(IsClientPipe(bClientPipe));
  ASSERT(bClientPipe);     //Must be called from the client side

  DWORD dwMaxCollectionCount = 0;
  BOOL bSuccess = ::GetNamedPipeHandleState(m_hPipe, NULL, NULL, &dwMaxCollectionCount, NULL, NULL, 0);
                                      
  if (!bSuccess)
    TRACE1("CNamedPipe::GetMaxCollectionCount() failed, GetLastError returned %d\n", ::GetLastError());

  return dwMaxCollectionCount;
}

DWORD CNamedPipe::GetCollectionTimeout() const
{
  ASSERT(m_hPipe != NULL); //Pipe must be open
#ifdef _DEBUG
  BOOL bClientPipe;
#endif
  ASSERT(IsClientPipe(bClientPipe));
  ASSERT(bClientPipe);     //Must be called from the client side

  DWORD dwCollectDataTimeout = 0;
  BOOL bSuccess = ::GetNamedPipeHandleState(m_hPipe, NULL, NULL, NULL, &dwCollectDataTimeout, NULL, 0);
                                      
  if (!bSuccess)
    TRACE1("CNamedPipe::GetCollectionTimeout() failed, GetLastError returned %d\n", ::GetLastError());

  return dwCollectDataTimeout;
}

DWORD CNamedPipe::GetOutboundBufferSize() const
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  DWORD dwOutBufferSize = 0;
  BOOL bSuccess = ::GetNamedPipeInfo(m_hPipe, NULL, &dwOutBufferSize, NULL, NULL);
                                      
  if (!bSuccess)
    TRACE1("CNamedPipe::GetOutboundBufferSize() failed, GetLastError returned %d\n", ::GetLastError());

  return dwOutBufferSize;
}

DWORD CNamedPipe::GetInboundBufferSize() const
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  DWORD dwInBufferSize = 0;
  BOOL bSuccess = ::GetNamedPipeInfo(m_hPipe, NULL, NULL, &dwInBufferSize, NULL);
                                      
  if (!bSuccess)
    TRACE1("CNamedPipe::GetInboundBufferSize() failed, GetLastError returned %d\n", ::GetLastError());

  return dwInBufferSize;
}

CString CNamedPipe::GetClientUserName() const
{
  ASSERT(m_hPipe != NULL); //Pipe must be open
#ifdef _DEBUG
  BOOL bServerPipe;
#endif
  ASSERT(IsServerPipe(bServerPipe));
  ASSERT(bServerPipe);     //Must be called from the server side

  TCHAR pszUserName[_MAX_PATH];
  BOOL bSuccess = ::GetNamedPipeHandleState(m_hPipe, NULL, NULL, NULL, NULL, pszUserName, _MAX_PATH);
  
  CString sName;
  if (!bSuccess)
    TRACE1("CNamedPipe::GetClientUserName() failed, GetLastError returned %d\n", ::GetLastError());
  else
    sName = pszUserName;

  return sName;
}

DWORD CNamedPipe::GetMaxInstances() const
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  DWORD dwMaxInstances = 0;
  BOOL bSuccess = ::GetNamedPipeInfo(m_hPipe, NULL, NULL, NULL, &dwMaxInstances);
                                      
  if (!bSuccess)
    TRACE1("CNamedPipe::GetMaxInstances() failed, GetLastError returned %d\n", ::GetLastError());

  return dwMaxInstances;
}

BOOL CNamedPipe::SetMode(BOOL bByteMode, BOOL bBlockingMode)
{
  ASSERT(m_hPipe != NULL); //Pipe must be open

  DWORD dwMode;
  if (bByteMode)
  {
    if (bBlockingMode)
      dwMode = PIPE_READMODE_BYTE | PIPE_WAIT;
    else
      dwMode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
  }
  else
  {
    if (bBlockingMode)
      dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
    else
      dwMode = PIPE_READMODE_MESSAGE | PIPE_NOWAIT;
  }

  BOOL bSuccess = ::SetNamedPipeHandleState(m_hPipe, &dwMode, NULL, NULL);
                                      
  if (!bSuccess)
    TRACE1("CNamedPipe::SetMode() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

BOOL CNamedPipe::SetMaxCollectionCount(DWORD dwCollectionCount)
{
  ASSERT(m_hPipe != NULL); //Pipe must be open
#ifdef _DEBUG
  BOOL bClientPipe;
#endif
  ASSERT(IsClientPipe(bClientPipe));
  ASSERT(bClientPipe);     //Must be called from the client side

  BOOL bSuccess = ::SetNamedPipeHandleState(m_hPipe, NULL, &dwCollectionCount, NULL);
                                      
  if (!bSuccess)
    TRACE1("CNamedPipe::SetMaxCollectionCount() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

BOOL CNamedPipe::SetCollectionTimeout(DWORD dwDataTimeout)
{
  ASSERT(m_hPipe != NULL); //Pipe must be open
#ifdef _DEBUG
  BOOL bClientPipe;
#endif
  ASSERT(IsClientPipe(bClientPipe));
  ASSERT(bClientPipe);     //Must be called from the client side

  BOOL bSuccess = ::SetNamedPipeHandleState(m_hPipe, NULL, NULL, &dwDataTimeout);
                                      
  if (!bSuccess)
    TRACE1("CNamedPipe::SetCollectionTimeout() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

BOOL CNamedPipe::Call(LPCTSTR lpszServerName, LPCTSTR lpszPipeName, LPVOID lpInBuffer, 
                      DWORD dwInBufferSize, LPVOID lpOutBuffer, DWORD dwOutBufferSize, 
                      DWORD& dwBytesRead, DWORD dwTimeOut)
{
  //Construct the canonical pipe name
  CString sPipeName;
  sPipeName.Format(_T("\\\\%s\\PIPE\\%s"), lpszServerName, lpszPipeName);

  BOOL bSuccess = ::CallNamedPipe(sPipeName, lpInBuffer, dwInBufferSize, lpOutBuffer, 
                                  dwOutBufferSize, &dwBytesRead, dwTimeOut);

  if (!bSuccess)
    TRACE1("CNamedPipe::Call() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

BOOL CNamedPipe::ServerAvailable(LPCTSTR lpszServerName, LPCTSTR lpszPipeName, DWORD dwTimeOut)
{
  //Construct the canonical pipe name
  CString sPipeName;
  sPipeName.Format(_T("\\\\%s\\PIPE\\%s"), lpszServerName, lpszPipeName);

  BOOL bSuccess = ::WaitNamedPipe(sPipeName, dwTimeOut);

  if (!bSuccess)
    TRACE1("CNamedPipe::ServerAvailable() failed, GetLastError returned %d\n", ::GetLastError());

  return bSuccess;
}

#ifdef _DEBUG
void CNamedPipe::AssertValid() const
{
  CObject::AssertValid();
  ASSERT(IsOpen());
}
#endif

#ifdef _DEBUG
void CNamedPipe::Dump(CDumpContext& dc) const
{
  CObject::Dump(dc);

  CString sText;
  sText.Format(_T("Open=%d, m_hPipe=%x\n"), IsOpen(), m_hPipe);
  dc << sText;
}
#endif





