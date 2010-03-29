#include "stdafx.h"
#include "DetoursHookCenter.h"
#include "DetoursHookCenterDlg.h"
#include "HookUtils.h"
#include "../../Utils/SendObj.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CDetoursHookCenterDlg, CDialog)
	ON_BN_CLICKED(IDC_HOOK_NOTEPAD, OnHookNotepad)
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
END_MESSAGE_MAP()

BOOL CDetoursHookCenterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

#ifdef _DEBUG 
	this->SetWindowText(_T("DetoursHookCenter (Debug)"));
#endif

	m_Log.SubclassDlgItem(IDC_RICH_LOG, this);
 	return TRUE;
}

void CDetoursHookCenterDlg::OnHookNotepad()
{
	hook_utils::LoadNotepadWithHookDll(m_Log);
}

LRESULT CDetoursHookCenterDlg::OnCopyData(WPARAM wParam, LPARAM lParam)
{
	PCOPYDATASTRUCT copy = (PCOPYDATASTRUCT) lParam;
	CString sText = ((CSendObj *)(copy->lpData))->m_sPath;
	m_Log.AddRichText(sText);

	return 0;
}

