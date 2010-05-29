#include "stdafx.h"
#include "resource.h"
#include "DCAntiVirus.h"
#include "DCAntiVirusDlg.h"

#include "DCAntiVirusScheduledScanDlg.h"
#include "DCAntivirusScanDlg.h"

#include "../Utils/Registry.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

class CPrecisionTimer
{
	LARGE_INTEGER lFreq, lStart;

public:
	CPrecisionTimer()
	{
		QueryPerformanceFrequency(&lFreq);
	}

	inline void Start()
	{
		QueryPerformanceCounter(&lStart);
	}

	inline double Stop()
	{
		// Return duration in seconds...
		LARGE_INTEGER lEnd;
		QueryPerformanceCounter(&lEnd);
		return (double(lEnd.QuadPart - lStart.QuadPart) / lFreq.QuadPart);
	}
};

CDCAntiVirusApp theApp;

UINT ScanDlg(LPVOID pParam)
{
	CDCAntivirusScanDlg scanDlg;
	scanDlg.Create(IDD_SCAN_DLG);
	scanDlg.ShowWindow(SW_HIDE);
	scanDlg.RunModalLoop();
	return 0;
};

BOOL CDCAntiVirusApp::InitInstance()
{
	double dSec = 0.0402;

			int nSec = (int)floor(dSec);
		int hour=nSec/3600;
		nSec=nSec%3600;
		int min=nSec/60;
		nSec=nSec%60;
		int sec=nSec;
		int milisec;
		
	
	LPCTSTR pszParam = __argv[1];
	
	if(__argc > 1 && strstr(pszParam, "scan"))
	{
		CDCAntiVirusScheduledScanDlg dlg;
		dlg.DoModal();
	}
	else
	{
		HWND trayHwnd = FindWindow(NULL, "DCAntiVirus");

		if(NULL != trayHwnd)
		{
			return FALSE;
		}

		registry_utils::CheckBaseDir();

		AfxBeginThread(ScanDlg, (LPVOID)NULL);

		CWinApp::InitInstance();

		CoInitialize(NULL);//For task scheduler

		CDCAntiVirusDlg dlg;
		m_pMainWnd = &dlg;
		dlg.DoModal();
	}

	return FALSE;
}
