#include "stdafx.h"
#include "DetoursHookCenter.h"
#include "DetoursHookCenterDlg.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CDetoursHookCenterApp theApp;

BOOL CDetoursHookCenterApp::InitInstance()
{
	CWinApp::InitInstance();

	AfxInitRichEdit2();

	CDetoursHookCenterDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();
	
	return FALSE;
}
