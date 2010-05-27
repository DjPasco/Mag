#include "stdafx.h"
#include "Resource.h"
#include "DCAntivirusLogDlg.h"

#include "../Utils/Registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CDCAntivirusLogDlg, CDialog)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_INFECTED, &CDCAntivirusLogDlg::OnRClickListInfected)
	ON_COMMAND(ID_MENU_DELETEFILE, &CDCAntivirusLogDlg::OnMenuDeletefile)
	ON_COMMAND(ID_MENU_QUARANTINEFILE, &CDCAntivirusLogDlg::OnMenuQuarantinefile)
END_MESSAGE_MAP()

CDCAntivirusLogDlg::CDCAntivirusLogDlg(CWnd *pParent)
:CDialog(IDD_DIALOG_LOG, pParent)
{
	//	
}

CDCAntivirusLogDlg::~CDCAntivirusLogDlg()
{
	//
}

BOOL CDCAntivirusLogDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(!m_listLog.SubclassDlgItem(IDC_LIST_INFECTED, this))
	{
		return FALSE;
	}

	m_listLog.ModifyStyle(0, LVS_REPORT);
	m_listLog.SetExtendedStyle(LVS_EX_FULLROWSELECT 
                         | LVS_EX_GRIDLINES 
                         | LVS_EX_LABELTIP);


	m_listLog.InsertColumn(0, "Virus name", LVCFMT_LEFT, 80);
	m_listLog.InsertColumn(1, "File path", LVCFMT_LEFT, 400);

	FillLog();
	FillList();

	return TRUE;
}

void CDCAntivirusLogDlg::FillList()
{
	typedef CInfItems::const_iterator CIt;
	CIt begin = m_items.begin();
	CIt end = m_items.end();

	for(CIt it = begin; it != end; ++it)
	{
		int nIns = m_listLog.InsertItem(0, (*it).m_sVirus);
		m_listLog.SetItemText(nIns, 1, (*it).m_sFile);
	}
}

void CDCAntivirusLogDlg::RemoveFromList(std::vector<int> items)
{
	int nSize = items.size();

	for(int i = nSize-1; i>=0; --i)
	{
		m_listLog.DeleteItem(items[i]);
	}
}

void CDCAntivirusLogDlg::FillLog()
{
	CString sLog;
	sLog.Format(" Scanned files: %d\n Infected files: %d\n Scan time: %d:%d:%d", m_nFilesCount, m_items.size(), m_time.GetHours(), m_time.GetMinutes(), m_time.GetSeconds());
	GetDlgItem(IDC_STATIC_INFO)->SetWindowText(sLog);
}

void CDCAntivirusLogDlg::OnRClickListInfected(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<NMITEMACTIVATE *>(pNMHDR);
	
	CMenu Menu;
    if(Menu.LoadMenu(IDR_LIST_CONTEXT))
    {
        CMenu* pPopup = Menu.GetSubMenu(0);
		
		CPoint point;
		GetCursorPos(&point); 
        
		pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, this);
    }

	*pResult = 0;
}

void CDCAntivirusLogDlg::OnMenuDeletefile()
{
	POSITION pos = m_listLog.GetFirstSelectedItemPosition();
	if(pos != NULL)
	{
		std::vector<int> items;
		while (pos)
		{
			int nItem = m_listLog.GetNextSelectedItem(pos);
			items.push_back(nItem);
			CString sPath = m_listLog.GetItemText(nItem, 1);
			DeleteFile(sPath);
		}

		RemoveFromList(items);
	}
}

void CDCAntivirusLogDlg::OnMenuQuarantinefile()
{
	POSITION pos = m_listLog.GetFirstSelectedItemPosition();
	if(pos != NULL)
	{
		std::vector<int> items;

		while (pos)
		{
			int nItem = m_listLog.GetNextSelectedItem(pos);
			items.push_back(nItem);
			CString sPath = m_listLog.GetItemText(nItem, 1);
			CString sQuarantineDir = path_utils::GetQuarantineDir();
			CreateDirectory(sQuarantineDir, NULL);

			CFile file(sPath, CFile::modeRead);
			CString sNewPath = sQuarantineDir + file.GetFileName();
			file.Close();

			CopyFile(sPath, sNewPath, FALSE);
			DeleteFile(sPath);
		}

		RemoveFromList(items);
	}
}
