#include "stdafx.h"
#include "resource.h"
#include "DCAntiVirusSettingsDlg.h"

#include "DCAntiVirusSchedDlg.h"
#include "ScheduledTask.h"

#include "../Utils/Registry.h"
#include "../Utils/Settings.h"
#include "../Utils/SendObj.h"

const char *sgNoTask = "\nNo task defined\n";

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CDCAntiVirusSettingsDlg::CDCAntiVirusSettingsDlg(CWnd* pParent)
	: CDialog(IDD_SETTINGS, pParent)
{
//
}

CDCAntiVirusSettingsDlg::~CDCAntiVirusSettingsDlg()
{
//
}

BEGIN_MESSAGE_MAP(CDCAntiVirusSettingsDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_SCAN,		OnCheckEnableScan)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_IDLE_SCAN,	OnCheckEnableIdleScan)
	ON_BN_CLICKED(IDOK,							OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_SCHED_SCAN, OnChangeSchedScan)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_SCHED_SCAN, OnDeleteSchedScan)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_UPD,		OnChangeUpd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_UPD,		OnDeleteUpd)
	ON_BN_CLICKED(IDC_BUTTON_ADD,				OnAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE,			OnRemove)
	ON_BN_CLICKED(IDC_BUTTON_ABOUT_SPEC_SCAN, &CDCAntiVirusSettingsDlg::OnAboutSpecScan)
END_MESSAGE_MAP()

BOOL CDCAntiVirusSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(!m_list.SubclassDlgItem(IDC_LIST_ITEMS, this))
	{
		return FALSE;
	}

	m_list.ModifyStyle(0, LVS_REPORT|LVS_SINGLESEL); 
	m_list.InsertColumn(0, "Items", LVCFMT_LEFT, 350);

	LoadRegistryData();
	LoadShedInfo();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDCAntiVirusSettingsDlg::UpdateOnAccessControls()
{
	BOOL bEnable = ((CButton *)GetDlgItem(IDC_CHECK_ENABLE_SCAN))->GetCheck();

	GetDlgItem(IDC_RADIO_ASK)		->EnableWindow(bEnable);
	GetDlgItem(IDC_RADIO_DENNY)		->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_DEEP_SCANN)->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_OFFICE)	->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_ARCHIVE)	->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_PDF)		->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_HTML)		->EnableWindow(bEnable);
	GetDlgItem(IDC_STATIC_TYPES)	->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_TYPES)		->EnableWindow(bEnable);
}

void CDCAntiVirusSettingsDlg::UpdateIdleScanControls()
{
	BOOL bEnable = ((CButton *)GetDlgItem(IDC_CHECK_ENABLE_IDLE_SCAN))->GetCheck();

	GetDlgItem(IDC_EDIT_IDLE_TIME)	->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_LOAD)			->EnableWindow(bEnable);
}

void CDCAntiVirusSettingsDlg::OnCheckEnableScan()
{
	UpdateOnAccessControls();
}

void CDCAntiVirusSettingsDlg::OnCheckEnableIdleScan()
{
	UpdateIdleScanControls();
}

void CDCAntiVirusSettingsDlg::LoadRegistryData()
{
	CSettingsInfo info;
	if(!settings_utils::Load(info))
	{
		UpdateAllControls();
		return;
	}

	SetData(info);
	UpdateAllControls();

	CString sItems = registry_utils::GetProfileString(sgSection, sgShedScanItems, "");
	if(!sItems.IsEmpty())
	{
		char *token;
		char *str = sItems.GetBuffer(0);
		token = strtok( str, "\n" );
		if(NULL != token)
		{
			m_list.InsertItem(m_list.GetItemCount(), token);
		}

		while(token != NULL)
		{
			token = strtok( NULL, "\n" );
			if(NULL != token)
			{
				m_list.InsertItem(m_list.GetItemCount(), token);
			}
		}
	}

	CString sTypes = registry_utils::GetProfileString(sgSection, sgSchedFileTypes, "");	
	GetDlgItem(IDD_EDIT_TYPES_SETT)->SetWindowText(sTypes);

	CString sUse = registry_utils::GetProfileString(sgSection, sgSchedInternalDB, "");
	BOOL bUse(FALSE);
	if(0 == sUse.CompareNoCase("1"))
	{
		bUse = TRUE;
	}

	CheckControl(IDD_USE_INTERNAL_SETT, bUse);
}

