#include "stdafx.h"
#include "DetoursHookCenter.h"
#include "DetoursHookCenterDlg.h"
#include "HookUtils.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CDetoursHookCenterApp theApp;

BOOL CDetoursHookCenterApp::InitInstance()
{
	CWinApp::InitInstance();

	CCommandLineInfo rCmdInfo;
	CWinApp::ParseCommandLine(rCmdInfo);
	
	CLog log;
	hook_utils::LoadNotepadWithHookDll(log);
	//AfxInitRichEdit2();

	//CDetoursHookCenterDlg dlg;
	//m_pMainWnd = &dlg;
	//dlg.DoModal();
	
	return FALSE;
}
