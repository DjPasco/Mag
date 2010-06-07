#ifndef _DC_ANTI_VIRUS_SCAN_DLG_H__
#define _DC_ANTI_VIRUS_SCAN_DLG_H__
#pragma once

#ifndef _PDH_H_
	#include <pdh.h>
#endif

class CSendObj;
class CTrayRequestData;
class CTraySendObj;
class CFileResult;

class CDCAntivirusScanDlg : public CDialog
{
public:
	CDCAntivirusScanDlg();
	virtual ~CDCAntivirusScanDlg();

protected:
	LRESULT OnCopyData(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	void OnTimer(UINT nIDEvent);
	DECLARE_MESSAGE_MAP()

protected:
	LONG GetCPUCycle(HQUERY query, HCOUNTER counter);
	bool TimeForScan();

private:
	void ReloadSettings(CSendObj *pObj);

	void RequestData(CSendObj *pObj);

	void SendInfoToTray(CTrayRequestData &pTrayInfo);
	void SendMessageToTray(LPCSTR sMessage);
	bool SendObj(CTraySendObj &obj);
	void SendFileToTray(LPCSTR sFile, LPCSTR sVirus, int nFilesCount);

private:
	//CScanner *m_pScanner;
	UINT_PTR m_nTimer;

	HQUERY		m_hQuery;
	HCOUNTER	m_hCounter;
	bool		m_bCounterInit;

	BOOL m_bIdleScan;
	int m_nMaxCPULoad;
	int m_nIdleTime;//must be miliseconds
	
	BOOL m_bScan;
	BOOL m_bDeny;
};

#endif
