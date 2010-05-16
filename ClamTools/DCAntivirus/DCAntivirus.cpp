#include "stdafx.h"
#include "DCAntiVirus.h"
#include "DCAntiVirusDlg.h"

#include "../Utils/Registry.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CDCAntiVirusApp theApp(sgAppName);

CDCAntiVirusApp::CDCAntiVirusApp(LPCTSTR lpszAppName)
: CWinApp(lpszAppName)
{
	//
}

BOOL CDCAntiVirusApp::InitInstance()
{
	CWinApp::InitInstance();
	SetRegistryKey(sgAppName);

	CoInitialize(NULL);//For task scheduler

	CDCAntiVirusDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}
