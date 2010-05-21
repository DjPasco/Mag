#include "stdafx.h"
#include "resource.h"
#include "DCAntiVirusMemoryScanDlg.h"
#include "EnumerateFiles.h"
#include "Psapi.h"
#include <tlhelp32.h>

#include "../Utils/SendObj.h"
#include "../Utils/Registry.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

static bool OnFile(LPCTSTR lpzFile)
{
	HWND hwnd = NULL;
	hwnd = ::FindWindow(NULL, sgServerName);

	CSendObj obj;
	strcpy_s(obj.m_sPath, MAX_PATH, lpzFile);
	obj.m_nType = EManualScan;

	COPYDATASTRUCT copy;
	copy.dwData = 1;
	copy.cbData = sizeof(obj);
	copy.lpData = &obj;

	LRESULT result = SendMessage(hwnd,
								 WM_COPYDATA,
								 0,
								 (LPARAM) (LPVOID) &copy);

	if(2 == result)
	{
		return false;
	}

	return true;
};

UINT ScanMemory(LPVOID pParam)
{
	if(NULL != pParam)
	{
		CDCAntiVirusMemoryScanDlg *pDlg = (CDCAntiVirusMemoryScanDlg *)pParam;
		
		pDlg->EnumerateFiles();

		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if(INVALID_HANDLE_VALUE == snapshot)
		{
			return 0;
		}

		//int nCount = 0;

		if(TRUE == Process32First(snapshot, &entry))
		{
			while(TRUE == Process32Next(snapshot, &entry))
			{
				HMODULE ModDLLHandle = NULL;
				BYTE * BytDLLBaseAdress = 0;
				MODULEENTRY32 MOEModuleInformation = { 0 };
				MOEModuleInformation.dwSize = sizeof(MODULEENTRY32);

				HANDLE HanModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, entry.th32ProcessID);

				Module32First(HanModuleSnapshot, &MOEModuleInformation);

				do
				{
					CFileFind find;
					if(find.FindFile(MOEModuleInformation.szExePath))
					{
						pDlg->ShowCurrentItem(MOEModuleInformation.szExePath);

						if(OnFile(MOEModuleInformation.szExePath))
						{
							pDlg->OnOK(MOEModuleInformation.szExePath, "OK");
						}
						else
						{
							CString sVirus = registry_utils::GetProfileString(sgSection, sgVirusName, "");
							if(!sVirus.IsEmpty())
							{
								pDlg->OnVirus(MOEModuleInformation.szExePath, sVirus);
								registry_utils::WriteProfileString(sgSection, sgVirusName, "");
							}
						}
					}
				} while(Module32Next(HanModuleSnapshot, &MOEModuleInformation));

				CloseHandle(HanModuleSnapshot);
			}
		}

		pDlg->OnFinish();

	}
	return 0;
}

CDCAntiVirusMemoryScanDlg::CDCAntiVirusMemoryScanDlg(CWnd* pParent)
	: CDialog(IDD_DIALOG_MEMORY_SCAN, pParent)
{
//
}

CDCAntiVirusMemoryScanDlg::~CDCAntiVirusMemoryScanDlg()
{
//
}

BEGIN_MESSAGE_MAP(CDCAntiVirusMemoryScanDlg, CDialog)
	//
END_MESSAGE_MAP()

BOOL CDCAntiVirusMemoryScanDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(!m_listLog.SubclassDlgItem(ID_LOG_MEMORY, this))
	{
		return FALSE;
	}

	m_listLog.ModifyStyle(0, LVS_REPORT|LVS_SINGLESEL); 
	m_listLog.InsertColumn(0, "Item status", LVCFMT_LEFT, 150);
	m_listLog.InsertColumn(1, "Item path", LVCFMT_LEFT, 350);

	if(!m_progres.SubclassDlgItem(ID_PROGRES_MEMORY, this))
	{
		return FALSE;
	}

	AfxBeginThread(ScanMemory, (LPVOID)this);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDCAntiVirusMemoryScanDlg::ShowCurrentItem(LPCSTR sItem)
{
	GetDlgItem(ID_EDIT_CURRENT_MEMORY)->SetWindowText(sItem);

	m_progres.SetPos(m_progres.GetPos()+1);
}

void CDCAntiVirusMemoryScanDlg::EnumerateFiles()
{
	GetDlgItem(IDC_STATIC_ACTION2)->SetWindowText("Calculating processes...");

	m_progres.SetPos(0);

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if(INVALID_HANDLE_VALUE == snapshot)
	{
		return;
	}

	int nCount = 0;

	if(TRUE == Process32First(snapshot, &entry))
	{
		while(TRUE == Process32Next(snapshot, &entry))
		{
			HMODULE ModDLLHandle = NULL;
			BYTE * BytDLLBaseAdress = 0;
			MODULEENTRY32 MOEModuleInformation = { 0 };
			MOEModuleInformation.dwSize = sizeof(MODULEENTRY32);

			HANDLE HanModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, entry.th32ProcessID);

			Module32First(HanModuleSnapshot, &MOEModuleInformation);

			do
			{
				CFileFind find;
				if(find.FindFile(MOEModuleInformation.szExePath))
				{
					nCount++;
				}
			} while(Module32Next(HanModuleSnapshot, &MOEModuleInformation));

			CloseHandle(HanModuleSnapshot);
		}
	}

	m_progres.SetRange32(0, nCount);

	GetDlgItem(IDC_STATIC_ACTION2)->SetWindowText("Scanning....");
}

void CDCAntiVirusMemoryScanDlg::OnVirus(LPCSTR sItem, LPCSTR sVirus)
{
	int nCount = m_listLog.GetItemCount();
	int nIns = m_listLog.InsertItem(nCount, sVirus);
	m_listLog.SetItemText(nIns, 1, sItem);
}

void CDCAntiVirusMemoryScanDlg::OnOK(LPCSTR sItem, LPCSTR sOK)
{
	int nCount = m_listLog.GetItemCount();
	int nIns = m_listLog.InsertItem(nCount, sOK);
	m_listLog.SetItemText(nIns, 1, sItem);
}

void CDCAntiVirusMemoryScanDlg::OnFinish()
{
	m_progres.SetPos(0);
	GetDlgItem(IDC_STATIC_ACTION2)->SetWindowText("Scan completed.");
	GetDlgItem(ID_EDIT_CURRENT_MEMORY)->SetWindowText("");
}
