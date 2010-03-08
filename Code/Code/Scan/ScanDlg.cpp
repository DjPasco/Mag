#include "stdafx.h"
#include "Scan.h"
#include "ScanDlg.h"
#include "MD5\md5wrapper.h"
#include <fstream>
#include <time.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static CRichEditCtrl g_Log;
bool g_bEnableScan = true;

bool g_bExit = false;
bool g_UserDefTypes = false;

static void AddRichText(CString &str);
static void SetLastPos();
static void EraseLog();
static void AddBoldText(CString &str);
static void AddText(CString &str);

static bool CheckFile(LPCSTR sFile);


static const char * const _strPrefix =	"{\\rtf1\\ansi\\deff0\\deftab720{\\fonttbl{\\f0\\fswiss MS Sans Serif;}}"
										"{\\colortbl\\red0\\green0\\blue0;}"
										"\\deflang1033\\pard\\plain\\f0\\fs8\\cf0{";
static const char * const _strPrefixBold = "\\b\n";
static const char * const _strPostfixBold = "\\b0\n";
static const char * const _strPostfix = "}}";

UINT OnScan(LPVOID pParam);

CScanDlg::CScanDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScanDlg::IDD, pParent),
	m_nCurrPos(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	AfxInitRichEdit();
}

BEGIN_MESSAGE_MAP(CScanDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_REGISTERED_MESSAGE(EXIT_SCAN, OnExitScan)
	ON_BN_CLICKED(IDC_SCAN_ONLY, OnScanOnly)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, OnClear)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
END_MESSAGE_MAP()

BOOL CScanDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	if(!g_Log.SubclassDlgItem(IDC_RICHEDIT, this))
	{
		return FALSE;
	}

	for(int i = 0; i < SIZE; ++i)
	{
		m_files[i] = "";
	}


	CString s = "/*******************/";
	AddText(s);
	s = "Scan started";
	AddText(s);
	s = "/*******************/\n";
	AddText(s);

	AddTypes();
	
	m_pScanTask = AfxBeginThread(OnScan, (LPVOID)this);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CScanDlg::OnExitScan()
{
	CDialog::OnOK();
}

void CScanDlg::OnPaint() 
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

void CScanDlg::Scan()
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
		DoAction(sMessage, &client);
		closesocket(client);
		if(g_bExit)
		{
			break;
		}
	}

	closesocket(s);

	WSACleanup();
	if(g_bExit)
	{
		::PostMessage(g_Log.GetParent()->GetSafeHwnd(), EXIT_SCAN, 0, 0);
	}
}

void CScanDlg::OnScanOnly()
{
	if(this->IsDlgButtonChecked(IDC_SCAN_ONLY))
	{
		this->GetDlgItem(IDC_EDIT_TYPE)->EnableWindow();
		this->GetDlgItem(IDC_LIST)->EnableWindow();
		this->GetDlgItem(IDC_NEW)->EnableWindow();
		this->GetDlgItem(IDC_REMOVE)->EnableWindow();
		g_UserDefTypes = true;
	}
	else
	{
		this->GetDlgItem(IDC_EDIT_TYPE)->EnableWindow(FALSE);
		this->GetDlgItem(IDC_LIST)->EnableWindow(FALSE);
		this->GetDlgItem(IDC_NEW)->EnableWindow(FALSE);
		this->GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
		g_UserDefTypes = false;
	}
}

HCURSOR CScanDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

UINT OnScan(LPVOID pParam)
{
	CScanDlg *pDlg = (CScanDlg*)pParam;
	pDlg->Scan();
	return 0;
}

CString CScanDlg::Readline(SOCKET *client)
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

