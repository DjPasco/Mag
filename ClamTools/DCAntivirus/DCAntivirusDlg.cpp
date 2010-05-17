#include "stdafx.h"
#include "resource.h"
#include "DCAntiVirusDlg.h"

#include "DCAntiVirusSettingsDlg.h"
#include "Hook/Hook.h"

#include "../Utils/TraySendObj.h"
#include "../Utils/SendObj.h"
#include "../Utils/Registry.h"

#define WM_HOOK_SYSTEM	WM_USER+1

#define IGNORE_HOOK

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
#ifndef IGNORE_HOOK
	hook_utils::GlobalUnHook();
#endif
}

BEGIN_MESSAGE_MAP(CDCAntiVirusDlg, CTrayDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_MESSAGE(WM_HOOK_SYSTEM,	OnHookSystem)
	ON_MESSAGE(WM_COPYDATA,		OnCopyData)
	ON_BN_CLICKED(IDC_BUTTON3,	OnSettings)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_DB, &CDCAntiVirusDlg::OnUpdateDb)
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

	RequestData();

	this->SendMessage(WM_HOOK_SYSTEM);
	SetTimer(m_nTimer, 1000, NULL);

	GetDlgItem(IDC_EDIT11)->SetWindowPos(NULL, 0, 0, 1, 20, SWP_NOMOVE|SWP_NOOWNERZORDER);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDCAntiVirusDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	if (IsIconic())
	{
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
#ifndef IGNORE_HOOK
	hook_utils::GlobalHook(true);
	//hook_utils::StartExeWithHookDll("c:\\WINDOWS\\NOTEPAD.EXE");
#endif

	return 0;
}

void CDCAntiVirusDlg::OnTimer(UINT nIDEvent)
{
#ifndef IGNORE_HOOK
	int nNewCount = hook_utils::GetProcessCount();
	if(m_nProcCount != nNewCount)
	{
		m_nProcCount = nNewCount;
		//hook_utils::GlobalHook(false);
	}
#endif

	CTrayDialog::OnTimer(nIDEvent);
}

LRESULT CDCAntiVirusDlg::OnCopyData(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);

	PCOPYDATASTRUCT copy = (PCOPYDATASTRUCT) lParam;

	CTraySendObj *pData = NULL;
	pData = (CTraySendObj *)copy->lpData;

	EInfoType type = (EInfoType)pData->m_nType;
	switch(type)
	{
	case EData:
		{
			CString sVersion, sSigs, sTime;
			sVersion.Format("%d", pData->m_nVersion);
			sSigs.Format("%d", pData->m_nSigs);
			sTime.Format("%s", pData->m_sText);
			
			if(pData->m_bMain)
			{
				GetDlgItem(IDC_EDIT_MAIN_DB_VERSION)->SetWindowText(sVersion);
				GetDlgItem(IDC_EDIT_MAIN_DB_TIME)->SetWindowText(sTime);
				GetDlgItem(IDC_EDIT_MAIN_DB_SIG)->SetWindowText(sSigs);
			}
			else
			{
				GetDlgItem(IDC_EDIT_DAILY_DB_VERSION)->SetWindowText(sVersion);
				GetDlgItem(IDC_EDIT_DAILY_DB_TIME)->SetWindowText(sTime);
				GetDlgItem(IDC_EDIT_DAILY_DB_SIG)->SetWindowText(sSigs);
			}

			CString sCount;
			sCount.Format("%d", pData->m_nFilesCount);
			GetDlgItem(IDC_EDIT_FILES_COUNT)->SetWindowText(sCount);
		}
		break;
	case EFile:
		{
			CString sFile;
			sFile.Format("%s", pData->m_sText);
			GetDlgItem(IDC_LAST_FILE_EDIT)->SetWindowText(sFile);

			CString sVirus;
			sVirus.Format("%s", pData->m_sText2);
			GetDlgItem(IDC_EDIT_VIRUS_NAME)->SetWindowText(sVirus);
			
			CString sCount;
			sCount.Format("%d", pData->m_nFilesCount);
			GetDlgItem(IDC_EDIT_FILES_COUNT)->SetWindowText(sCount);
		}
		break;
	default:
		{
			ASSERT(FALSE);
		}
		break;
	}

	return 0;
}

void CDCAntiVirusDlg::RequestData()
{
	HWND hwnd = NULL;
	hwnd = ::FindWindow(NULL, "DCAntiVirusScan");

	if(NULL == hwnd)
	{
		GetDlgItem(IDC_EDIT_SERVICE_STATUS)->SetWindowText("Stopped!");
		return;
	}

	GetDlgItem(IDC_EDIT_SERVICE_STATUS)->SetWindowText("Running");

	CSendObj obj;
	obj.m_nType = ERequest;

	COPYDATASTRUCT copy;
	copy.dwData = 1;
	copy.cbData = sizeof(obj);
	copy.lpData = &obj;

	::SendMessage(hwnd, WM_COPYDATA, 0, (LPARAM) (LPVOID) &copy);
}

void CDCAntiVirusDlg::OnSettings()
{
	CDCAntiVirusSettingsDlg dlgSettings;
	dlgSettings.DoModal();
}

void CDCAntiVirusDlg::OnUpdateDb()
{
	CString sFreshClamPath		= path_utils::GetFreshClamPath();
	CString sParameters = path_utils::GenerateFrechClamParameters();;

	ShellExecute(this->GetSafeHwnd(), "open", sFreshClamPath, sParameters, "", SW_SHOW); 
}
