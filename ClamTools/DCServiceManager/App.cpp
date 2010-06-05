#include "stdafx.h"
#include "resource.h"
#include "ntserv_msg.h"
#include "app.h"

#include "../Utils/Scanner/FileHashDBUtils.h"
#include "../Utils/Scanner/Scanner.h"
#include "../Utils/Registry.h"
#include "../Utils/Settings.h"
#include "../Utils/Log.h"
#include "../Utils/npipe.h"
#include "../Utils/TraySendObj.h"
#include "../Utils/SendObj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void DoAction(CSendObj *pData, CScanner *pScanner, CFileResult &result)
{
	result.m_bOK = true;
	strcpy_s(result.m_sVirusName, MAX_PATH, "File is clean.");

	switch(pData->m_nType)
	{
	case EScan:
		{
			CString sFile = pData->m_sPath;
			CString sVirusName;
			if(!pScanner->ScanFile(sFile, sVirusName, pData->m_PID, result.m_bScanned, true))
			{
				result.m_bOK = false;
				strcpy_s(result.m_sVirusName, MAX_PATH, sVirusName);
			}
		}
		break;
	case EReloadSettings:
		{
			CSettingsInfo info;
			if(settings_utils::Load(info))
			{
				pScanner->SetScanSettings(info.m_bDeep, info.m_bOffice, info.m_bArchives, info.m_bPDF, info.m_bHTML);
				pScanner->SetFilesTypes(info.m_sFilesTypes);
			}
		}
		break;
	case EManualScan:
		{
			int nOldPriority = GetThreadPriority(GetCurrentThread());
			SetThreadPriority(GetCurrentThread(), priority_utils::GetRealPriority(path_utils::GetPriority()));

			registry_utils::WriteProfileString(sgSection, sgVirusName, "");
			CString sFile = pData->m_sPath;
			CString sVirusName;
			bool bClean(true);
			if(pData->m_bUseInternalDB)
			{
				bClean = pScanner->ScanFile(sFile, sVirusName, pData->m_PID, result.m_bScanned, false); 
			}
			else
			{
				bClean = pScanner->ScanFileNoIntDB(sFile, sVirusName, pData->m_PID, result.m_bScanned); 
			}

			SetThreadPriority(GetCurrentThread(), nOldPriority);

			if(!bClean)
			{
				result.m_bOK = false;
				strcpy_s(result.m_sVirusName, MAX_PATH, sVirusName);
			}
		}
		break;
	case EReloadDB:
		{
			pScanner->ReloadDB();
		}
		break;
	}

	result.m_nFilesCount = pScanner->GetFilesCount();
}

UINT Server(LPVOID pParam)
{
	CScannedFileMap *pFilesMap = (CScannedFileMap *)pParam;

	if(NULL == pFilesMap)
	{
		return 0;
	}

	CScanner *pScanner = new CScanner;
	pScanner->LoadDatabases();
	pScanner->SetFilesMap(pFilesMap);

	CNamedPipe serverPipe;
	SECURITY_ATTRIBUTES sa;
	sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
	InitializeSecurityDescriptor(sa.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
	// ACL is set as NULL in order to allow all access to the object.
	SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, NULL, FALSE);
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	if (!serverPipe.Create(_T(sgScanServer), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_WAIT, 1, 4096, 4096, 1, &sa))
	{
		delete pScanner;
		return 0;
	}

	while (1)
	{
		if (!serverPipe.ConnectClient())
		{
			continue;
		}
		
		CSendObj obj;
		DWORD dwBytes;
		if (serverPipe.Read(&obj, sizeof(CSendObj), dwBytes, NULL))
		{
			if(EQuit == obj.m_nType)
			{
				serverPipe.DisconnectClient();
				break;
			}

			if(obj.m_nType == ERequest)
			{
				CTrayRequestData data;
				ZeroMemory(&data, sizeof(CTrayRequestData));
				pScanner->RequestData(data);
				serverPipe.Write(&data, sizeof(CTrayRequestData), dwBytes);
			}
			else
			{
				CFileResult result;
				ZeroMemory(&result, sizeof(CFileResult));

				DoAction(&obj, pScanner, result);

				serverPipe.Write(&result, sizeof(CFileResult), dwBytes);
			}
		}

		if (!serverPipe.DisconnectClient())
		{
			continue;
		}
	}

	delete pScanner;

	return 0;
};

//#define _TEST_

CApp theApp;
BOOL CApp::InitInstance()
{

#ifdef _TEST_ 
	
	CScanner *pScanner = new CScanner;
	pScanner->LoadDatabases();
	Server((LPVOID)pScanner);
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

void CMyService::StopRealScanServer()
{
	CNamedPipe clientPipe;
	if (!CNamedPipe::ServerAvailable(".", _T(sgScanServer), 1000))
	{
		return;
	}

	if (!clientPipe.Open(".", _T(sgScanServer), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, 0))
	{
		return;
	}

	DWORD dwBytes;

	CSendObj obj;
	obj.m_nType = EQuit;
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

	//Starting real scan pipes server
	CWinThread *pThread = AfxBeginThread(Server, (LPVOID)pFilesMap);

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
	StopRealScanServer();
	WaitForSingleObject(pThread->m_hThread, INFINITE);

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