#include "stdafx.h"
#include "resource.h"
#include "ntserv_msg.h"
#include "app.h"

#include "PipeServerUtils.h" 

#include "../Utils/Scanner/FileHashDBUtils.h"
#include "../Utils/Log.h"
#include "../Utils/npipe.h"
#include "../Utils/SendObj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define _TEST_

CApp theApp;
BOOL CApp::InitInstance()
{

#ifdef _TEST_ 
	
	CScannedFileMap *pFilesMap = new CScannedFileMap;
	file_hash_DB_utils::ReadPassData(pFilesMap);

	bool bCriticalSectionOpened = file_hash_DB_utils::CreateHashDBCriticalSection(pFilesMap);

	if(bCriticalSectionOpened)
	{
		//Starting real scan pipes server
		//pRealScanThread = AfxBeginThread(pipe_server_utils::RealScanServer, (LPVOID)pFilesMap);
		//Starting manual scan pipes server
		pipe_server_utils::ManualScanServer((LPVOID)pFilesMap);
	}

#else

	CNTServiceCommandLineInfo cmdInfo;
	CMyService Service;
	Service.ParseCommandLine(cmdInfo);
	Service.ProcessShellCommand(cmdInfo);

#endif

	return FALSE;
}


CMyService::CMyService() : CNTService(sgServiceName, sgServiceDisplayName,  SERVICE_ACCEPT_STOP, sgServiceDescription) 
{
	m_bWantStop = FALSE;
	m_bPaused = FALSE;
	m_dwBeepInternal = 1000;
}

void CMyService::StopScanServer(LPCSTR sServerName)
{
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

	CSendObj obj;
	obj.m_nType = EQuitServer;
	clientPipe.Write(&obj, sizeof(CSendObj), dwBytes);
	clientPipe.Close();
}

void CMyService::ServiceMain(DWORD /*dwArgc*/, LPTSTR* /*lpszArgv*/)
{
	//register our control handler
	RegisterCtrlHandler();

	//Pretend that starting up takes some time
	ReportStatusToSCM(SERVICE_START_PENDING, NO_ERROR, 0, 1, 0);
	
	CScannedFileMap *pFilesMap = new CScannedFileMap;
	scan_log_utils::LogHeader("Loading hash DB", GetCurrentProcessId());
	CPrecisionTimer timer;
	timer.Start();
	file_hash_DB_utils::ReadPassData(pFilesMap);
	double dSec = timer.Stop();
	scan_log_utils::LogTime("Load time", dSec);

	//Lock hash DB file
	HANDLE hDataFile = CreateFile(path_utils::GetDataFilePath(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD low, high;
	low = GetFileSize(hDataFile, &high);
	LockFile(hDataFile, 0, 0, low, high);
	
	ReportStatusToSCM(SERVICE_RUNNING, NO_ERROR, 0, 1, 0);

	CWinThread *pRealScanThread = NULL;
	CWinThread *pManualScanThread = NULL;
	bool bCriticalSectionOpened = file_hash_DB_utils::CreateHashDBCriticalSection(pFilesMap);
	if(bCriticalSectionOpened)
	{
		//Starting real scan pipes server
		pRealScanThread = AfxBeginThread(pipe_server_utils::RealScanServer, (LPVOID)pFilesMap);
		//Starting manual scan pipes server
		pManualScanThread = AfxBeginThread(pipe_server_utils::ManualScanServer, (LPVOID)pFilesMap);
	}

	//Report to the event log that the service has started successfully
	m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, CNTS_MSG_SERVICE_STARTED, m_sDisplayName);

	//The tight loop which constitutes the service
	BOOL bOldPause = m_bPaused;
	while (!m_bWantStop)
	{
		Sleep(m_dwBeepInternal);
		//SCM has requested a Pause / Continue
		if (m_bPaused != bOldPause)
		{
			if (m_bPaused)
			{
				ReportStatusToSCM(SERVICE_PAUSED, NO_ERROR, 0, 1, 0);
				//Report to the event log that the service has paused successfully
				m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, CNTS_MSG_SERVICE_PAUSED, m_sDisplayName);
			}
			else
			{
				ReportStatusToSCM(SERVICE_RUNNING, NO_ERROR, 0, 1, 0);
				//Report to the event log that the service has stopped continued
				m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, CNTS_MSG_SERVICE_CONTINUED, m_sDisplayName);
			}
		}

		bOldPause = m_bPaused;
	}

	//Pretend that closing down takes some time
	ReportStatusToSCM(SERVICE_STOP_PENDING, NO_ERROR, 0, 1, 0);
	
	//Stoping real scan pipe server
	if(NULL != pRealScanThread)
	{
		StopScanServer(sgScanServer);
		WaitForSingleObject(pRealScanThread->m_hThread, 20000);
	}

	if(NULL != pManualScanThread)
	{
		//Stoping manual scan pipe server
		StopScanServer(sgManualScanServer);
		WaitForSingleObject(pManualScanThread->m_hThread, 20000);
	}

	file_hash_DB_utils::DeleteHashDBCriticalSection(pFilesMap);

	//Unlock hash DB file
	UnlockFile(hDataFile, 0, 0, low, high);
	CloseHandle(hDataFile);

	scan_log_utils::LogHeader("Saving hash DB", GetCurrentProcessId());
	timer.Start();
	file_hash_DB_utils::WritePassData(pFilesMap);
	dSec = timer.Stop();
	scan_log_utils::LogTime("Save time", dSec);

	pFilesMap->clear();
	delete pFilesMap;

	ReportStatusToSCM(SERVICE_STOPPED, NO_ERROR, 0, 1, 0);

	//Report to the event log that the service has stopped successfully
	m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, CNTS_MSG_SERVICE_STOPPED, m_sDisplayName);
}

void CMyService::OnStop()
{
  CSingleLock l(&m_CritSect, TRUE); //synchronise access to the variables

  //Change the current state to STOP_PENDING
  m_dwCurrentState = SERVICE_STOP_PENDING;  

	//Signal the other thread to end
	m_bWantStop = TRUE;
}

void CMyService::OnPause()
{
  CSingleLock l(&m_CritSect, TRUE); //synchronise access to the variables

  //Change the current state
  m_dwCurrentState = SERVICE_PAUSE_PENDING;  

	//Signal the other thread
  m_bPaused = TRUE;
}

void CMyService::OnContinue()
{
  CSingleLock l(&m_CritSect, TRUE); //synchronise access to the variables

  //Change the current state
  m_dwCurrentState = SERVICE_CONTINUE_PENDING;  

	//Signal the other thread
  m_bPaused = FALSE;
}

void CMyService::OnUserDefinedRequest(DWORD dwControl)
{
  //Any value greater than 200 increments the doubles the beep frequency
  //otherwise the frequency is halved
  if (dwControl > 200)
    m_dwBeepInternal /= 2;
  else
    m_dwBeepInternal *= 2;

  //Report to the event log that the beep interval has been changed
  CString sInterval;
  sInterval.Format(_T("%d"), m_dwBeepInternal);
  m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, MSG_SERVICE_SET_FREQUENCY, sInterval);
}

void CMyService::ShowHelp()
{
	AfxMessageBox(_T("Service Usage: DCService.exe [-install | -uninstall | -help]\n"));
}