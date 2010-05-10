#include "stdafx.h"
#include "Resource.h"
#include "DCAntivirusScanDlg.h"

#include "../IdleTracker/IdleTracker.h"
#include "../Utils/SendObj.h"
#include "../Utils/Scanner/Scanner.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static LPCTSTR gszProcessorTime="\\Processor(_Total)\\% Processor Time";

#define MAX_LOAD 10
#define IDL_TIME 5000
#define CHECH_IDLE 3000


BEGIN_MESSAGE_MAP(CDCAntivirusScanDlg, CDialog)
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
	ON_WM_TIMER()
END_MESSAGE_MAP()

CDCAntivirusScanDlg::CDCAntivirusScanDlg(CScanner *pScanner)
	: m_pScanner(pScanner)
{
	//
}

CDCAntivirusScanDlg::~CDCAntivirusScanDlg()
{
	IdleTrackerTerm();
}

BOOL CDCAntivirusScanDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	IdleTrackerInit();
	
	SetTimer(m_nTimer, CHECH_IDLE, NULL);

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
	CString sFile = ((CSendObj *)(copy->lpData))->m_sPath;
	CString sVirusName;
	m_pScanner->ScanFile(sFile, sVirusName);
	return 0;
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

long CDCAntivirusScanDlg::GetCPUUsage()
{
	if(m_bCounterInit)
	{
		return GetCPUCycle(m_hQuery, m_hCounter);
	}

	return 0;
}

bool CDCAntivirusScanDlg::TimeForScan()
{
	UINT timeDuration = (UINT)(GetTickCount() - IdleTrackerGetLastTickCount());
	if(timeDuration > IDL_TIME)
	{
		return true;
	}

	return false;
}
