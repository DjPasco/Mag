#pragma once

#include "DCAntiVirusLogDlg.h"
#include "ManualScanObs.h"

class CScanItems;
class CDCAntiVirusMemoryScanDlg : public CDialog,
								  public CManualScanObs
{
public:
	CDCAntiVirusMemoryScanDlg(CWnd* pParent = NULL);
	virtual ~CDCAntiVirusMemoryScanDlg();

protected:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()

	LRESULT OnStartScan(WPARAM wParam, LPARAM lParam);

public:
	void EnumerateFiles(CScanItems &files);

	//ManualScanObs overides
public:
	virtual bool Continue();
	virtual void ShowCurrentItem(LPCSTR sItem);
	virtual void OnVirus(LPCSTR sItem, LPCSTR sVirus);
	virtual void OnFinish(LPCSTR sFinishText);
	virtual void OnMessage(LPCSTR sMessage);

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
