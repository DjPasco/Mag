#pragma once

#include <vector>
#include "ScanEndingObs.h"

typedef std::vector<CString> CScanItems;

class CDCAntiVirusScheduledScanDlg : public CDialog,
								     public CScanEndingObs
{
public:
	CDCAntiVirusScheduledScanDlg(CWnd* pParent = NULL);
	virtual ~CDCAntiVirusScheduledScanDlg();

protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	void EnableProgresItems(BOOL bEnable);
	void EnableStartItems(BOOL bEnable);

	void FillScanItems();
	void FillItemTypes();

public:
	void EnumerateFiles();
	virtual bool Continue();
	CScanItems GetScanItems();
	CString GetExts();
	void ShowCurrentItem(LPCSTR sItem);
	void OnVirus(LPCSTR sItem, LPCSTR sVirus);
	void OnOK(LPCSTR sItem, LPCSTR sOK);
	bool GetUseInternalDB();

	void OnFinish();

private:
	CListCtrl m_listScanItems;
	CListCtrl m_listInfected;

	CProgressCtrl m_progres;

	bool m_bScanning;

public:
	void OnStop();
};
