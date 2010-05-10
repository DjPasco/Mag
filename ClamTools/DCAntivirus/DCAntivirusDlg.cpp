#include "stdafx.h"
#include "DCAntiVirus.h"
#include "DCAntiVirusDlg.h"
#include "Hook/Hook.h"

#define WM_HOOK_SYSTEM	WM_USER+1

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDCAntiVirusDlg::CDCAntiVirusDlg(CWnd* pParent)
	: CTrayDialog(IDD_DCANTIVIRUS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_SHIELD_FRAME);
	m_nProcCount = hook_utils::GetProcessCount();
}


CDCAntiVirusDlg::~CDCAntiVirusDlg()
{
	hook_utils::GlobalUnHook();
}

BEGIN_MESSAGE_MAP(CDCAntiVirusDlg, CTrayDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_MESSAGE(WM_HOOK_SYSTEM, OnHookSystem)
END_MESSAGE_MAP()

BOOL CDCAntiVirusDlg::OnInitDialog()
{
	CTrayDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	TraySetIcon(IDI_SHIELD_FRAME);
    TraySetToolTip("DCAntiVirus");
    TraySetMenu(IDR_MENU1);

	TraySetMinimizeToTray(TRUE);
	TrayShow();

	this->SendMessage(WM_HOOK_SYSTEM);
	SetTimer(m_nTimer, 1000, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDCAntiVirusDlg::OnPaint()
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
		CTrayDialog::OnPaint();
	}
}

HCURSOR CDCAntiVirusDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CDCAntiVirusDlg::OnHookSystem(WPARAM wParam, LPARAM lParam)
{
	hook_utils::GlobalHook(true);
	//hook_utils::StartExeWithHookDll("c:\\WINDOWS\\NOTEPAD.EXE");

	return 0;
}

void CDCAntiVirusDlg::OnTimer(UINT nIDEvent)
{
	int nNewCount = hook_utils::GetProcessCount();
	if(m_nProcCount != nNewCount)
	{
		m_nProcCount = nNewCount;
		hook_utils::GlobalHook(false);
	}

	CTrayDialog::OnTimer(nIDEvent);
}
