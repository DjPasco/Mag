#include "stdafx.h"
#include "DetoursHookCenter.h"
#include "DetoursHookCenterDlg.h"
#include "HookUtils.h"
#include <iostream>

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

static const UINT WM_MY_MESSAGE = RegisterWindowMessage(TEXT("WM_MY_MESSAGE"));

UINT StartOnFiles(LPVOID pParam);

class CAboutDlg : public CDialog
{
public:
	CAboutDlg(): CDialog(IDD_ABOUTBOX){};
};

CDetoursHookCenterDlg::CDetoursHookCenterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DETOURSHOOKCENTER_DIALOG, pParent),
	  m_pFilesTask(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CDetoursHookCenterDlg::~CDetoursHookCenterDlg()
{
	//
}

void CDetoursHookCenterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDetoursHookCenterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_HOOK_NOTEPAD, OnHookNotepad)
	ON_REGISTERED_MESSAGE(WM_MY_MESSAGE, OnMyMessage)
END_MESSAGE_MAP()


BOOL CDetoursHookCenterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_Log.SubclassDlgItem(IDC_RICH_LOG, this);

 	return TRUE;
}

void CDetoursHookCenterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDetoursHookCenterDlg::OnPaint()
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
HCURSOR CDetoursHookCenterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDetoursHookCenterDlg::OnHookNotepad()
{
	m_pFilesTask = AfxBeginThread(StartOnFiles, (LPVOID)this);
	hook_utils::LoadNotepadWithHookDll(m_Log);
}

LRESULT CDetoursHookCenterDlg::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	m_Log.AddRichText("!!!!");
	return 0;
}

CString CDetoursHookCenterDlg::Readline(SOCKET *client)
{
	CString sMess;
	char buffer;
	int rVal;

	while(true)
	{
		rVal = recv(*(client), &buffer, 1, 0);
		if(rVal == SOCKET_ERROR)
		{
			return "";
			WSACleanup();
		}
		
		if(buffer != '\n')
		{
			sMess += buffer;
		}
		else
		{
			break;
		}
	}

	return sMess;
}

void CDetoursHookCenterDlg::OnFiles()
{
	WORD sockVersion;
	WSADATA wsaData;
	int rVal;

	sockVersion = MAKEWORD(2,2);
	WSAStartup(sockVersion, &wsaData);

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(s == INVALID_SOCKET)
	{
		closesocket(s);
		WSACleanup();
		return;
	}

	SOCKADDR_IN sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");

	rVal = bind(s, (LPSOCKADDR)&sin, sizeof(sin));
	if(rVal == SOCKET_ERROR)
	{
		closesocket(s);
		WSACleanup();
		return;
	}

	rVal = listen(s, 2);
	if(rVal == SOCKET_ERROR)
	{
		closesocket(s);
		WSACleanup();
		return;
	}

	bool b_Done(false);

	while (!b_Done)
	{
		SOCKET client;
		client = accept(s, NULL, NULL);

		if(client == INVALID_SOCKET)
		{
			closesocket(s);
			WSACleanup();
			return;
		}

		CString sMessage;
		sMessage = Readline(&client);
		m_Log.AddRichText(sMessage);
		
		closesocket(client);
	}

	closesocket(s);

	WSACleanup();
}

UINT StartOnFiles(LPVOID pParam)
{
	CDetoursHookCenterDlg *pDlg = (CDetoursHookCenterDlg*)pParam;
	pDlg->OnFiles();
	return 0;
}