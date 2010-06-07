#include "stdafx.h"
#include "resource.h"
#include "DCAntiVirusScheduledScanDlg.h"

#include "../Utils/Registry.h"
#include "ManualScanUtils.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

#define WM_START_SCHED_SCAN WM_USER+101

CDCAntiVirusScheduledScanDlg::CDCAntiVirusScheduledScanDlg(CWnd* pParent)
	: CDialog(IDD_DIALOG_SCHED_SCAN, pParent),
	  m_bScanning(false)
{
//
}

CDCAntiVirusScheduledScanDlg::~CDCAntiVirusScheduledScanDlg()
{
//
}

BEGIN_MESSAGE_MAP(CDCAntiVirusScheduledScanDlg, CDialog)
	ON_MESSAGE(WM_START_SCHED_SCAN,	  OnStartScan)
	ON_BN_CLICKED(IDD_STOP, &CDCAntiVirusScheduledScanDlg::OnStop)
END_MESSAGE_MAP()

BOOL CDCAntiVirusScheduledScanDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(!m_listScanItems.SubclassDlgItem(IDD_ITEMS_LIST_SC, this))
	{
		return FALSE;
	}

	m_listScanItems.ModifyStyle(0, LVS_REPORT|LVS_SINGLESEL); 
	m_listScanItems.InsertColumn(0, "Items", LVCFMT_LEFT, 350);

	if(!m_progres.SubclassDlgItem(IDD_PROGRES_SC, this))
	{
		return FALSE;
	}	

	FillScanItems();
	FillItemTypes();

	//Starting scan
	PostMessage(WM_START_SCHED_SCAN);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

LRESULT CDCAntiVirusScheduledScanDlg::OnStartScan(WPARAM wParam, LPARAM lParam)
{
	EnableStartItems(FALSE);
	EnableProgresItems(TRUE);
	m_bScanning = true;
	m_nCount = 0;
	m_tStart = CTime::GetCurrentTime();

	CScanItems files;
	EnumerateFiles(files);
	CScanOptions *pOpt = new CScanOptions;
	pOpt->m_Items = files;
	pOpt->m_pObs = this;
	pOpt->m_bUseInternal = GetUseInternalDB();
	
	AfxBeginThread(manual_scan_utils::Scan, (LPVOID)pOpt, priority_utils::GetRealPriority(path_utils::GetPriority()));

	return 0;
}

void CDCAntiVirusScheduledScanDlg::EnableProgresItems(BOOL bEnable)
{
	GetDlgItem(IDD_STATIC_SCAN_PROGRES_SC)->EnableWindow(bEnable);
	GetDlgItem(IDD_PROGRES_SC)->EnableWindow(bEnable);

	GetDlgItem(IDD_STATIC_CURRENT_SC)->EnableWindow(bEnable);
	GetDlgItem(IDD_EDIT_CUR_SC)->EnableWindow(bEnable);

	GetDlgItem(IDD_STATIC_CURR_ACTION_SC)->EnableWindow(bEnable);
}

void CDCAntiVirusScheduledScanDlg::EnableStartItems(BOOL bEnable)
{
	GetDlgItem(IDD_ITEMS_LIST_SC)->EnableWindow(bEnable);

	GetDlgItem(IDOK)->EnableWindow(bEnable);
	GetDlgItem(IDD_USE_INTERNAL)->EnableWindow(bEnable);

	GetDlgItem(IDD_EDIT_TYPES_SC)->EnableWindow(bEnable);
}

bool CDCAntiVirusScheduledScanDlg::Continue()
{
	return m_bScanning;
}

CScanItems CDCAntiVirusScheduledScanDlg::GetScanItems()
{
	CScanItems items;

	int nCount = m_listScanItems.GetItemCount();
	for(int i = 0; i < nCount; ++i)
	{
		items.push_back(m_listScanItems.GetItemText(i, 0));
	}

	return items;
}