void CDCAntiVirusSettingsDlg::SaveRegistryData()
{
	CSettingsInfo info;
	GetData(info);	
	settings_utils::Save(info);

	CString sItems;
	int nCount = m_list.GetItemCount();
	for(int i = 0; i < nCount; ++i)
	{
		sItems += m_list.GetItemText(i, 0);
		if(i+1 != nCount)
		{
			sItems += "\n";
		}
	}

	registry_utils::WriteProfileString(sgSection, sgShedScanItems, sItems);

	CString sTypes;	
	GetDlgItem(IDD_EDIT_TYPES_SETT)->GetWindowText(sTypes);
	registry_utils::WriteProfileString(sgSection, sgSchedFileTypes, sTypes);

	BOOL bUseInternal = ControlChecked(IDD_USE_INTERNAL_SETT);
	CString sUse;
	sUse.Format("%d", bUseInternal);
	registry_utils::WriteProfileString(sgSection, sgSchedInternalDB, sUse);
}

void CDCAntiVirusSettingsDlg::OnBnClickedOk()
{
	SaveRegistryData();

	CDialog::OnOK();

	HWND hwnd = NULL;
	hwnd = ::FindWindow(NULL, sgServerName);

	if(NULL != hwnd)
	{
		CSendObj obj;
		obj.m_nType = EReloadSettings;

		COPYDATASTRUCT copy;
		copy.dwData = 1;
		copy.cbData = sizeof(obj);
		copy.lpData = &obj;

		::SendMessage(hwnd, WM_COPYDATA, 0, (LPARAM) (LPVOID) &copy);
	}
}

void CDCAntiVirusSettingsDlg::GetData(CSettingsInfo &info)
{
	GetScanData(info);
	GetIdleData(info);
}

void CDCAntiVirusSettingsDlg::GetScanData(CSettingsInfo &info)
{
	info.m_bScan =	ControlChecked(IDC_CHECK_ENABLE_SCAN);
	info.m_bDeny =	ControlChecked(IDC_RADIO_DENNY);

	info.m_bDeep		= ControlChecked(IDC_CHECK_DEEP_SCANN);
	info.m_bOffice		= ControlChecked(IDC_CHECK_OFFICE);
	info.m_bArchives	= ControlChecked(IDC_CHECK_ARCHIVE);
	info.m_bPDF			= ControlChecked(IDC_CHECK_PDF);
	info.m_bHTML		= ControlChecked(IDC_CHECK_HTML);

	GetDlgItem(IDC_EDIT_TYPES)->GetWindowText(info.m_sFilesTypes);
}

void CDCAntiVirusSettingsDlg::GetIdleData(CSettingsInfo &info)
{
	info.m_bIdle = ControlChecked(IDC_CHECK_ENABLE_IDLE_SCAN);

	CString sTime;
	GetDlgItem(IDC_EDIT_IDLE_TIME)->GetWindowText(sTime);
	info.m_nIdleTime = atoi(sTime);

	CString sLoad;
	GetDlgItem(IDC_EDIT_LOAD)->GetWindowText(sLoad);
	info.m_nCPULoad = atoi(sLoad);
}

bool CDCAntiVirusSettingsDlg::ControlChecked(UINT ID)
{
	if(((CButton *)GetDlgItem(ID))->GetCheck())
	{
		return true;
	}	

	return false;
}

