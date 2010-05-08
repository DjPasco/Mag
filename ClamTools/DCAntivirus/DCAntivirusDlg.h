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


	virtual BOOL OnInitDialog();
	void OnPaint();
	void OnTimer(UINT nIDEvent);
	HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	HICON m_hIcon;

	CScanner			*m_pScanner;
	CDCAntivirusScanDlg	*m_pScanDlg;

	int m_nProcCount;
	UINT_PTR m_nTimer;
};
