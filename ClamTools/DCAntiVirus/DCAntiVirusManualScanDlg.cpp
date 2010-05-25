#include "stdafx.h"
#include "resource.h"
#include "DCAntiVirusManualScanDlg.h"
#include "EnumerateFiles.h"

#include "../Utils/SendObj.h"
#include "../Utils/Registry.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

class CScanFiles : public CEnumerateFiles
{
public:
	CScanFiles(HWND hwnd, CDCAntiVirusManualScanDlg *pDlg, CScanEndingObs *pObs, bool bUseInternalDB)
		:CEnumerateFiles(pObs, false), m_hwnd(hwnd), m_pDlg(pDlg), m_bUseInternalDB(bUseInternalDB) {};

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
	CDCAntiVirusManualScanDlg *m_pDlg;
	bool m_bUseInternalDB;
};

class CCountFiles : public CEnumerateFiles
{
public:
	CCountFiles(CScanEndingObs *pObs): CEnumerateFiles(pObs, true),
		           m_nCount(0){};

public:
	virtual void OnFile(LPCTSTR lpzFile)
	{
		m_nCount++;
	}

public:
	int m_nCount;
};

UINT Scan(LPVOID pParam)
{
	if(NULL != pParam)
	{
		CDCAntiVirusManualScanDlg *pDlg = (CDCAntiVirusManualScanDlg *)pParam;
		
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
			CScanFiles scanner(hwnd, pDlg, pDlg, bUseInternalDB);
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

CDCAntiVirusManualScanDlg::CDCAntiVirusManualScanDlg(CWnd* pParent)
	: CDialog(IDD_DIALOG_SCAN, pParent),
	  m_bScanning(false)
{
//
}

CDCAntiVirusManualScanDlg::~CDCAntiVirusManualScanDlg()
{
//
}

BEGIN_MESSAGE_MAP(CDCAntiVirusManualScanDlg, CDialog)
	
	ON_BN_CLICKED(IDC_BUTTON_ADD_ADD,		OnAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_SCAN,	OnRemove)
	ON_BN_CLICKED(IDC_BUTTON_SCAN,			OnScan)
END_MESSAGE_MAP()

BOOL CDCAntiVirusManualScanDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(!m_listScanItems.SubclassDlgItem(IDC_LIST_ITEMS_SCAN, this))
	{
		return FALSE;
	}

	m_listScanItems.ModifyStyle(0, LVS_REPORT|LVS_SINGLESEL); 
	m_listScanItems.InsertColumn(0, "Items", LVCFMT_LEFT, 350);

	if(!m_listInfected.SubclassDlgItem(IDC_LIST_ITEMS_INFECTED, this))
	{
		return FALSE;
	}

	m_listInfected.ModifyStyle(0, LVS_REPORT|LVS_SINGLESEL); 
	m_listInfected.InsertColumn(0, "Item status", LVCFMT_LEFT, 150);
	m_listInfected.InsertColumn(1, "Item path", LVCFMT_LEFT, 350);

	if(!m_progres.SubclassDlgItem(IDC_PROGRESS1, this))
	{
		return FALSE;
	}	

	EnableProgresItems(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}
void CDCAntiVirusManualScanDlg::OnAdd()
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
			int nCount = m_listScanItems.GetItemCount();
			m_listScanItems.InsertItem(nCount, sPathName);
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

void CDCAntiVirusManualScanDlg::OnRemove()
{
	int nSel = m_listScanItems.GetSelectionMark();
	if(-1 != nSel)
	{
		m_listScanItems.DeleteItem(nSel);
	}
}

void CDCAntiVirusManualScanDlg::EnableProgresItems(BOOL bEnable)
{
	GetDlgItem(IDC_STATIC_PROGRES)->EnableWindow(bEnable);
	GetDlgItem(IDC_PROGRESS1)->EnableWindow(bEnable);

	GetDlgItem(IDC_STATIC_CURRENT)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT1_CUR)->EnableWindow(bEnable);

	GetDlgItem(IDC_STATIC_ACTION)->EnableWindow(bEnable);
}

void CDCAntiVirusManualScanDlg::EnableStartItems(BOOL bEnable)
{
	GetDlgItem(IDC_STATIC_SCAN)->EnableWindow(bEnable);
	GetDlgItem(IDC_LIST_ITEMS_SCAN)->EnableWindow(bEnable);

	GetDlgItem(IDC_BUTTON_ADD_ADD)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_REMOVE_SCAN)->EnableWindow(bEnable);

	GetDlgItem(IDOK)->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_INTERNAL_DB)->EnableWindow(bEnable);

	GetDlgItem(IDC_STATIC)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_TYPES_TO_SCAN)->EnableWindow(bEnable);
}

