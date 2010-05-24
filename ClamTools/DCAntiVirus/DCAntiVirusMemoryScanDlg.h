#pragma once

class CDCAntiVirusMemoryScanDlg : public CDialog
{
public:
	CDCAntiVirusMemoryScanDlg(CWnd* pParent = NULL);
	virtual ~CDCAntiVirusMemoryScanDlg();

protected:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()

public:
	void EnumerateFiles();
	void ShowCurrentItem(LPCSTR sItem);
	void OnVirus(LPCSTR sItem, LPCSTR sVirus);
	void OnOK(LPCSTR sItem, LPCSTR sOK);

	void OnFinish();

	bool Continue();

private:
	CListCtrl m_listLog;
	CProgressCtrl m_progres;
	bool m_bScanning;
public:
	afx_msg void OnStopMemory();
};