void CDCAntiVirusSettingsDlg::SetData(const CSettingsInfo &info)
{
	SetScanData(info);
	SetIdleData(info);
}

void CDCAntiVirusSettingsDlg::SetScanData(const CSettingsInfo &info)
{
	CheckControl(IDC_CHECK_ENABLE_SCAN,	info.m_bScan);

	if(info.m_bDeny)
	{
		CheckControl(IDC_RADIO_DENNY,	true);
	}
	else
	{
		CheckControl(IDC_RADIO_ASK,		true);
	}

	CheckControl(IDC_CHECK_DEEP_SCANN,	info.m_bDeep);
	CheckControl(IDC_CHECK_OFFICE,		info.m_bOffice);
	CheckControl(IDC_CHECK_ARCHIVE,		info.m_bArchives);
	CheckControl(IDC_CHECK_PDF,			info.m_bPDF);
	CheckControl(IDC_CHECK_HTML,		info.m_bHTML);

	GetDlgItem(IDC_EDIT_TYPES)->SetWindowText(info.m_sFilesTypes);
}

void CDCAntiVirusSettingsDlg::SetIdleData(const CSettingsInfo &info)
{
	CheckControl(IDC_CHECK_ENABLE_IDLE_SCAN, info.m_bIdle);

	CString sTime;
	sTime.Format("%d", info.m_nIdleTime);
	GetDlgItem(IDC_EDIT_IDLE_TIME)->SetWindowText(sTime);

	CString sLoad;
	sLoad.Format("%d", info.m_nCPULoad);
	GetDlgItem(IDC_EDIT_LOAD)->SetWindowText(sLoad);
}

void CDCAntiVirusSettingsDlg::CheckControl(UINT ID, BOOL bSheck)
{
	((CButton *)GetDlgItem(ID))->SetCheck(bSheck);
}

void CDCAntiVirusSettingsDlg::UpdateAllControls()
{
	UpdateOnAccessControls();
	UpdateIdleScanControls();
}

void CDCAntiVirusSettingsDlg::OnChangeSchedScan()
{
	CScheduledTask *pTask = new CScheduledTask;

	CDCAntiVirusShedDlg dlgSched("Change scheduled scan settings", pTask);
	
	if(IDOK == dlgSched.DoModal())
	{
		pTask->SetProgram(path_utils::GetAppPath());
		pTask->SetParameters("scan");
		if(S_OK == pTask->SaveTask(sgShedScanTaskName, false))
		{
			SetTaskInfo(pTask, IDC_STATIC_SCAN_SHED, sgShedScanTaskInfo);
		}
	}

	delete pTask;
}

void CDCAntiVirusSettingsDlg::OnDeleteSchedScan()
{
    if(S_OK == CScheduledTask::DeleteTask(sgShedScanTaskName))
    {
		MessageBox("Scan task deleted.", 0, MB_ICONINFORMATION);
		SetTaskInfo(NULL, IDC_STATIC_SCAN_SHED, sgShedScanTaskInfo);
    }
}

void CDCAntiVirusSettingsDlg::OnChangeUpd()
{
	CScheduledTask *pTask = new CScheduledTask;

	CDCAntiVirusShedDlg dlgSched("Change scheduled update settings", pTask);
	
	if(IDOK == dlgSched.DoModal())
	{
		CString sFreshClamPath		= path_utils::GetFreshClamPath();
		CString sParameters = path_utils::GenerateFrechClamParameters();;
		pTask->SetProgram(sFreshClamPath);
		pTask->SetParameters(sParameters);
		if(S_OK == pTask->SaveTask(sgShedUpdTaskName, false))
		{
			SetTaskInfo(pTask, IDC_STATIC_UPD_SHED, sgShedUpdTaskInfo);
		}
	}

	delete pTask;
}

