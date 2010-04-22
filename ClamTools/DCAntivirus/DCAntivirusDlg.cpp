#include "stdafx.h"
#include "DCAntivirus.h"
#include "DCAntivirusDlg.h"
#include "..\..\Detours\Utils\SendObj.h"
#include "..\ClamInclude\clamav.h"
#include <hash_map>
#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CDCAntivirusDlg::CDCAntivirusDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDCAntivirusDlg::IDD, pParent),
	m_pEngine(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	cl_init(CL_INIT_DEFAULT);
	m_pEngine = cl_engine_new();
	LPCSTR sPath = "c:\\MAG_REPO\\LibClamAV\\daily.cvd";

	unsigned int nSignCount = 0;
	int nRet = cl_load(sPath, m_pEngine, &nSignCount, CL_DB_BYTECODE);
	nRet = cl_engine_compile(m_pEngine);

	std::hash_map<int, int> map;
	map;
}

CDCAntivirusDlg::~CDCAntivirusDlg()
{
	cl_engine_free(m_pEngine);
}

void CDCAntivirusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDCAntivirusDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_COPYDATA, OnCopyData)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CDCAntivirusDlg message handlers

BOOL CDCAntivirusDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDCAntivirusDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDCAntivirusDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CDCAntivirusDlg::OnCopyData(WPARAM wParam, LPARAM lParam)
{
	PCOPYDATASTRUCT copy = (PCOPYDATASTRUCT) lParam;
	CString sText = ((CSendObj *)(copy->lpData))->m_sPath;
	const char *sVirname;
	unsigned long nScanned;
	cl_scanfile(sText, &sVirname, &nScanned, m_pEngine, CL_SCAN_STDOPT);
	return 0;
}


