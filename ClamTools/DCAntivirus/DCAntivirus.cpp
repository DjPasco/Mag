#include "stdafx.h"
#include "DCAntiVirus.h"
#include "DCAntiVirusDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDCAntiVirusApp theApp;

BOOL CDCAntiVirusApp::InitInstance()
{
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CDCAntiVirusDlg *pDlg = new CDCAntiVirusDlg;
	m_pMainWnd = pDlg;
	pDlg->DoModal();
	delete pDlg;
	return FALSE;
}
