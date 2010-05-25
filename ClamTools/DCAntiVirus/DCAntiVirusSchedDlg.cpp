#include "stdafx.h"
#include "resource.h"
#include "DCAntiVirusSchedDlg.h"
#include "ScheduledTask.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CDCAntiVirusShedDlg::CDCAntiVirusShedDlg(LPCSTR sTitle, CScheduledTask *pTask, CWnd* pParent)
	: CDialog(IDD_DIALOG_SHED, pParent),
	  m_pTask(pTask),
	  m_sTitle(sTitle)
{
//
}

CDCAntiVirusShedDlg::~CDCAntiVirusShedDlg()
{
//
}

BEGIN_MESSAGE_MAP(CDCAntiVirusShedDlg, CDialog)
	ON_BN_CLICKED(IDOK,							OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CDCAntiVirusShedDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	((CButton *)this->GetDlgItem(IDC_DAILY_SHED))->SetCheck(TRUE);

	this->SetWindowText(m_sTitle);

	// Gain a pointer to the control
	CDateTimeCtrl* pCtrl = (CDateTimeCtrl*) GetDlgItem(IDC_STARTTIME_SHED);
	ASSERT(pCtrl != NULL);
	pCtrl->SetFormat(_T("hh:mm"));

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDCAntiVirusShedDlg::OnBnClickedOk()
{
	CDateTimeCtrl* pStartDateCtrl;
	SYSTEMTIME     stBegin;
	SYSTEMTIME     stTime;

    pStartDateCtrl = static_cast<CDateTimeCtrl*>(GetDlgItem(IDC_STARTDATE_SHED));
    pStartDateCtrl->GetTime(&stBegin);

	CDateTimeCtrl* pStartTimeCtrl;
	pStartTimeCtrl = static_cast<CDateTimeCtrl*>(GetDlgItem(IDC_STARTTIME_SHED));
	pStartTimeCtrl->GetTime(&stTime);
	stBegin.wHour   = stTime.wHour;
	stBegin.wMinute = stTime.wMinute;

    m_pTask->SetStartDateTime(stBegin);

	if(ControlChecked(IDC_DAILY_SHED))
		m_pTask->SetFrequency(CScheduledTask::freqDaily);
	else if(ControlChecked(IDC_WEEKLY_SHED))
		m_pTask->SetFrequency(CScheduledTask::freqWeekly);
	else if(ControlChecked(IDC_MONTHLY_SHED))
		m_pTask->SetFrequency(CScheduledTask::freqMonthly);

	CDialog::OnOK();
}

bool CDCAntiVirusShedDlg::ControlChecked(UINT ID)
{
	if(((CButton *)this->GetDlgItem(ID))->GetCheck())
	{
		return true;
	}	

	return false;
}