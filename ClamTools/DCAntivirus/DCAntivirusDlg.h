#ifndef _DC_ANTI_VIRUS_DLG_H__
#define _DC_ANTI_VIRUS_DLG_H__
#pragma once

class CScanner;
class CDCAntivirusScanDlg;

class CDCAntiVirusDlg : public CDialog
{
public:
	CDCAntiVirusDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CDCAntiVirusDlg();

protected:
	LRESULT OnLoadDB(WPARAM wParam, LPARAM lParam);
	LRESULT OnHookSystem(WPARAM wParam, LPARAM lParam);

// Implementation
protected:
	HICON m_hIcon;
	virtual BOOL OnInitDialog();
	void OnPaint();
	HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	CScanner			*m_pScanner;
	CDCAntivirusScanDlg	*m_pScanDlg;
};

#endif
