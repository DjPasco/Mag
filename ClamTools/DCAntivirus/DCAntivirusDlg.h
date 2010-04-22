#pragma once

struct cl_engine;

// CDCAntivirusDlg dialog
class CDCAntivirusDlg : public CDialog
{
// Construction
public:
	CDCAntivirusDlg(CWnd* pParent = NULL);	// standard constructor
	~CDCAntivirusDlg();

// Dialog Data
	enum { IDD = IDD_DCANTIVIRUS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	LRESULT OnCopyData(WPARAM wParam, LPARAM lParam);

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	cl_engine *m_pEngine;
};
