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
			HTREEITEM proc_item = m_treeProc.InsertItem(entry.szExeFile);

			if(hook_utils::ExistsModule(entry.th32ProcessID, sHookPath))
			{
				m_treeProc.InsertItem("SystemHook.dll", proc_item);
			}

			if(hook_utils::ExistsModule(entry.th32ProcessID, sFullDetoursPath))
			{
				m_treeProc.InsertItem("detoured.dll", proc_item);
			}

			m_treeProc.Expand(proc_item, TVE_EXPAND);
		}
	}

	CloseHandle(snapshot);
}