void CDCAntiVirusScheduledScanDlg::ShowCurrentItem(LPCSTR sItem)
{
	GetDlgItem(IDD_EDIT_CUR_SC)->SetWindowText(sItem);

	m_progres.SetPos(m_progres.GetPos()+1);

	m_nCount++;
}

void CDCAntiVirusScheduledScanDlg::EnumerateFiles(CScanItems &files)
{
	OnMessage("Enumerating files....");

	m_progres.SetPos(0);
	CScanItems items = GetScanItems();
	typedef CScanItems::const_iterator CIt;
	CIt begin = items.begin();
	CIt end = items.end();

	CCountFiles counter(this, files);

	for(CIt it = begin; it != end; ++it)
	{
		counter.Execute((*it), "*.*", true);
	}

	m_progres.SetRange32(0, files.size());
}

void CDCAntiVirusScheduledScanDlg::OnFinish(LPCSTR sFinishText)
{
	m_tEnd = CTime::GetCurrentTime();
	CTimeSpan time = m_tEnd - m_tStart;

	m_progres.SetPos(0);
	GetDlgItem(IDD_STATIC_CURR_ACTION_SC)->SetWindowText(sFinishText);

	m_bScanning = false;
	GetDlgItem(IDD_EDIT_CUR_SC)->SetWindowText("");
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDD_STOP)->EnableWindow(FALSE);

	CDCAntivirusLogDlg dlg(this);
	dlg.SetFilesCount(m_nCount);
	dlg.SetInfectedItems(m_infItems);
	dlg.SetTime(time);
	dlg.DoModal();
}

void CDCAntiVirusScheduledScanDlg::OnVirus(LPCSTR sItem, LPCSTR sVirus)
{
	CInfectedItem item;
	item.m_sFile = sItem;
	item.m_sVirus = sVirus;

	m_infItems.push_back(item);
}

CString CDCAntiVirusScheduledScanDlg::GetExts()
{
	CString sExt;
	GetDlgItem(IDD_EDIT_TYPES_SC)->GetWindowText(sExt);

	if(sExt.IsEmpty())
	{
		return "*.*";
	}
	
	return sExt;
}

bool CDCAntiVirusScheduledScanDlg::GetUseInternalDB()
{
	if(((CButton *)GetDlgItem(IDD_USE_INTERNAL))->GetCheck())
	{
		return true;
	}	

	return false;
}

void CDCAntiVirusScheduledScanDlg::OnStop()
{
	m_bScanning = false;
}

void CDCAntiVirusScheduledScanDlg::FillScanItems()
{
	CString sItems = registry_utils::GetProfileString(sgSection, sgShedScanItems, "");
	if(!sItems.IsEmpty())
	{
		char *token;
		char *str = sItems.GetBuffer(0);
		token = strtok( str, "\n" );
		if(NULL != token)
		{
			m_listScanItems.InsertItem(m_listScanItems.GetItemCount(), token);
		}

		while(token != NULL)
		{
			token = strtok( NULL, "\n" );
			if(NULL != token)
			{
				m_listScanItems.InsertItem(m_listScanItems.GetItemCount(), token);
			}
		}
	}
}

void CDCAntiVirusScheduledScanDlg::FillItemTypes()
{
	CString sTypes = registry_utils::GetProfileString(sgSection, sgSchedFileTypes, "");	
	GetDlgItem(IDD_EDIT_TYPES_SC)->SetWindowText(sTypes);

	CString sUse = registry_utils::GetProfileString(sgSection, sgSchedInternalDB, "");
	BOOL bUse(FALSE);
	if(0 == sUse.CompareNoCase("1"))
	{
		bUse = TRUE;
	}

	((CButton *)GetDlgItem(IDD_USE_INTERNAL))->SetCheck(bUse);
}

void CDCAntiVirusScheduledScanDlg::OnCancel()
{
	if(m_bScanning)
	{
		return;
	}
	
	CDialog::OnCancel();
}

void CDCAntiVirusScheduledScanDlg::OnMessage(LPCSTR sMessage)
{
	GetDlgItem(IDD_STATIC_CURR_ACTION_SC)->SetWindowText(sMessage);
}