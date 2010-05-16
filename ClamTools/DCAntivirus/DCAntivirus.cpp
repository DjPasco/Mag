#include "stdafx.h"
#include "DCAntiVirus.h"
#include "DCAntiVirusDlg.h"

#include "../Utils/Registry.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CDCAntiVirusApp theApp;

BOOL CDCAntiVirusApp::InitInstance()
{
	registry_utils::CheckBaseDir();

	CWinApp::InitInstance();

	CoInitialize(NULL);//For task scheduler

	CDCAntiVirusDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}
