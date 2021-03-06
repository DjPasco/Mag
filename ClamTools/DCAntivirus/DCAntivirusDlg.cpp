#include "stdafx.h"
#include "resource.h"
#include "DCAntiVirusDlg.h"

#include "DCAntiVirusSettingsDlg.h"
#include "DCAntiVirusManualScanDlg.h"
#include "DCAntiVirusMemoryScanDlg.h"
#include "DCAntiVirusHookDlg.h"
#include "Hook/Hook.h"

#include "../Utils/TraySendObj.h"
#include "../Utils/SendObj.h"
#include "../Utils/Registry.h"
#include "../Utils/Settings.h"
#include "../Utils/npipe.h"

#define IGNORE_HOOK

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CSendHelper
{
public:
	CSendObj m_obj;
	HWND m_hwnd;
};

CDCAntiVirusDlg::CDCAntiVirusDlg(CWnd* pParent)
	: CTrayDialog(IDD_DCANTIVIRUS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_SHIELD_FRAME);
	m_nProcCount = hook_utils::GetProcessCount();
}

CDCAntiVirusDlg::~CDCAntiVirusDlg()
{
	//
}

BEGIN_MESSAGE_MAP(CDCAntiVirusDlg, CTrayDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_MESSAGE(WM_COPYDATA,					OnCopyData)
	ON_BN_CLICKED(IDC_BUTTON3,				OnSettings)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE_DB,		OnUpdateDb)
	ON_BN_CLICKED(IDC_BUTTON_MANUAL_SCAN,	OnManualScan)
	ON_BN_CLICKED(IDC_BUTTON_MEMORYL_SCAN,	OnMemoryScan)
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
    //TraySetMenu(IDR_MENU1);

	TraySetMinimizeToTray(TRUE);
	TrayShow();

	RequestData();

	HookSystem();
	SetTimer(m_nTimer, 1000, NULL);

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

void CDCAntiVirusDlg::HookSystem()
{
#ifndef IGNORE_HOOK
	hook_utils::GlobalHook(true);
	//hook_utils::StartExeWithHookDll("c:\\WINDOWS\\NOTEPAD.EXE");
#endif

	RunHookInfoDlg();
}

void CDCAntiVirusDlg::OnTimer(UINT nIDEvent)
{
#ifndef IGNORE_HOOK
	int nNewCount = hook_utils::GetProcessCount();
	if(m_nProcCount != nNewCount)
	{
		m_nProcCount = nNewCount;
		hook_utils::GlobalHook(false);
	}
#endif

	RequestData();

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
	case EMessage:
		{
			GetDlgItem(IDC_EDIT_SERVICE_STATUS)->SetWindowText(pData->m_sText);
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
	CSendObj obj;
	obj.m_nType = ERequest;
	SendObj(obj);
}

UINT Send(LPVOID pParam)
{
	CSendHelper *helper = (CSendHelper *)pParam;

	COPYDATASTRUCT copy;
	copy.dwData = 1;
	copy.cbData = sizeof(helper->m_obj);
	copy.lpData = &helper->m_obj;

	::SendMessage(helper->m_hwnd, WM_COPYDATA, 0, (LPARAM) (LPVOID) &copy);

	delete helper;
	return 0;
}

bool CDCAntiVirusDlg::SendObj(CSendObj &obj)
{
	HWND hwnd = NULL;
	hwnd = ::FindWindow(NULL, sgServerName);

	if(NULL == hwnd)
	{
		return false;
	}

	CSendHelper *helper = new CSendHelper;
	helper->m_obj = obj;
	helper->m_hwnd = hwnd;

	AfxBeginThread(Send, (LPVOID)helper, THREAD_PRIORITY_HIGHEST);
	return true;
}

void CDCAntiVirusDlg::OnSettings()
{
	CDCAntiVirusSettingsDlg dlgSettings;
	dlgSettings.DoModal();
}

void CDCAntiVirusDlg::OnUpdateDb()
{
	CString sFreshClamPath		= path_utils::GetFreshClamPath();
	CString sParameters = path_utils::GenerateFrechClamParameters();

	CString sCmdFile = registry_utils::GetProfileString(sgSection, sgBaseDir, "");
	sCmdFile += "\\update.bat";
	FILE *pFile = fopen(sCmdFile, "w");
	if(NULL == pFile)
	{
		return;
	}

	fprintf(pFile, "@echo off\n\"%s\" %s\npause", sFreshClamPath, sParameters);
	fclose(pFile);

	STARTUPINFO si = {0};
    si.cb = sizeof si;

    PROCESS_INFORMATION pi = {0};
    CreateProcess(sCmdFile, NULL, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

    ::WaitForSingleObject(pi.hProcess, INFINITE);

	CSendObj obj;
	obj.m_nType = EReloadDB;
	SendObj(obj);
}

void CDCAntiVirusDlg::OnManualScan()
{
	CDCAntiVirusManualScanDlg dlg;
	dlg.DoModal();
}

void CDCAntiVirusDlg::OnMemoryScan()
{
	CDCAntiVirusMemoryScanDlg dlg;
	dlg.DoModal();
}

void CDCAntiVirusDlg::RunHookInfoDlg()
{
	CSettingsInfo info;
	if(settings_utils::Load(info))
	{
		if(!info.m_bLog)
		{
			return;
		}
	}

	CDCAntivirusHookDlg dlg;
	dlg.DoModal();
}

void CDCAntiVirusDlg::OnCancel()
{
#ifndef IGNORE_HOOK
	hook_utils::GlobalUnHook();
#endif	

	RunHookInfoDlg();

	CDialog::OnCancel();
}

void CDCAntiVirusDlg::OnOK()
{
#ifndef IGNORE_HOOK
	hook_utils::GlobalUnHook();
#endif

	RunHookInfoDlg();

	CDialog::OnOK();
}
