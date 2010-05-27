#pragma once

#include "DCAntiVirusLogDlg.h"

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
	void OnFinish(LPCSTR sReason);

	bool Continue();

private:
	CProgressCtrl m_progres;
	bool m_bScanning;
	int m_nCount;

	CInfItems m_infItems;
	CTime m_tStart;
	CTime m_tEnd;

public:
	void OnStopMemory();
};
