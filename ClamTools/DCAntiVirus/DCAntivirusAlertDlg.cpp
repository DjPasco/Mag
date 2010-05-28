#include "stdafx.h"
#include "Resource.h"
#include "DCAntivirusAlertDlg.h"
#include <Psapi.h>

#include "../Utils/Registry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CDCAntivirusAlertDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_DEL,		 OnDel)
	ON_BN_CLICKED(IDC_BUTTON_QUARANTINE, OnQuarantine)
END_MESSAGE_MAP()

CDCAntivirusAlertDlg::CDCAntivirusAlertDlg(LPCSTR sFilePath, LPCSTR sVirusName, DWORD PID)
:CDialog(IDD_DIALOG_ALERT, NULL),
m_sFilePath(sFilePath),
m_sVirusName(sVirusName),
m_PID(PID)
{
	//	
}

CDCAntivirusAlertDlg::~CDCAntivirusAlertDlg()
{
	//
}

BOOL CDCAntivirusAlertDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, m_PID);
 
	if(NULL != hProcess)
	{
		char sProcName[MAX_PATH];
		GetModuleFileNameEx(hProcess, NULL, sProcName, MAX_PATH);
		CString sInfo;
		sInfo.Format("File: %s\ninfected with virus: %s\nFile launched by %s\n\nAllow/Deny access or Delete/Quarantine file?", m_sFilePath, m_sVirusName, sProcName);

		GetDlgItem(IDC_STATIC)->SetWindowText(sInfo);
	}

	CloseHandle(hProcess);

	
	return TRUE;
}
void CDCAntivirusAlertDlg::OnDel()
{
	DeleteFile(m_sFilePath);
	CDialog::OnOK();
}

void CDCAntivirusAlertDlg::OnQuarantine()
{
	CString sQuarantineDir = path_utils::GetQuarantineDir();
	CreateDirectory(sQuarantineDir, NULL);

	CFile file(m_sFilePath, CFile::modeRead);
	CString sNewPath = sQuarantineDir + file.GetFileName();
	file.Close();

	CopyFile(m_sFilePath, sNewPath, FALSE);
	DeleteFile(m_sFilePath);

	CDialog::OnOK();
}