void CDCAntiVirusSettingsDlg::OnDeleteUpd()
{
    if(S_OK == CScheduledTask::DeleteTask(sgShedUpdTaskName))
    {
		MessageBox("Update task deleted.", 0, MB_ICONINFORMATION);
		SetTaskInfo(NULL, IDC_STATIC_UPD_SHED, sgShedUpdTaskInfo);
    }
}

void CDCAntiVirusSettingsDlg::SetTaskInfo(CScheduledTask *pTask, UINT ID, LPCSTR sEntry)
{
	if(NULL == pTask)
	{
		GetDlgItem(ID)->SetWindowText(sgNoTask);
		registry_utils::WriteProfileString(sgSection, sEntry, sgNoTask);
	}
	else
	{
		CString sTaskText;
		CTime time;
		pTask->GetStartDateTime(time);
		sTaskText = time.Format("Start date: %Y:%m:%d\n");
		sTaskText += time.Format("Start time: %H:%M\n");
		sTaskText += "Frequency: ";
		switch(pTask->GetFrequency())
		{
		case CScheduledTask::freqDaily :
			{
				sTaskText += "Daily";
			}
			break;
		case CScheduledTask::freqWeekly :
			{
				sTaskText += "Weekly";
			}
			break;
		case CScheduledTask::freqMonthly :
			{
				sTaskText += "Monthly";
			}
			break;
		}

		GetDlgItem(ID)->SetWindowText(sTaskText);
		registry_utils::WriteProfileString(sgSection, sEntry, sTaskText);
	}
}

void CDCAntiVirusSettingsDlg::LoadShedInfo()
{
	CString sScan = registry_utils::GetProfileString(sgSection, sgShedScanTaskInfo, "");
	if(sScan.IsEmpty())
	{
		registry_utils::WriteProfileString(sgSection, sgShedScanTaskInfo, sgNoTask);
		GetDlgItem(IDC_STATIC_SCAN_SHED)->SetWindowText(sgNoTask);
	}
	else
	{
		GetDlgItem(IDC_STATIC_SCAN_SHED)->SetWindowText(sScan);
	}

	CString sUpdate = registry_utils::GetProfileString(sgSection, sgShedUpdTaskInfo, "");
	if(sUpdate.IsEmpty())
	{
		registry_utils::WriteProfileString(sgSection, sgShedUpdTaskInfo, sgNoTask);
		GetDlgItem(IDC_STATIC_UPD_SHED)->SetWindowText(sgNoTask);
	}
	else
	{
		GetDlgItem(IDC_STATIC_UPD_SHED)->SetWindowText(sUpdate);
	}
}

void CDCAntiVirusSettingsDlg::OnAdd()
{
	BROWSEINFO bi = { 0 };
    char path[MAX_PATH];
    bi.lpszTitle = "Pick item to scan";
    bi.pszDisplayName = path;
	bi.ulFlags |= BIF_BROWSEINCLUDEFILES|BIF_NONEWFOLDERBUTTON|BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if(NULL != pidl)
    {
		char sPathName[MAX_PATH]; 
		BOOL bRet = SHGetPathFromIDList(pidl, sPathName);
		if(bRet)
		{
			int nCount = m_list.GetItemCount();
			m_list.InsertItem(nCount, sPathName);
		}

	     // free memory used
        IMalloc *imalloc = 0;
        if(SUCCEEDED(SHGetMalloc(&imalloc)))
        {
            imalloc->Free(pidl);
            imalloc->Release();
        }
    }
}

void CDCAntiVirusSettingsDlg::OnRemove()
{
	int nSel = m_list.GetSelectionMark();
	if(-1 != nSel)
	{
		m_list.DeleteItem(nSel);
	}
}

void CDCAntiVirusSettingsDlg::OnAboutSpecScan()
{
	CString sSpecScanHelp = "Additional actions are taken on selected items.\nFor example, HTML files are scanned for script viruses.";
	MessageBox(sSpecScanHelp, "About special scan", MB_ICONINFORMATION);
}