void CDCAntiVirusManualScanDlg::OnScan()
{
	if(!m_bScanning)
	{
		m_listInfected.DeleteAllItems();
		EnableStartItems(FALSE);
		EnableProgresItems(TRUE);
		GetDlgItem(IDC_BUTTON_SCAN)->SetWindowText("Stop");

		m_bScanning = true;

		AfxBeginThread(Scan, (LPVOID)this, THREAD_PRIORITY_LOWEST);
	}
	else
	{
		m_bScanning = false;

		EnableStartItems(TRUE);
		EnableProgresItems(FALSE);
		GetDlgItem(IDC_BUTTON_SCAN)->SetWindowText("Start");

		GetDlgItem(IDC_STATIC_ACTION)->SetWindowText("Scan stoped by user.");

		m_progres.SetPos(0);

		GetDlgItem(IDC_EDIT1_CUR)->SetWindowText("");
	}
}

bool CDCAntiVirusManualScanDlg::Continue()
{
	return m_bScanning;
}

CScanItems CDCAntiVirusManualScanDlg::GetScanItems()
{
	CScanItems items;

	int nCount = m_listScanItems.GetItemCount();
	for(int i = 0; i < nCount; ++i)
	{
		items.push_back(m_listScanItems.GetItemText(i, 0));
	}

	return items;
}

void CDCAntiVirusManualScanDlg::ShowCurrentItem(LPCSTR sItem)
{
	GetDlgItem(IDC_EDIT1_CUR)->SetWindowText(sItem);

	m_progres.SetPos(m_progres.GetPos()+1);
}

void CDCAntiVirusManualScanDlg::EnumerateFiles()
{
	GetDlgItem(IDC_STATIC_ACTION)->SetWindowText("Calculating files....");

	m_progres.SetPos(0);
	CScanItems items = GetScanItems();
	typedef CScanItems::const_iterator CIt;
	CIt begin = items.begin();
	CIt end = items.end();

	CCountFiles counter(this);

	for(CIt it = begin; it != end; ++it)
	{
		counter.Execute((*it), GetExts(), true);
	}

	m_progres.SetRange32(0, counter.m_nCount);

	GetDlgItem(IDC_STATIC_ACTION)->SetWindowText("Scanning....");
}

void CDCAntiVirusManualScanDlg::OnFinish()
{
	m_progres.SetPos(0);
	GetDlgItem(IDC_STATIC_ACTION)->SetWindowText("Scan completed.");

	m_bScanning = false;

	EnableStartItems(TRUE);
	EnableProgresItems(FALSE);
	GetDlgItem(IDC_BUTTON_SCAN)->SetWindowText("Start");

	GetDlgItem(IDC_EDIT1_CUR)->SetWindowText("");
}

void CDCAntiVirusManualScanDlg::OnVirus(LPCSTR sItem, LPCSTR sVirus)
{
	int nIns = m_listInfected.InsertItem(0, sVirus);
	m_listInfected.SetItemText(nIns, 1, sItem);
}

CString CDCAntiVirusManualScanDlg::GetExts()
{
	CString sExt;
	GetDlgItem(IDC_EDIT_TYPES_TO_SCAN)->GetWindowText(sExt);

	if(sExt.IsEmpty())
	{
		return "*.*";
	}
	
	return sExt;
}

void CDCAntiVirusManualScanDlg::OnOK(LPCSTR sItem, LPCSTR sOK)
{
	int nIns = m_listInfected.InsertItem(0, sOK);
	m_listInfected.SetItemText(nIns, 1, sItem);
}

bool CDCAntiVirusManualScanDlg::GetUseInternalDB()
{
	if(((CButton *)GetDlgItem(IDC_CHECK_INTERNAL_DB))->GetCheck())
	{
		return true;
	}	

	return false;
}

void CDCAntiVirusManualScanDlg::OnCancel()
{
	if(m_bScanning)
	{
		return;
	}
	
	CDialog::OnCancel();
}
