#pragma once

#include "ScanEndingObs.h"
#include "DCAntiVirusLogDlg.h"
#include "ManualScanObs.h"

class CScanItems;

class CDCAntiVirusManualScanDlg : public CDialog,
								  public CScanEndingObs,
								  public CManualScanObs
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
	bool GetUseInternalDB();
	void EnumerateFiles(CScanItems &files);
	CString GetExts();

	//ManualScanObs overides
public:
	virtual bool Continue();
	virtual void ShowCurrentItem(LPCSTR sItem);
	virtual void OnVirus(LPCSTR sItem, LPCSTR sVirus);
	virtual void OnFinish(LPCSTR sFinishText);
	virtual void OnMessage(LPCSTR sMessage);

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
