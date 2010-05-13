#include "stdafx.h"
#include "DCAntiVirus.h"
#include "DCAntiVirusDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CDCAntiVirusApp theApp;
BOOL CDCAntiVirusApp::InitInstance()
{
	CWinApp::InitInstance();
	SetRegistryKey(_T("DCAntivirus"));

	CDCAntiVirusDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}
