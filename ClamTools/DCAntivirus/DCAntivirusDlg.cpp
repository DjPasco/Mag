#include "stdafx.h"
#include "Resource.h"
#include "DCAntiVirusDlg.h"

#include "../Detours/Utils/SendObj.h"
#include "Scanner/Scanner.h"
#include "Hook/Hook.h"
#include <vector>

#define WM_LOAD_DB		WM_USER+1
#define WM_HOOK_SYSTEM	WM_LOAD_DB+1

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDCAntiVirusDlg::CDCAntiVirusDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DCANTIVIRUS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pScanner = new CScanner;
}

void CDCAntiVirusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDCAntiVirusDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
	ON_MESSAGE(WM_LOAD_DB, OnLoadDB)
	ON_MESSAGE(WM_HOOK_SYSTEM, OnHookSystem)
END_MESSAGE_MAP()

BOOL CDCAntiVirusDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	this->PostMessage(WM_LOAD_DB);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDCAntiVirusDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CDCAntiVirusDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

LRESULT CDCAntiVirusDlg::OnCopyData(WPARAM wParam, LPARAM lParam)
{
	PCOPYDATASTRUCT copy = (PCOPYDATASTRUCT) lParam;
	CString sFile = ((CSendObj *)(copy->lpData))->m_sPath;
	CString sVirusName;
	m_pScanner->ScanFile(sFile, sVirusName);
	return 0;
}

LRESULT CDCAntiVirusDlg::OnLoadDB(WPARAM wParam, LPARAM lParam)
{
	this->GetDlgItem(IDC_EDIT_DB_STATUS)->SetWindowText("Loading...");
	this->GetDlgItem(IDC_EDIT_DB_STATUS)->UpdateWindow();

	if(m_pScanner->LoadDatabases())
	{
		this->GetDlgItem(IDC_EDIT_DB_STATUS)->SetWindowText("Database loaded.");
	}
	else
	{
		this->GetDlgItem(IDC_EDIT_DB_STATUS)->SetWindowText("Invalid database.");
	}

	this->PostMessage(WM_HOOK_SYSTEM);

	return 0;
}

LRESULT CDCAntiVirusDlg::OnHookSystem(WPARAM wParam, LPARAM lParam)
{
	hook_utils::StartExeWithHookDll("c:\\WINDOWS\\NOTEPAD.EXE");
	return 0;
}


