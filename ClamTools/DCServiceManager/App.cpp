#include "stdafx.h"
#include "resource.h"
#include "ntserv_msg.h"
#include "app.h"
#include "DCAntivirusScanDlg.h"
#include "../Utils/Scanner/Scanner.h"
#include "../Utils/Registry.h"

//#define _TEST_

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT ScanDlg(LPVOID pParam)
{
	if(NULL != pParam)
	{
		CScanner *pScanner = (CScanner *)pParam;
		CDCAntivirusScanDlg scanDlg(pScanner);
		scanDlg.Create(IDD_SCAN_DLG);
		scanDlg.ShowWindow(SW_HIDE);
		scanDlg.RunModalLoop();
	}
	return 0;
}

CApp theApp;
BOOL CApp::InitInstance()
{
	registry_utils::CheckBaseDir();

#ifdef _TEST_
	CScanner *pScanner = new CScanner;
	pScanner->LoadDatabases();
	AfxBeginThread(ScanDlg, (LPVOID)pScanner);

	while (true)
	{
		Sleep(1000);
	}

	delete pScanner;
#else
	CNTServiceCommandLineInfo cmdInfo;
	CMyService Service;
	Service.ParseCommandLine(cmdInfo);
	Service.ProcessShellCommand(cmdInfo);
#endif

	return FALSE;
}


CMyService::CMyService() : CNTService(sgServiceName, sgServiceDisplayName, SERVICE_ACCEPT_STOP, sgServiceDescription) 
{
	m_bWantStop = FALSE;
	m_bPaused = FALSE;
	m_dwBeepInternal = 1000;
}

void CMyService::ServiceMain(DWORD /*dwArgc*/, LPTSTR* /*lpszArgv*/)
{
	//register our control handler
	RegisterCtrlHandler();

	//Pretend that starting up takes some time
	ReportStatusToSCM(SERVICE_START_PENDING, NO_ERROR, 0, 1, 0);
		CScanner *pScanner = new CScanner;
		pScanner->LoadDatabases();
		AfxBeginThread(ScanDlg, (LPVOID)pScanner);
	ReportStatusToSCM(SERVICE_RUNNING, NO_ERROR, 0, 1, 0);

	//Report to the event log that the service has started successfully
	m_EventLogSource.Report(EVENTLOG_INFORMATION_TYPE, CNTS_MSG_SERVICE_STARTED, m_sDisplayName);

	//The tight loop which constitutes the service
	BOOL bOldPause = m_bPaused;
	while (!m_bWantStop)
	{
		//As a demo, we just do a message beep
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
	delete pScanner;
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
	AfxMessageBox(_T("A demo service which just beeps the speaker\nUsage: testsrv [-install | -uninstall | -help]\n"));
}