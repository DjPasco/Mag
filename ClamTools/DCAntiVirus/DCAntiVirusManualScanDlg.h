#pragma once

#include <vector>
#include "ScanEndingObs.h"

typedef std::vector<CString> CScanItems;

class CDCAntiVirusManualScanDlg : public CDialog,
								  public CScanEndingObs
{
public:
	CDCAntiVirusManualScanDlg(LPCSTR sTitle, CWnd* pParent = NULL);
	virtual ~CDCAntiVirusManualScanDlg();

protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	void EnableProgresItems(BOOL bEnable);
	void EnableStartItems(BOOL bEnable);

public:
	void EnumerateFiles();
	virtual bool Continue();
	CScanItems GetScanItems();
	void ShowCurrentItem(LPCSTR sItem);
	void OnVirus(LPCSTR sItem, LPCSTR sVirus);

	void OnFinish();

private:
	CString m_sTitle;

	CListCtrl m_listScanItems;
	CListCtrl m_listInfected;

	CProgressCtrl m_progres;

	bool m_bScanning;

public:
	void OnAdd();
	void OnRemove();
	void OnScan();
};
