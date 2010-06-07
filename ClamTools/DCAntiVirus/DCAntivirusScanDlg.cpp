#include "stdafx.h"
#include "Resource.h"
#include "DCAntivirusScanDlg.h"

#include "DCAntivirusAlertDlg.h"

#include "../Utils/SendObj.h"
#include "../Utils/TraySendObj.h"
#include "../Utils/Settings.h"
#include "../Utils/Log.h"
#include "../Utils/Scanner/Scanner.h"
#include "../Utils/npipe.h"
#include "../Utils/PipeClientUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static LPCTSTR gszProcessorTime="\\Processor(_Total)\\% Processor Time";

#define MAX_LOAD 10
#define CHECK_IDLE 60000 // One minute

class CTraySendHelper
{
public:
	CTraySendObj	m_obj;
	HWND			m_hwnd;
};

UINT SendToTray(LPVOID pParam)
{
	CTraySendHelper *helper = (CTraySendHelper *)pParam;

	COPYDATASTRUCT copy;
	copy.dwData = 1;
	copy.cbData = sizeof(helper->m_obj);
	copy.lpData = &helper->m_obj;

	::SendMessage(helper->m_hwnd, WM_COPYDATA, 0, (LPARAM) (LPVOID) &copy);

	delete helper;
	return 0;
}

BEGIN_MESSAGE_MAP(CDCAntivirusScanDlg, CDialog)
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
	ON_WM_TIMER()
END_MESSAGE_MAP()

CDCAntivirusScanDlg::CDCAntivirusScanDlg()
	: m_bScan(true),
	  m_bDeny(true),
	  m_bIdleScan(true),
	  m_nMaxCPULoad(20),
	  m_nIdleTime(900000)// 15 minutes
{
	ReloadSettings(NULL);
}

CDCAntivirusScanDlg::~CDCAntivirusScanDlg()
{
	//
}

BOOL CDCAntivirusScanDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_hQuery = NULL;
	m_hCounter = NULL;
	m_bCounterInit = false;

	if(PdhOpenQuery(NULL, 1, &m_hQuery) == ERROR_SUCCESS)
	{
		if(PdhAddCounter(m_hQuery, gszProcessorTime, NULL, &m_hCounter) == ERROR_SUCCESS)
		{
			m_bCounterInit = TRUE;
		}
	}

	SetTimer(m_nTimer, CHECK_IDLE, NULL);
	
	return TRUE;
}

bool CDCAntivirusScanDlg::SendObj(CTraySendObj &obj)
{
	HWND hwnd = NULL;
	hwnd = ::FindWindow(NULL, sgAppName);

	if(NULL == hwnd)
	{
		return false;
	}

	CTraySendHelper *helper = new CTraySendHelper;
	helper->m_obj = obj;
	helper->m_hwnd = hwnd;

	AfxBeginThread(SendToTray, (LPVOID)helper, THREAD_PRIORITY_HIGHEST);
	return true;
}

LRESULT CDCAntivirusScanDlg::OnCopyData(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	
	PCOPYDATASTRUCT copy = (PCOPYDATASTRUCT) lParam;
	CSendObj *pData = NULL;
	pData = (CSendObj *)copy->lpData; 

	if(NULL == pData)
	{
		return 1;
	}

	switch(pData->m_nType)
	{
	case EScan:
		{
			if(m_bScan)
			{
				CFileResult result;
				ZeroMemory(&result, sizeof(CFileResult));
				pipe_client_utils::SendFileToPipeServer(sgScanServer, pData, result);
				if(result.m_bScanned)
				{
					SendFileToTray(pData->m_sPath, result.m_sVirusName, result.m_nFilesCount);
				}

				if(!result.m_bOK)
				{
					if(m_bDeny)
					{
						return 2;
					}
					else
					{
						CDCAntivirusAlertDlg dlg(pData->m_sPath, result.m_sVirusName, pData->m_PID);
						int nRet = dlg.DoModal();
						if(IDOK == nRet)
						{
							return 2;
						}

						return 1;
					}
				}
			}
		}
		break;
	case ERequest:
		{
			RequestData(pData);
		}
		break;
	case EReloadSettings:
		{
			ReloadSettings(pData);	
		}
		break;
	case EManualScan:
		{
			CFileResult result;
			ZeroMemory(&result, sizeof(CFileResult));
			pipe_client_utils::SendFileToPipeServer(sgScanServer, pData, result);
			if(result.m_bScanned)
			{
				SendFileToTray(pData->m_sPath, result.m_sVirusName, result.m_nFilesCount);
			}
			if(!result.m_bOK)
			{
				registry_utils::WriteProfileString(sgSection, sgVirusName, result.m_sVirusName);
				return 2;
			}
		}
		break;
	case EReloadDB:
		{
			CFileResult result;
			ZeroMemory(&result, sizeof(CFileResult));
			pipe_client_utils::SendFileToPipeServer(sgScanServer, pData, result);
		}
		break;
	}
	
	return 1;
}

