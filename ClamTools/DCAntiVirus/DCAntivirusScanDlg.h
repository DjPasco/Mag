#ifndef _DC_ANTI_VIRUS_SCAN_DLG_H__
#define _DC_ANTI_VIRUS_SCAN_DLG_H__
#pragma once

#ifndef _PDH_H_
	#include <pdh.h>
#endif

#ifndef _SCAN_VALIDATOR_OBS_H__
	#include "Scanner/ScanValidatorObs.h"
#endif


class CScanner;

class CDCAntivirusScanDlg : public CDialog,
							public CScanValidatorObs
{
public:
	CDCAntivirusScanDlg(CScanner *pScanner);
	virtual ~CDCAntivirusScanDlg();

public:
	virtual bool ContinueScan();
	virtual long GetCPUUsage();

protected:
	LRESULT OnCopyData(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	void OnTimer(UINT nIDEvent);
	DECLARE_MESSAGE_MAP()

protected:
	LONG GetCPUCycle(HQUERY query, HCOUNTER counter);
	bool TimeForScan();

private:
	CScanner *m_pScanner;
	UINT_PTR m_nTimer;

	HQUERY		m_hQuery;
	HCOUNTER	m_hCounter;
	bool		m_bCounterInit;
};

#endif
