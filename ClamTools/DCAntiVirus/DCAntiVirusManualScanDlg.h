#pragma once

#include <vector>
#include "ScanEndingObs.h"
#include "DCAntiVirusLogDlg.h"

typedef std::vector<CString> CScanItems;

class CDCAntiVirusManualScanDlg : public CDialog,
								  public CScanEndingObs
{
public:
	CDCAntiVirusManualScanDlg(CWnd* pParent = NULL);
	virtual ~CDCAntiVirusManualScanDlg();

protected:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

private:
	void EnableProgresItems(BOOL bEnable);
	void EnableStartItems(BOOL bEnable);

public:
	void EnumerateFiles();
	virtual bool Continue();
	CScanItems GetScanItems();
	CString GetExts();
	void ShowCurrentItem(LPCSTR sItem);
	void OnVirus(LPCSTR sItem, LPCSTR sVirus);
	bool GetUseInternalDB();

	void OnFinish();

private:
	CListCtrl m_listScanItems;
	CProgressCtrl m_progres;

	bool m_bScanning;
	int m_nCount;

	CInfItems m_infItems;
	CTime m_tStart;
	CTime m_tEnd;

public:
	void OnAdd();
	void OnRemove();
	void OnScan();
};