void CScanDlg::DoAction(CString &sMessage, SOCKET *client)
{
	CString sText;

	if(sMessage == "SHOW")
	{
		this->ShowWindow(SW_SHOW);
		return;
	}

	if(sMessage == "HIDE")
	{
		this->ShowWindow(SW_HIDE);
		return;
	}

	if(sMessage == "ENABLE")
	{
		g_bEnableScan = true;
		sText = "Scan enabled.";
		AddText(sText);
		return;
	}

	if(sMessage == "DISABLE")
	{
		g_bEnableScan = false;
		sText = "Scan disabled.";
		AddText(sText);
		return;
	}

	if(sMessage == "EXIT")
	{
		g_bExit = true;
		return;
	}

	if(sMessage.Left(5) == "FILE ")
	{
		sMessage.Delete(0, 5);

		char szScanBuffer[MAX_PATH];
		int nSize;

		if(!g_bEnableScan)
		{
			sprintf(szScanBuffer, "OK\n");
			nSize = strlen(szScanBuffer);
			send(*(client), szScanBuffer, nSize, 0);
			return;
		}

		if(SkipScan(sMessage))
		{
			CButton* btn = (CButton*)this->GetDlgItem(IDC_SKIPP);
			if(btn != NULL && btn->GetCheck())
			{
				sText.Format("File: %s", sMessage);
				AddText(sText);
				sText.Format("Skipped");
				AddText(sText);
			}
			sprintf(szScanBuffer, "OK\n");
			nSize = strlen(szScanBuffer);
			send(*(client), szScanBuffer, nSize, 0);
			return;
		}

		sText.Format("Scaning file: %s", sMessage);
		AddText(sText);

		CButton* btn = (CButton*)this->GetDlgItem(IDC_TIME);
		if(btn != NULL && btn->GetCheck())
		{
			char tbuffer [9];
		   _strtime(tbuffer);
		   sText.Format("Started at: %s", tbuffer);
		   AddText(sText);
		}

		if(CheckFile(sMessage))
		{
			AddFile(sMessage);
			btn = (CButton*)this->GetDlgItem(IDC_TIME);
			if(btn != NULL && btn->GetCheck())
			{
				char tbuffer [9];
				_strtime(tbuffer);
				sText.Format("Ended at: %s", tbuffer);
				AddText(sText);
			}

			sText.Format("File is clean.");
			AddText(sText);

			sprintf(szScanBuffer, "OK\n");
			nSize = strlen(szScanBuffer);
		}
		else
		{
			sText.Format("FILE IS INFECTED!!!!!");
			AddText(sText);

			sprintf(szScanBuffer, "FOUND\n");
			nSize = strlen(szScanBuffer);
		}

		send(*(client), szScanBuffer, nSize, 0);
	}
}

void CScanDlg::AddFile(LPCSTR sFile)
{
	md5wrapper md5;
	std::string hash = md5.getHashFromFile(sFile);

	if(m_nCurrPos < SIZE)
	{
		m_files[m_nCurrPos++] = hash;
	}
	else
	{
		m_nCurrPos = 0;
		m_files[m_nCurrPos++] = hash;
	}

}

bool CScanDlg::CheckSelTypes(LPCSTR sFile)
{

	CListBox *list = (CListBox*)this->GetDlgItem(IDC_LIST);
	if(NULL == list)
	{
		return false;
	}

	CString s(sFile);

	for(int i = 0; i < list->GetCount(); ++i)
	{
		if(list->GetSel(i))
		{
			CString sType;
			list->GetText(i, sType);
			sType.MakeUpper();
			CString temp;
			temp = s.Right(sType.GetLength());
			temp.MakeUpper();
			if(sType == temp)
			{
				return true;
			}
		}
	}

	return false;
}

bool CScanDlg::SkipScan(LPCSTR sFile)
{
	CString s(sFile);
	bool bTypeFound(false);

	if (g_UserDefTypes)
	{
		if(CheckSelTypes(sFile))
		{
			bTypeFound = true;
		}
	}
	else
	{
		std::vector<CString>::const_iterator it_end = m_types.end();
		for(std::vector<CString>::const_iterator it = m_types.begin(); it != it_end; ++it)
		{
			CString type = (*it);
			type.MakeUpper();
			CString temp;
			temp = s.Right(type.GetLength());
			temp.MakeUpper();
			if(type == temp)
			{
				bTypeFound = true;
				break;
			}
		}
	}

	if(!bTypeFound)
	{
		return true;
	}

	md5wrapper md5;
	std::string hash = md5.getHashFromFile(sFile);
	bool bHashFound(false);
	for(int i = 0; i < SIZE; ++i)
	{
		if(m_files[i] == hash)
		{
			//bHashFound = true;
			break;
		}
	}

	return bHashFound;
}

