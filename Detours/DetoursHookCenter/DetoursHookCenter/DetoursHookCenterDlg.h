#pragma once

#include "Log.h"

class CDetoursHookCenterDlg : public CDialog
{
public:
	CDetoursHookCenterDlg(CWnd* pParent = NULL)
		:CDialog(IDD_DETOURSHOOKCENTER_DIALOG, pParent) { };
	virtual ~CDetoursHookCenterDlg() { };

protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

protected:
	void OnHookNotepad();
	LRESULT OnCopyData(WPARAM wParam, LPARAM lParam);

private:
	CLog m_Log;
};
