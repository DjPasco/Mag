#include "stdafx.h"
#include "Resource.h"
#include "DCAntivirusHookDlg.h"
#include <tlhelp32.h>
#include "Hook/Hook.h"

#include "../Utils/Registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDCAntivirusHookDlg::CDCAntivirusHookDlg()
:CDialog(IDD_DIALOG_PROC, NULL)
{
	//	
}

CDCAntivirusHookDlg::~CDCAntivirusHookDlg()
{
	//
}

BOOL CDCAntivirusHookDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(!m_treeProc.SubclassDlgItem(IDC_PROCESS_TREE, this))
	{
		return FALSE;
	}

	FillProcessTree();

	return TRUE;
}

void CDCAntivirusHookDlg::FillProcessTree()
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if(INVALID_HANDLE_VALUE == snapshot)
	{
		return;
	}

	if(TRUE == Process32First(snapshot, &entry))
	{
		char sHookPath[MAX_PATH];
		path_utils::GetHookDllPath(sHookPath);

		char sFullDetoursPath[MAX_PATH];
		path_utils::GetDetourDllPath(sFullDetoursPath);

		while(TRUE == Process32Next(snapshot, &entry))
		{
			CString sProcItem;
			sProcItem.Format("%d | %s", entry.th32ProcessID, entry.szExeFile);
			HTREEITEM proc_item = m_treeProc.InsertItem(sProcItem);

			bool bSysHook(false), bDetHook(false);

			if(hook_utils::ExistsModule(entry.th32ProcessID, sHookPath))
			{
				m_treeProc.InsertItem("SystemHook.dll", proc_item);
				bSysHook = true;
			}

			if(hook_utils::ExistsModule(entry.th32ProcessID, sFullDetoursPath))
			{
				m_treeProc.InsertItem("detoured.dll", proc_item);
				bDetHook = true;
			}

			//Bold item if one of dll's are not found on process.
			if(!bSysHook || !bDetHook)
			{
				TV_ITEM tvi;
				tvi.mask = TVIF_STATE | TVIF_HANDLE;
				tvi.hItem = proc_item;
				tvi.state = TVIS_BOLD;
				tvi.stateMask = TVIS_BOLD;
				m_treeProc.SetItem(&tvi);
			}
		}
	}

	CloseHandle(snapshot);
}