#if !defined(AFX_SCANDLG_H__A75B2D09_D9D9_42E4_9666_7C75EAC22E81__INCLUDED_)
#define AFX_SCANDLG_H__A75B2D09_D9D9_42E4_9666_7C75EAC22E81__INCLUDED_

#define EXIT_SCAN_MSG  "EXIT_SCAN_MSG "
const UINT EXIT_SCAN = ::RegisterWindowMessage(EXIT_SCAN_MSG);

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>

#define SIZE 1000

class CScanDlg : public CDialog
{
public:
	CScanDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_SCAN_DIALOG };

	void Scan();

protected:
	CString Readline(SOCKET *client);
	void DoAction(CString &sMessage, SOCKET *client);
	bool SkipScan(LPCSTR sFile);
	bool CheckSelTypes(LPCSTR sFile);
	void AddFile(LPCSTR sFile);

	void AddTypes();

	void OnScanOnly();
	void OnScanType();

	void OnClear();

	void OnNew();
	void OnRemove();

public:
	void OnExitScan();

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	void OnPaint();
	HCURSOR OnQueryDragIcon();

	CWinThread *m_pScanTask;

	std::string m_files[SIZE];
	std::vector<CString> m_types;
	int m_nCurrPos;

	DECLARE_MESSAGE_MAP()
};

#endif
