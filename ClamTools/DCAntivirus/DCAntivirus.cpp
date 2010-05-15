#include "stdafx.h"
#include "DCAntiVirus.h"
#include "DCAntiVirusDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CDCAntiVirusApp theApp("DCAntivirus");

CDCAntiVirusApp::CDCAntiVirusApp(LPCTSTR lpszAppName)
: CWinApp(lpszAppName)
{
	//
}

BOOL CDCAntiVirusApp::InitInstance()
{
	CWinApp::InitInstance();
	SetRegistryKey(_T("DCAntivirus"));

	CoInitialize(NULL);//For task scheduler

	CDCAntiVirusDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}
