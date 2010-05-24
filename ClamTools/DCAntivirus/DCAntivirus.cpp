#include "stdafx.h"
#include "DCAntiVirus.h"
#include "DCAntiVirusDlg.h"

#include "DCAntiVirusScheduledScanDlg.h"

#include "../Utils/Registry.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CDCAntiVirusApp theApp;

BOOL CDCAntiVirusApp::InitInstance()
{
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

		CWinApp::InitInstance();

		CoInitialize(NULL);//For task scheduler

		CDCAntiVirusDlg dlg;
		m_pMainWnd = &dlg;
		dlg.DoModal();
	}

	return FALSE;
}
