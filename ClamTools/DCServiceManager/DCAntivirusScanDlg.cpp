#include "stdafx.h"
#include "Resource.h"
#include "DCAntivirusScanDlg.h"

#include "DCAntivirusAlertDlg.h"

#include "../IdleTracker/IdleTracker.h"
#include "../Utils/SendObj.h"
#include "../Utils/Settings.h"
#include "../Utils/Scanner/Scanner.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static LPCTSTR gszProcessorTime="\\Processor(_Total)\\% Processor Time";

#define MAX_LOAD 10
#define CHECK_IDLE 60000 // One minute

BEGIN_MESSAGE_MAP(CDCAntivirusScanDlg, CDialog)
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
	ON_WM_TIMER()
END_MESSAGE_MAP()

CDCAntivirusScanDlg::CDCAntivirusScanDlg(CScanner *pScanner)
	: m_pScanner(pScanner),
	  m_bScan(true),
	  m_bDeny(true),
	  m_bIdleScan(true),
	  m_nMaxCPULoad(20),
	  m_nIdleTime(900000)// 15 minutes
{
	ReloadSettings();
}

CDCAntivirusScanDlg::~CDCAntivirusScanDlg()
{
	IdleTrackerTerm();
}

BOOL CDCAntivirusScanDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	IdleTrackerInit();
	
	SetTimer(m_nTimer, CHECK_IDLE, NULL);

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
	
	return TRUE;
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
				CString sFile = pData->m_sPath;
				CString sVirusName;
				if(!m_pScanner->ScanFile(sFile, sVirusName, true))
				{
					if(m_bDeny)
					{
						return 2;
					}
					else
					{
						CDCAntivirusAlertDlg dlg(sFile, sVirusName, pData->m_PID);
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
			m_pScanner->RequestData();
		}
		break;
	case EReloadSettings:
		{
			ReloadSettings();	
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
				bClean = m_pScanner->ScanFile(sFile, sVirusName, false); 
			}
			else
			{
				bClean = m_pScanner->ScanFileNoIntDB(sFile, sVirusName); 
			}

			SetThreadPriority(GetCurrentThread(), nOldPriority);

			if(!bClean)
			{
				registry_utils::WriteProfileString(sgSection, sgVirusName, sVirusName);
				return 2;
			}
		}
		break;
	case EReloadDB:
		{
			m_pScanner->ReloadDB();
		}
		break;
	}
	
	return 1;
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
			m_pScanner->ScanFilesForOptimisation(this);
		}
	}

	CDialog::OnTimer(nIDEvent);
}

bool CDCAntivirusScanDlg::ContinueScan()
{
	return TimeForScan();
}

bool CDCAntivirusScanDlg::IsCPULoaded()
{
	if(m_bCounterInit)
	{
		long lCPULoad = GetCPUCycle(m_hQuery, m_hCounter);
		return m_nMaxCPULoad < lCPULoad;
	}

	return false;
}

bool CDCAntivirusScanDlg::TimeForScan()
{
	UINT timeDuration = (UINT)(GetTickCount() - IdleTrackerGetLastTickCount());
	if(timeDuration > (UINT)m_nIdleTime)
	{
		return true;
	}

	return false;
}

void CDCAntivirusScanDlg::ReloadSettings()
{
	CSettingsInfo info;
	if(settings_utils::Load(info))
	{
		m_bScan = info.m_bScan;
		m_bDeny = info.m_bDeny;

		m_bIdleScan		= info.m_bIdle;
		m_nMaxCPULoad	= info.m_nCPULoad;
		m_nIdleTime		= info.m_nIdleTime * 60000;

		m_pScanner->SetScanSettings(info.m_bDeep, info.m_bOffice, info.m_bArchives, info.m_bPDF, info.m_bHTML);
		m_pScanner->SetFilesTypes(info.m_sFilesTypes);
	}
}
