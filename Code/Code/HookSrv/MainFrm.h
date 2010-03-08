#if !defined(_MAINFRM_H_)
#define _MAINFRM_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TrayIcon.h"
#include "ApplicationScope.h"

class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Implementation
public:
	virtual ~CMainFrame();

// Generated message map functions
protected:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnClose();
	void OnAppExit();
	LRESULT OnTrayNotification(WPARAM wp, LPARAM lp);
	DECLARE_MESSAGE_MAP()

	void SendMessageToServer(LPCSTR sMessage, u_short port);
	void VaitForClamd();

	void OnEnableScan();
	void OnShow();

private:
	CTrayIcon	m_TrayIcon;		// my tray icon
	BOOL		m_bShutdown;	// Determines whether the app terminates

	bool m_bHookEnabled;
	bool m_bShow;
	CApplicationScope& m_ApplicationScope;
	PROCESS_INFORMATION m_clamProcess;
};

#endif