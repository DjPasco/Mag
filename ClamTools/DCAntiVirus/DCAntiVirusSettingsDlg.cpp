#include "stdafx.h"
#include "resource.h"
#include "DCAntiVirusSettingsDlg.h"

#include "DCAntiVirusSchedDlg.h"
#include "ScheduledTask.h"

#include "../Utils/Registry.h"
#include "../Utils/Settings.h"
#include "../Utils/SendObj.h"

#include <string>

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
END_MESSAGE_MAP()

BOOL CDCAntiVirusSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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
}

void CDCAntiVirusSettingsDlg::SaveRegistryData()
{
	CSettingsInfo info;
	GetData(info);	
	settings_utils::Save(info);
}

void CDCAntiVirusSettingsDlg::OnBnClickedOk()
{
	SaveRegistryData();

	CDialog::OnOK();

	HWND hwnd = NULL;
	hwnd = ::FindWindow(NULL, "DCAntiVirusScan");

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
		pTask->SetProgram     ("c:\\avg_free_stf_en_90_730a1834.exe");
		//pTask->SetParameters  ("Parameters");
		//pTask->SetStartingDir ("StartDir");
		//pTask->SetComment     ("Comment");
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
		pTask->SetProgram     ("c:\\avg_free_stf_en_90_730a1834.exe");
		//pTask->SetParameters  ("Parameters");
		//pTask->SetStartingDir ("StartDir");
		//pTask->SetComment     ("Comment");
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
