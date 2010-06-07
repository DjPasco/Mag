#pragma once

#include "ScanEndingObs.h"
#include "DCAntiVirusLogDlg.h"
#include "ManualScanObs.h"

class CScanItems;
class CDCAntiVirusScheduledScanDlg : public CDialog,
								     public CScanEndingObs,
									 public CManualScanObs
{
public:
	CDCAntiVirusScheduledScanDlg(CWnd* pParent = NULL);
	virtual ~CDCAntiVirusScheduledScanDlg();

protected:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()

	LRESULT OnStartScan(WPARAM wParam, LPARAM lParam);

private:
	void EnableProgresItems(BOOL bEnable);
	void EnableStartItems(BOOL bEnable);

	void FillScanItems();
	void FillItemTypes();

	//ManualScanObs overides
public:
	virtual bool Continue();
	virtual void ShowCurrentItem(LPCSTR sItem);
	virtual void OnVirus(LPCSTR sItem, LPCSTR sVirus);
	virtual void OnFinish(LPCSTR sFinishText);
	virtual void OnMessage(LPCSTR sMessage);

public:
	void EnumerateFiles(CScanItems &files);
	CScanItems GetScanItems();
	CString GetExts();
	bool GetUseInternalDB();

private:
	CListCtrl m_listScanItems;
	CProgressCtrl m_progres;

	bool m_bScanning;
	int m_nCount;

	CInfItems m_infItems;
	CTime m_tStart;
	CTime m_tEnd;

public:
	void OnStop();
};
