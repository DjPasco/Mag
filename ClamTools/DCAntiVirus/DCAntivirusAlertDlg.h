#pragma once

class CDCAntivirusAlertDlg : public CDialog
{
public:
	CDCAntivirusAlertDlg(LPCSTR sFilePath, LPCSTR sVirusName, DWORD PID);
	virtual ~CDCAntivirusAlertDlg();

protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CString m_sFilePath;
	CString m_sVirusName;
	DWORD m_PID;

public:
	void OnDel();
	void OnQuarantine();
};
