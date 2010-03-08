#include "stdafx.h"
#include "trayicon.h"
#include <afxpriv.h>		// for AfxLoadString
#include "resource.h"

IMPLEMENT_DYNAMIC(CTrayIcon, CCmdTarget)

CTrayIcon::CTrayIcon(UINT uEnabled, UINT uDisabled)
{
	// Initialize NOTIFYICONDATA
	memset(&m_nidEnabled, 0 , sizeof(m_nidEnabled));
	m_nidEnabled.cbSize = sizeof(m_nidEnabled);
	m_nidEnabled.uID = IDR_TRAYICON;	// never changes after construction

	// Use resource string as tip if there is one
	AfxLoadString(uEnabled, m_nidEnabled.szTip, sizeof(m_nidEnabled.szTip));

	m_uDisabledIcon = uDisabled;
	m_uEnabledIcon = uEnabled;
}

CTrayIcon::~CTrayIcon()
{
	SetIcon(0); // remove icon from system tray
}

void CTrayIcon::SetNotificationWnd(CWnd* pNotifyWnd, UINT uCbMsg)
{
	// If the following assert fails, you're probably
	// calling me before you created your window. Oops.
	ASSERT(pNotifyWnd==NULL || ::IsWindow(pNotifyWnd->GetSafeHwnd()));
	m_nidEnabled.hWnd = pNotifyWnd->GetSafeHwnd();

	ASSERT(uCbMsg==0 || uCbMsg>=WM_USER);
	m_nidEnabled.uCallbackMessage = uCbMsg;
}

BOOL CTrayIcon::SetIcon(UINT uID)
{ 
	HICON hicon=NULL;
	if (uID) 
	{
		AfxLoadString(uID, m_nidEnabled.szTip, sizeof(m_nidEnabled.szTip));
		hicon = AfxGetApp()->LoadIcon(uID);
	}
	return SetIcon(hicon, NULL);
}

BOOL CTrayIcon::SetIcon(HICON hicon, char* lpTip) 
{
	UINT msg;
	m_nidEnabled.uFlags = 0;

	// Set the icon
	if (hicon) 
	{
		// Add or replace icon in system tray
		msg = m_nidEnabled.hIcon ? NIM_MODIFY : NIM_ADD;
		m_nidEnabled.hIcon = hicon;
		m_nidEnabled.uFlags |= NIF_ICON;
	} 
	else 
	{ // remove icon from tray
		if (m_nidEnabled.hIcon==NULL)
			return TRUE;		// already deleted
		msg = NIM_DELETE;
	}

	// Use the tip, if any
	if (lpTip)
		strncpy(m_nidEnabled.szTip, lpTip, sizeof(m_nidEnabled.szTip));
	if (m_nidEnabled.szTip[0])
		m_nidEnabled.uFlags |= NIF_TIP;

	// Use callback if any
	if (m_nidEnabled.uCallbackMessage && m_nidEnabled.hWnd)
		m_nidEnabled.uFlags |= NIF_MESSAGE;

	// Do it
	BOOL bRet = Shell_NotifyIcon(msg, &m_nidEnabled);
	if (msg==NIM_DELETE || !bRet)
		m_nidEnabled.hIcon = NULL;	// failed
	return bRet;
}

LRESULT CTrayIcon::OnTrayNotification(WPARAM wID, LPARAM lEvent)
{
	if (wID!=m_nidEnabled.uID ||
		(lEvent!=WM_RBUTTONUP && lEvent!=WM_LBUTTONDBLCLK))
		return 0;

	// If there's a resource menu with the same ID as the icon, use it as 
	// the right-button popup menu. CTrayIcon will interprets the first
	// item in the menu as the default command for WM_LBUTTONDBLCLK
	// 
	CMenu menu;
	if (!menu.LoadMenu(m_nidEnabled.uID))
		return 0;
	CMenu* pSubMenu = menu.GetSubMenu(0);
	if (!pSubMenu) 
		return 0;

	if (lEvent == WM_RBUTTONUP) 
	{
		//
		// Display the menu at the current mouse location. There's a "bug"
		// (Microsoft calls it a feature) in Windows 95 that requires calling
		// SetForegroundWindow. To find out more, search for Q135788 in MSDN.
		//
		CPoint mouse;
		GetCursorPos(&mouse);
		::SetForegroundWindow(m_nidEnabled.hWnd);	
		::TrackPopupMenu(pSubMenu->m_hMenu, 0, mouse.x, mouse.y, 0,
			m_nidEnabled.hWnd, NULL);
	} 
	else  
	{
		// double click: execute first menu item
		::SendMessage(m_nidEnabled.hWnd, WM_COMMAND, pSubMenu->GetMenuItemID(0), 0);
	}
	return 1; // handled
}

void CTrayIcon::EnableScan(bool bEnable)
{
	CMenu menu;
	if (!menu.LoadMenu(m_nidEnabled.uID))
	{
		return;
	}
	
	CMenu* pSubMenu = menu.GetSubMenu(0);
	if (!pSubMenu) 
	{
		return;
	}

	if(bEnable)
	{
		SetIcon(m_uEnabledIcon);
	}
	else
	{
		SetIcon(m_uDisabledIcon);
	}
}
