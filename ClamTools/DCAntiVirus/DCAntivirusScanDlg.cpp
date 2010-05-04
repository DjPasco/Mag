#include "stdafx.h"
#include "Resource.h"
#include "DCAntivirusScanDlg.h"

#include "../Utils/SendObj.h"
#include "Scanner/Scanner.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CDCAntivirusScanDlg, CDialog)
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
END_MESSAGE_MAP()

CDCAntivirusScanDlg::CDCAntivirusScanDlg(CScanner *pScanner)
	: m_pScanner(pScanner)
{
	//
}

LRESULT CDCAntivirusScanDlg::OnCopyData(WPARAM wParam, LPARAM lParam)
{
	PCOPYDATASTRUCT copy = (PCOPYDATASTRUCT) lParam;
	CString sFile = ((CSendObj *)(copy->lpData))->m_sPath;
	CString sVirusName;
	m_pScanner->ScanFile(sFile, sVirusName);
	return 0;
}
