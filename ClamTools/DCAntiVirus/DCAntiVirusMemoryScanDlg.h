#pragma once

class CDCAntiVirusMemoryScanDlg : public CDialog
{
public:
	CDCAntiVirusMemoryScanDlg(CWnd* pParent = NULL);
	virtual ~CDCAntiVirusMemoryScanDlg();

protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:
	void EnumerateFiles();
	void ShowCurrentItem(LPCSTR sItem);
	void OnVirus(LPCSTR sItem, LPCSTR sVirus);
	void OnOK(LPCSTR sItem, LPCSTR sOK);

	void OnFinish();

private:
	CListCtrl m_listLog;
	CProgressCtrl m_progres;
};
