#ifndef _DC_ANTI_VIRUS_DLG_H__
#define _DC_ANTI_VIRUS_DLG_H__
#pragma once

class CScanner;

class CDCAntiVirusDlg : public CDialog
{
public:
	CDCAntiVirusDlg(CWnd* pParent = NULL);	// standard constructor

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	LRESULT OnCopyData(WPARAM wParam, LPARAM lParam);
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
	CScanner *m_pScanner;
};

#endif
