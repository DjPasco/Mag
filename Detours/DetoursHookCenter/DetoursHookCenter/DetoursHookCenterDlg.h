#pragma once

#include "Log.h"

// CDetoursHookCenterDlg dialog
class CDetoursHookCenterDlg : public CDialog
{
// Construction
public:
	CDetoursHookCenterDlg(CWnd* pParent = NULL);
	virtual ~CDetoursHookCenterDlg();

public:
	void OnFiles();

protected:
	CString Readline(SOCKET *client);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	void OnSysCommand(UINT nID, LPARAM lParam);
	void OnPaint();
	HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	void OnHookNotepad();
	LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);

private:
	CLog m_Log;

	CWinThread *m_pFilesTask;
};
