#include "stdafx.h"
#include "Resource.h"
#include "DCAntiVirusDlg.h"

#include "..\..\Detours\Utils\SendObj.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDCAntiVirusDlg::CDCAntiVirusDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DCANTIVIRUS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDCAntiVirusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDCAntiVirusDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
END_MESSAGE_MAP()

BOOL CDCAntiVirusDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDCAntiVirusDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CDCAntiVirusDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

LRESULT CDCAntiVirusDlg::OnCopyData(WPARAM wParam, LPARAM lParam)
{
	PCOPYDATASTRUCT copy = (PCOPYDATASTRUCT) lParam;
	CString sText = ((CSendObj *)(copy->lpData))->m_sPath;
	return 0;
}

