#include "stdafx.h"
#include "DetoursHookCenter.h"
#include "DetoursHookCenterDlg.h"
#include "HookUtils.h"
#include "../../Utils/SendObj.h"
#include <iostream>

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialog
{
public:
	CAboutDlg(): CDialog(IDD_ABOUTBOX){};
};

CDetoursHookCenterDlg::CDetoursHookCenterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DETOURSHOOKCENTER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CDetoursHookCenterDlg::~CDetoursHookCenterDlg()
{
	//
}

void CDetoursHookCenterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDetoursHookCenterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_HOOK_NOTEPAD, OnHookNotepad)
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
END_MESSAGE_MAP()


BOOL CDetoursHookCenterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_Log.SubclassDlgItem(IDC_RICH_LOG, this);

 	return TRUE;
}

void CDetoursHookCenterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDetoursHookCenterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDetoursHookCenterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDetoursHookCenterDlg::OnHookNotepad()
{
	hook_utils::LoadNotepadWithHookDll(m_Log);
	//hook_utils::EnumeratePayloads(m_Log);
}

LRESULT CDetoursHookCenterDlg::OnCopyData(WPARAM wParam, LPARAM lParam)
{
	PCOPYDATASTRUCT copy = (PCOPYDATASTRUCT) lParam;
	CString sText = ((CSendObj *)(copy->lpData))->m_sPath;
	m_Log.AddRichText(sText);

	return 0;
}

