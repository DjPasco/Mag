#include "stdafx.h"
#include "resource.h"
#include "DCAntiVirusScheduledScanDlg.h"
#include "EnumerateFiles.h"

#include "../Utils/SendObj.h"
#include "../Utils/Registry.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

class CScanSchedFiles : public CEnumerateFiles
{
public:
	CScanSchedFiles(HWND hwnd, CDCAntiVirusScheduledScanDlg *pDlg, CScanEndingObs *pObs, bool bUseInternalDB)
		:CEnumerateFiles(pObs), m_hwnd(hwnd), m_pDlg(pDlg), m_bUseInternalDB(bUseInternalDB) {};

public:
	virtual void OnFile(LPCTSTR lpzFile)
	{
		m_pDlg->ShowCurrentItem(lpzFile);

		CSendObj obj;
		strcpy_s(obj.m_sPath, MAX_PATH, lpzFile);
		obj.m_nType = EManualScan;
		obj.m_bUseInternalDB = m_bUseInternalDB;

		COPYDATASTRUCT copy;
		copy.dwData = 1;
		copy.cbData = sizeof(obj);
		copy.lpData = &obj;

		LRESULT result = SendMessage(m_hwnd,
									 WM_COPYDATA,
									 0,
									 (LPARAM) (LPVOID) &copy);

		if(2 == result)
		{
			CString sVirus = registry_utils::GetProfileString(sgSection, sgVirusName, "");
			if(!sVirus.IsEmpty())
			{
				m_pDlg->OnVirus(lpzFile, sVirus);
				registry_utils::WriteProfileString(sgSection, sgVirusName, "");
			}
		}
		else
		{
			m_pDlg->OnOK(lpzFile, "OK");
		}
	}

private:
	HWND m_hwnd;
	CDCAntiVirusScheduledScanDlg *m_pDlg;
	bool m_bUseInternalDB;
};

class CCountSchedFiles : public CEnumerateFiles
{
public:
	CCountSchedFiles(CScanEndingObs *pObs): CEnumerateFiles(pObs),
		           m_nCount(0){};

public:
	virtual void OnFile(LPCTSTR lpzFile)
	{
		m_nCount++;
	}

public:
	int m_nCount;
};

UINT SchedScan(LPVOID pParam)
{
	if(NULL != pParam)
	{
		CDCAntiVirusScheduledScanDlg *pDlg = (CDCAntiVirusScheduledScanDlg *)pParam;
		
		pDlg->EnumerateFiles();

		if(!pDlg->Continue())
		{
			pDlg->OnFinish();
			return 0;
		}
		
		CScanItems items = pDlg->GetScanItems();
		typedef CScanItems::const_iterator CIt;
		CIt begin = items.begin();
		CIt end = items.end();

		HWND hwnd = NULL;
		hwnd = ::FindWindow(NULL, sgServerName);

		if(NULL != hwnd)
		{
			bool bUseInternalDB = pDlg->GetUseInternalDB();
			CScanSchedFiles scanner(hwnd, pDlg, pDlg, bUseInternalDB);
			CString sExt = pDlg->GetExts();
			for(CIt it = begin; it != end; ++it)
			{
				scanner.Execute((*it), sExt, true);

				if(!pDlg->Continue())
				{
					pDlg->OnFinish();
					return 0;
				}
			}
		}

		pDlg->OnFinish();
	}
	return 0;
}

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

	if(!m_listInfected.SubclassDlgItem(IDD_SCANNED_LIST_SC, this))
	{
		return FALSE;
	}

	m_listInfected.ModifyStyle(0, LVS_REPORT|LVS_SINGLESEL); 
	m_listInfected.InsertColumn(0, "Item status", LVCFMT_LEFT, 150);
	m_listInfected.InsertColumn(1, "Item path", LVCFMT_LEFT, 350);

	if(!m_progres.SubclassDlgItem(IDD_PROGRES_SC, this))
	{
		return FALSE;
	}	

	FillScanItems();
	FillItemTypes();

	//Starting scan
	m_listInfected.DeleteAllItems();
	EnableStartItems(FALSE);
	EnableProgresItems(TRUE);
	m_bScanning = true;
	AfxBeginThread(SchedScan, (LPVOID)this);

	return TRUE;  // return TRUE  unless you set the focus to a control
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
}

void CDCAntiVirusScheduledScanDlg::EnumerateFiles()
{
	GetDlgItem(IDD_STATIC_CURR_ACTION_SC)->SetWindowText("Calculating files....");

	m_progres.SetPos(0);
	CScanItems items = GetScanItems();
	typedef CScanItems::const_iterator CIt;
	CIt begin = items.begin();
	CIt end = items.end();

	CCountSchedFiles counter(this);

	for(CIt it = begin; it != end; ++it)
	{
		counter.Execute((*it), "*.*", true);
	}

	m_progres.SetRange32(0, counter.m_nCount);

	GetDlgItem(IDD_STATIC_CURR_ACTION_SC)->SetWindowText("Scanning....");
}

void CDCAntiVirusScheduledScanDlg::OnFinish()
{
	m_progres.SetPos(0);
	GetDlgItem(IDD_STATIC_CURR_ACTION_SC)->SetWindowText("Scan completed.");

	m_bScanning = false;
	GetDlgItem(IDD_EDIT_CUR_SC)->SetWindowText("");
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDD_STOP)->EnableWindow(FALSE);
}

void CDCAntiVirusScheduledScanDlg::OnVirus(LPCSTR sItem, LPCSTR sVirus)
{
	int nIns = m_listInfected.InsertItem(0, sVirus);
	m_listInfected.SetItemText(nIns, 1, sItem);
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

void CDCAntiVirusScheduledScanDlg::OnOK(LPCSTR sItem, LPCSTR sOK)
{
	int nIns = m_listInfected.InsertItem(0, sOK);
	m_listInfected.SetItemText(nIns, 1, sItem);
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

	EnableStartItems(TRUE);
	EnableProgresItems(FALSE);
	GetDlgItem(IDD_STATIC_CURR_ACTION_SC)->SetWindowText("Scan stoped by user.");
	m_progres.SetPos(0);
	GetDlgItem(IDD_EDIT_CUR_SC)->SetWindowText("");
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