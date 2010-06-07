#include "stdafx.h"
#include "resource.h"
#include "DCAntiVirusMemoryScanDlg.h"

#include <tlhelp32.h>
#include "../Utils/Settings.h"
#include "ManualScanUtils.h"

#define WM_START_MEMORY_SCAN WM_USER+100

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CDCAntiVirusMemoryScanDlg::CDCAntiVirusMemoryScanDlg(CWnd* pParent)
	: CDialog(IDD_DIALOG_MEMORY_SCAN, pParent),
	m_bScanning(false)
{
//
}

CDCAntiVirusMemoryScanDlg::~CDCAntiVirusMemoryScanDlg()
{
//
}

BEGIN_MESSAGE_MAP(CDCAntiVirusMemoryScanDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_STOP_MEMORY, OnStopMemory)
	ON_MESSAGE(WM_START_MEMORY_SCAN,	  OnStartScan)
END_MESSAGE_MAP()

BOOL CDCAntiVirusMemoryScanDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(!m_progres.SubclassDlgItem(ID_PROGRES_MEMORY, this))
	{
		return FALSE;
	}

	PostMessage(WM_START_MEMORY_SCAN);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

LRESULT CDCAntiVirusMemoryScanDlg::OnStartScan(WPARAM wParam, LPARAM lParam)
{
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	m_bScanning = true;
	m_nCount = 0;

	m_tStart = CTime::GetCurrentTime();

	CSettingsInfo info;
	if(settings_utils::Load(info))
	{
		CScanItems files;
		EnumerateFiles(files);

		CScanOptions *pOpt = new CScanOptions;
		pOpt->m_Items = files;
		pOpt->m_pObs = this;
		pOpt->m_bUseInternal = info.m_bMemIntDB ? true : false;

		AfxBeginThread(manual_scan_utils::Scan, (LPVOID)pOpt, priority_utils::GetRealPriority(path_utils::GetPriority()));
	}
	else
	{
		OnMessage("No registry entry found!");
	}

	return 0;
}

void CDCAntiVirusMemoryScanDlg::ShowCurrentItem(LPCSTR sItem)
{
	GetDlgItem(ID_EDIT_CURRENT_MEMORY)->SetWindowText(sItem);

	m_progres.SetPos(m_progres.GetPos()+1);

	m_nCount++;
}

void CDCAntiVirusMemoryScanDlg::EnumerateFiles(CScanItems &files)
{
	OnMessage("Calculating processes...");

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
			if(!Continue())
			{
				return;
			}

			HMODULE ModDLLHandle = NULL;
			BYTE * BytDLLBaseAdress = 0;
			MODULEENTRY32 MOEModuleInformation = { 0 };
			MOEModuleInformation.dwSize = sizeof(MODULEENTRY32);

			HANDLE HanModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, entry.th32ProcessID);

			Module32First(HanModuleSnapshot, &MOEModuleInformation);

			do
			{
				if(files.end() == std::find(files.begin(), files.end(), MOEModuleInformation.szExePath))
				{
					files.push_back(MOEModuleInformation.szExePath);
				}
				
				if(!Continue())
				{
					CloseHandle(HanModuleSnapshot);
					return;
				}

			} while(Module32Next(HanModuleSnapshot, &MOEModuleInformation));

			CloseHandle(HanModuleSnapshot);

			if(!Continue())
			{
				return;
			}

		}
	}

	m_progres.SetRange32(0, files.size());
}

void CDCAntiVirusMemoryScanDlg::OnVirus(LPCSTR sItem, LPCSTR sVirus)
{
	CInfectedItem item;
	item.m_sFile = sItem;
	item.m_sVirus = sVirus;

	m_infItems.push_back(item);
}

void CDCAntiVirusMemoryScanDlg::OnFinish(LPCSTR sFinishText)
{
	m_tEnd = CTime::GetCurrentTime();
	CTimeSpan time = m_tEnd - m_tStart;

	m_bScanning = false;

	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP_MEMORY)->EnableWindow(FALSE);
	m_progres.SetPos(0);
	GetDlgItem(IDC_STATIC_ACTION2)->SetWindowText(sFinishText);
	GetDlgItem(ID_EDIT_CURRENT_MEMORY)->SetWindowText("");

	CDCAntivirusLogDlg dlg;
	dlg.SetFilesCount(m_nCount);
	dlg.SetInfectedItems(m_infItems);
	dlg.SetTime(time);
	dlg.DoModal();
}

void CDCAntiVirusMemoryScanDlg::OnCancel()
{
	if(m_bScanning)
	{
		return;
	}
	
	CDialog::OnCancel();
}

void CDCAntiVirusMemoryScanDlg::OnStopMemory()
{
	m_bScanning = false;
}

bool CDCAntiVirusMemoryScanDlg::Continue()
{
	return m_bScanning;
}

void CDCAntiVirusMemoryScanDlg::OnMessage(LPCSTR sMessage)
{
	GetDlgItem(IDC_STATIC_ACTION2)->SetWindowText(sMessage);
}

