#include "stdafx.h"
#include "HookSrv.h"
#include "MainFrm.h"

#pragma comment(lib, "Ws2_32.lib")

#define WM_MY_TRAY_NOTIFICATION WM_USER + 0x500

static const u_short scan_port = 8888;
static const u_short clam_port = 3310;

static void SocketError(LPCSTR str)
{
	MessageBox(NULL, str, "SCAN SERVER ERROR", MB_OK);
};

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_COMMAND(ID_ENABLE_SCAN, OnEnableScan)
	ON_COMMAND(ID_SHOW, OnShow)
	ON_MESSAGE(WM_MY_TRAY_NOTIFICATION, OnTrayNotification)
END_MESSAGE_MAP()

CMainFrame::CMainFrame():
	m_TrayIcon(IDI_TRAYICON, IDI_DISABLED),
	m_bShutdown(FALSE),
	m_ApplicationScope( CApplicationScope::GetInstance() ),
	m_bHookEnabled(true)
{
	ZeroMemory(&m_clamProcess, sizeof(m_clamProcess));
}

CMainFrame::~CMainFrame()
{
	//m_ApplicationScope.InstallHook( FALSE, 0);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set up tray icon
	m_TrayIcon.SetNotificationWnd(this, WM_MY_TRAY_NOTIFICATION);
	m_TrayIcon.SetIcon(IDI_TRAYICON);

	STARTUPINFO si;
	ZeroMemory(&si,sizeof(si));

	ShellExecute(this->GetSafeHwnd(), "open", "c:\\Program Files\\clamAV\\clamd.exe", "", "", SW_SHOW); 
	VaitForClamd();
	ShellExecute(this->GetSafeHwnd(), "open", "Scan.exe", "", "", SW_SHOW); 

	//m_ApplicationScope.InstallHook( TRUE, m_hWnd);

	return 0;
}

void CMainFrame::OnClose() 
{
	if (m_bShutdown)
	{
		CFrameWnd::OnClose();
	}
	else
	{
		ShowWindow(SW_HIDE);
	}
}

void CMainFrame::OnAppExit() 
{
	m_bShutdown = TRUE;	
	SendMessageToServer("EXIT", scan_port);
	SendMessageToServer("SHUTDOWN", clam_port);
	SendMessage(WM_CLOSE);	
}

void CMainFrame::OnEnableScan()
{
	if(m_bHookEnabled)
	{
		SendMessageToServer("DISABLE", scan_port);
		m_TrayIcon.EnableScan(false);
		m_bHookEnabled = false;
	}
	else
	{
		SendMessageToServer("ENABLE", scan_port);
		m_TrayIcon.EnableScan(true);
		m_bHookEnabled = true;
	}
}

void CMainFrame::OnShow()
{
	if(m_bShow)
	{
		SendMessageToServer("HIDE", scan_port);
		m_bShow = false;
	}
	else
	{
		SendMessageToServer("SHOW", scan_port);
		m_bShow = true;
	}
}

LRESULT CMainFrame::OnTrayNotification(WPARAM uID, LPARAM lEvent)
{
	// let tray icon do default stuff
	return m_TrayIcon.OnTrayNotification(uID, lEvent);
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

static bool dsresult_for_clam(SOCKET &socket)
{
	char buff[BUFSIZ];
	get_a_line(socket, buff, sizeof(buff));

	if(!strstr(buff, "PONG"))
	{
		return false;
	}
			
	return true;
}

void CMainFrame::VaitForClamd()
{
	Sleep(1000);
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR)
	{
		return;
	}

	struct sockaddr_in saServer;

	// Create a listening socket
	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(NULL == ListenSocket)
	{
		return;
	}

	// Set up the sockaddr structure
	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = inet_addr("127.0.0.1");
	saServer.sin_port = htons(3310);

	int k = 0;
	while(k!=10)
	{
		int nConnectReturn = connect(ListenSocket, (struct sockaddr *) &saServer, sizeof(struct sockaddr_in));

		if(0 == nConnectReturn)
		{
			char szScanBuffer[MAX_PATH];

			sprintf(szScanBuffer, "PING");
			int nSize = strlen(szScanBuffer);
			bool bRet(false);
			int nGetSize = send(ListenSocket, szScanBuffer, nSize, 0);
			if(nGetSize == nSize)
			{
				bRet = dsresult_for_clam(ListenSocket);
				break;
			}	
		}

		Sleep(1000);
		k++;
	}

	shutdown(ListenSocket, 0);

	closesocket(ListenSocket);

	WSACleanup();
}

void CMainFrame::SendMessageToServer(LPCSTR sMessage, u_short port)
{
	WORD sockVersion;
	WSADATA wsaData;

	sockVersion = MAKEWORD(1,1);
	//start dll
	WSAStartup(sockVersion, &wsaData);

	//create socket
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(s == INVALID_SOCKET)
	{
		WSACleanup();
		return;
	}

	SOCKADDR_IN sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");

	int nConnectReturn = connect(s, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));

	if(0 == nConnectReturn)
	{
		char szScanBuffer[MAX_PATH];

		sprintf(szScanBuffer, "%s\n", sMessage);
		int nSize = strlen(szScanBuffer);
		int nGetSize = send(s, szScanBuffer, nSize, 0);
	}

	closesocket(s);

	WSACleanup();
}


