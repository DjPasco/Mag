#pragma once

class CScheduledTask;

//Dialog to set sched scan options
class CDCAntiVirusShedDlg : public CDialog
{
public:
	CDCAntiVirusShedDlg(LPCSTR sTitle, CScheduledTask *pTask, CWnd* pParent = NULL);
	virtual ~CDCAntiVirusShedDlg();

protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

protected:
	void OnBnClickedOk();
	bool ControlChecked(UINT ID);

private:
	CScheduledTask *m_pTask;
	CString m_sTitle;
};
