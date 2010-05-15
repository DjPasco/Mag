#include "stdafx.h"
#include "resource.h"
#include "DCAntiVirusSettingsDlg.h"

#include "DCAntiVirusSchedDlg.h"
#include "ScheduledTask.h"

#include <string>

const char *sgSettingsWriteTemplate = "%d %d %d %d %d %d %d %d %d %d";
const char *sgSettingsReadTemplate = "%d%d%d%d%d%d%d%d%d%d";
const char *sgSection = "Setting";
const char *sgUserEntry = "User";

const char *sgShedScanTaskName = "DCAntiVirusScan";
const char *sgShedScanTaskInfo = "SheduledScanInfo";

const char *sgShedUpdTaskName = "DCAntiVirusUpdate";
const char *sgShedUpdTaskInfo = "SheduledUpdateInfo";


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

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDCAntiVirusSettingsDlg::UpdateOnAccessControls()
{
	BOOL bEnable = ((CButton *)this->GetDlgItem(IDC_CHECK_ENABLE_SCAN))->GetCheck();

	this->GetDlgItem(IDC_RADIO_ASK)			->EnableWindow(bEnable);
	this->GetDlgItem(IDC_RADIO_DENNY)		->EnableWindow(bEnable);
	this->GetDlgItem(IDC_CHECK_DEEP_SCANN)	->EnableWindow(bEnable);
	this->GetDlgItem(IDC_CHECK_OFFICE)		->EnableWindow(bEnable);
	this->GetDlgItem(IDC_CHECK_ARCHIVE)		->EnableWindow(bEnable);
	this->GetDlgItem(IDC_CHECK_PDF)			->EnableWindow(bEnable);
	this->GetDlgItem(IDC_CHECK_HTML)		->EnableWindow(bEnable);
}

void CDCAntiVirusSettingsDlg::UpdateIdleScanControls()
{
	BOOL bEnable = ((CButton *)this->GetDlgItem(IDC_CHECK_ENABLE_IDLE_SCAN))->GetCheck();

	this->GetDlgItem(IDC_EDIT_IDLE_TIME)	->EnableWindow(bEnable);
	this->GetDlgItem(IDC_EDIT_LOAD)			->EnableWindow(bEnable);
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
	CString sSettings = AfxGetApp()->GetProfileString(sgSection, sgUserEntry, "");

	if(sSettings.IsEmpty())
	{
		UpdateAllControls();
		return;
	}

	CSettingsInfo info;
	sscanf(sSettings, sgSettingsReadTemplate, &info.m_bScan,
											  &info.m_bDeny,
											  &info.m_bDeep,
											  &info.m_bOffice,
											  &info.m_bArchives,
											  &info.m_bPDF,
											  &info.m_bHTML,
											  &info.m_bIdle,
											  &info.m_nIdleTime,
											  &info.m_nCPULoad);

	SetData(info);
	UpdateAllControls();
}

void CDCAntiVirusSettingsDlg::SaveRegistryData()
{
	CSettingsInfo info;
	GetData(info);
	
	CString sData;
	sData.Format(sgSettingsWriteTemplate, info.m_bScan,
										  info.m_bDeny,
										  info.m_bDeep,
										  info.m_bOffice,
										  info.m_bArchives,
										  info.m_bPDF,
										  info.m_bHTML,
										  info.m_bIdle,
										  info.m_nIdleTime,
										  info.m_nCPULoad);

	AfxGetApp()->WriteProfileString(sgSection, sgUserEntry, sData);
}

void CDCAntiVirusSettingsDlg::OnBnClickedOk()
{
	SaveRegistryData();

	CDialog::OnOK();
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
	this->GetDlgItem(IDC_EDIT_IDLE_TIME)->GetWindowText(sTime);
	info.m_nIdleTime = atoi(sTime);

	CString sLoad;
	this->GetDlgItem(IDC_EDIT_LOAD)->GetWindowText(sLoad);
	info.m_nCPULoad = atoi(sLoad);
}

bool CDCAntiVirusSettingsDlg::ControlChecked(UINT ID)
{
	if(((CButton *)this->GetDlgItem(ID))->GetCheck())
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
	this->GetDlgItem(IDC_EDIT_IDLE_TIME)->SetWindowText(sTime);

	CString sLoad;
	sLoad.Format("%d", info.m_nCPULoad);
	this->GetDlgItem(IDC_EDIT_LOAD)->SetWindowText(sLoad);
}

void CDCAntiVirusSettingsDlg::CheckControl(UINT ID, BOOL bSheck)
{
	((CButton *)this->GetDlgItem(ID))->SetCheck(bSheck);
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
			AfxGetApp()->WriteProfileString(sgSection, sgShedScanTaskInfo, "Duomenys");
		}
	}

	delete pTask;
}

void CDCAntiVirusSettingsDlg::OnDeleteSchedScan()
{
    if(S_OK == CScheduledTask::DeleteTask(sgShedScanTaskName))
    {
		MessageBox("Scan task deleted.", 0, MB_ICONINFORMATION);
		AfxGetApp()->WriteProfileString(sgSection, sgShedScanTaskInfo, "");
		SetTaskInfo(NULL, IDC_STATIC_SCAN_SHED);
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
			AfxGetApp()->WriteProfileString(sgSection, sgShedUpdTaskInfo, "Duomenys");
		}
	}

	delete pTask;
}

void CDCAntiVirusSettingsDlg::OnDeleteUpd()
{
    if(S_OK == CScheduledTask::DeleteTask(sgShedUpdTaskName))
    {
		MessageBox("Update task deleted.", 0, MB_ICONINFORMATION);
		AfxGetApp()->WriteProfileString(sgSection, sgShedUpdTaskInfo, "");
		SetTaskInfo(NULL, IDC_STATIC_UPD_SHED);
    }
}

void CDCAntiVirusSettingsDlg::SetTaskInfo(CScheduledTask *pTask, UINT ID)
{
	if(NULL == pTask)
	{
		this->GetDlgItem(ID)->SetWindowText("");
	}
	else
	{
		CString sTaskText;
		sTaskText;
	}
}