void CScanDlg::AddTypes()
{
	m_types.push_back("exe");
	m_types.push_back("txt");
	m_types.push_back("doc");
	m_types.push_back("dll");
	m_types.push_back("avi");
	m_types.push_back("xcl");

	CListBox *list = (CListBox*)this->GetDlgItem(IDC_LIST);
	if(NULL == list)
	{
		return;
	}

	list->AddString("exe");
	list->AddString("txt");
	list->AddString("doc");
	list->AddString("dll");
	list->AddString("avi");
	list->AddString("xcl");
}

void CScanDlg::OnClear()
{
	EraseLog();
}

void CScanDlg::OnNew()
{
	CEdit *edit = (CEdit*)this->GetDlgItem(IDC_EDIT_TYPE);
	if(edit != NULL)
	{
		CString sText;
		edit->GetWindowText(sText);
		if(!sText.IsEmpty())
		{
			CListBox *list = (CListBox*)this->GetDlgItem(IDC_LIST);
			list->AddString(sText);

			edit->Clear();
		}
	}
}

void CScanDlg::OnRemove()
{
	CListBox *list = (CListBox*)this->GetDlgItem(IDC_LIST);
	if(NULL == list)
	{
		return;
	}

	for(int i = 0; i < list->GetCount(); ++i)
	{
		if(list->GetSel(i))
		{
			list->DeleteString(i);
		}
	}
}

void AddRichText(CString &str)
{
	AddText(str);
}

void SetLastPos()
{
	LONG lMax = g_Log.GetTextLength();
	g_Log.SetSel(lMax,lMax);
}

void AddBoldText(CString &str)
{
	str =_strPrefixBold + str + _strPostfixBold;
	str =_strPrefix +  str + _strPostfix;
	AddText(str);
}

void AddText(CString &str)
{
	CString s;
	s.Format("%s\n", str);
	g_Log.LockWindowUpdate();
	g_Log.HideSelection(TRUE,FALSE);
	SetLastPos();
	g_Log.ReplaceSel(s);
	g_Log.SetSel(0,0);
	g_Log.HideSelection(FALSE,FALSE);
	g_Log.LineScroll(2);
	g_Log.UnlockWindowUpdate();
	g_Log.UpdateWindow();
}

void EraseLog()
{
	LONG lMax = g_Log.GetTextLength();
	g_Log.SetSel(0, lMax);
	g_Log.ReplaceSel("");
}

static int get_a_line(SOCKET &socket, char *buf, size_t len)
{
	char *ptr;

	for(ptr = buf; ptr < &buf[len]; ptr++) 
	{
		/* FIXME: very inefficient to call recv so many times */
		if(recv(socket, ptr, sizeof(char), 0) <= 0)
			return 0;
		if(*ptr == '\n') {
			*++ptr = '\0';
			return 1;
		}
	}

	return 1;
}

static bool dsresult(SOCKET &socket, LPCSTR sFile)
{
	char buff[BUFSIZ];
	get_a_line(socket, buff, sizeof(buff));

	if(strstr(buff, "FOUND"))
	{
		return false;
	}
			
	return true;
}

bool CheckFile(LPCSTR sFile)
{
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR)
	{
		return false;
	}

	struct sockaddr_in saServer;

	// Create a listening socket
	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(NULL == ListenSocket)
	{
		return true;
	}

	// Set up the sockaddr structure
	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = inet_addr("127.0.0.1");
	saServer.sin_port = htons(3310);

	int nConnectReturn = connect(ListenSocket, (struct sockaddr *) &saServer, sizeof(struct sockaddr_in));

	bool bRet(true);

	if(0 == nConnectReturn)
	{
		char szScanBuffer[MAX_PATH];

		sprintf(szScanBuffer, "SCAN %s", sFile);
		int nSize = strlen(szScanBuffer);
		int nGetSize = send(ListenSocket, szScanBuffer, nSize, 0);
		if(nGetSize == nSize)
		{
			bRet = dsresult(ListenSocket, sFile);
		}	
	}
	else
	{
		CString sMessage = "Clamd daemon not found. File will be accepted like clean.";
		AddText(sMessage);
	}

	shutdown(ListenSocket, 0);

	closesocket(ListenSocket);

	WSACleanup();

	return bRet;
}

