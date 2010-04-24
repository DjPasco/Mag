#ifndef _DC_ANTI_VIRUS_SCAN_DLG_H__
#define _DC_ANTI_VIRUS_SCAN_DLG_H__
#pragma once

class CScanner;

class CDCAntivirusScanDlg : public CDialog
{
public:
	CDCAntivirusScanDlg(CScanner *pScanner);

protected:
	LRESULT OnCopyData(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CScanner *m_pScanner;
};

#endif