void CDCAntivirusScanDlg::RequestData(CSendObj *pObj)
{
	CNamedPipe clientPipe;
	if (!CNamedPipe::ServerAvailable(".", _T(sgScanServer), 1000))
	{
		SendMessageToTray("Service not available.");
		return;
	}

	SECURITY_ATTRIBUTES sa;
	sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
	InitializeSecurityDescriptor(sa.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
	// ACL is set as NULL in order to allow all access to the object.
	SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, NULL, FALSE);
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;

	if (!clientPipe.Open(".", _T(sgScanServer), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, &sa, 0))
	{
		SendMessageToTray("Can't open connection to service.");
		return;
	}

	DWORD dwBytes;

	CSendObj newObj(*pObj);						
	clientPipe.Write(&newObj, sizeof(CSendObj), dwBytes);

	CTrayRequestData data;
	if (!clientPipe.Read(&data, sizeof(CTrayRequestData), dwBytes, NULL))
	{
		SendMessageToTray("Can't read data from service.");
		return;
	}

	SendMessageToTray("Service running.");
	SendInfoToTray(data);

	clientPipe.Close();
}

void CDCAntivirusScanDlg::SendMessageToTray(LPCSTR sMessage)
{
	CTraySendObj obj;
	obj.m_nType = EMessage;
	strcpy_s(obj.m_sText, MAX_PATH, sMessage);

	SendObj(obj);
}

void CDCAntivirusScanDlg::SendInfoToTray(CTrayRequestData &pTrayInfo)
{
	CTraySendObj obj;
	obj.m_nType = EData;
	//Main DB info
	obj.m_bMain = true;
	strcpy_s(obj.m_sText, MAX_PATH, pTrayInfo.m_sMainDate);
	obj.m_nVersion		= pTrayInfo.m_nMainVersion;
	obj.m_nSigs			= pTrayInfo.m_nMainSigCount;
	obj.m_nFilesCount	= pTrayInfo.m_nFilesCount;

	SendObj(obj);

	//Daily DB info
	obj.m_bMain = false;
	strcpy_s(obj.m_sText, MAX_PATH, pTrayInfo.m_sDailyDate);
	obj.m_nVersion		= pTrayInfo.m_nDailyVersion;
	obj.m_nSigs			= pTrayInfo.m_nDailySigCount;
	obj.m_nFilesCount	= pTrayInfo.m_nFilesCount;

	SendObj(obj);
}

void CDCAntivirusScanDlg::SendFileToTray(LPCSTR sFile, LPCSTR sVirus, int nFilesCount)
{
	CTraySendObj obj;
	obj.m_nType = EFile;
	strcpy_s(obj.m_sText, MAX_PATH, sFile);
	strcpy_s(obj.m_sText2, MAX_PATH, sVirus);
	obj.m_nFilesCount = nFilesCount;

	SendObj(obj);
}

void CDCAntivirusScanDlg::OnTimer(UINT nIDEvent)
{
	if(!m_bIdleScan)
	{
		return;
	}

	if(m_bCounterInit && TimeForScan())
	{
		long lLoad = GetCPUCycle(m_hQuery, m_hCounter);
		if(-1 != lLoad && MAX_LOAD >= lLoad)
		{
			CSendObj obj;
			obj.m_nType = EIdleScan;
			CFileResult result;
			ZeroMemory(&result, sizeof(CFileResult));
			pipe_client_utils::SendFileToPipeServer(sgScanServer, &obj, result);
		}
	}

	CDialog::OnTimer(nIDEvent);
}

LONG CDCAntivirusScanDlg::GetCPUCycle(HQUERY query, HCOUNTER counter)
{
	// Collect the current raw data value for all counters in the 
	// specified query and updates the status code of each counter 
	if(PdhCollectQueryData(query) != ERROR_SUCCESS)
	{
		return -1;
	}

	PDH_RAW_COUNTER	ppdhRawCounter;

	// Get the CPU raw counter value	
	if(PdhGetRawCounterValue(counter, NULL, &ppdhRawCounter) == ERROR_SUCCESS)
	{
		PDH_FMT_COUNTERVALUE pdhFormattedValue;
		// Format the CPU counter
		if(PdhGetFormattedCounterValue(counter, PDH_FMT_LONG, NULL, &pdhFormattedValue) == ERROR_SUCCESS)
		{
			return pdhFormattedValue.longValue;
		}
	}

	return -1;
}

bool CDCAntivirusScanDlg::TimeForScan()
{
	LASTINPUTINFO li;
	li.cbSize = sizeof(LASTINPUTINFO);
	GetLastInputInfo(&li);
	int i = li.dwTime;
	UINT timeDuration = (GetTickCount() - i);
	
	if(timeDuration > (UINT)m_nIdleTime)
	{
		return true;
	}

	return false;
}

void CDCAntivirusScanDlg::ReloadSettings(CSendObj *pObj)
{
	CSettingsInfo info;
	if(settings_utils::Load(info))
	{
		m_bScan = info.m_bScan;
		m_bDeny = info.m_bDeny;

		m_bIdleScan		= info.m_bIdle;
		m_nMaxCPULoad	= info.m_nCPULoad;
		m_nIdleTime		= info.m_nIdleTime * 60000;

		if(NULL != pObj)
		{
			CFileResult result;
			ZeroMemory(&result, sizeof(CFileResult));
			pipe_client_utils::SendFileToPipeServer(sgScanServer, pObj, result);
		}
	}
}
