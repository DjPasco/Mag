#ifndef _TRAY_DIALOG_H__
#define _TRAY_DIALOG_H__
#pragma once

#define WM_TRAY_ICON_NOTIFY_MESSAGE (WM_USER + 100)

class CTrayDialog : public CDialog
{

private:
	BOOL m_bMinimizeToTray;

	BOOL			m_bTrayIconVisible;
	NOTIFYICONDATA	m_nidIconData;
	CMenu			m_mnuTrayMenu;
	UINT			m_nDefaultMenuItem;
	// Construction
public:
	void TraySetMinimizeToTray(BOOL bMinimizeToTray = TRUE);
	BOOL TraySetMenu(UINT nResourceID,UINT nDefaultPos=0);	
	BOOL TraySetMenu(HMENU hMenu,UINT nDefaultPos=0);	
	BOOL TraySetMenu(LPCTSTR lpszMenuName,UINT nDefaultPos=0);	
	BOOL TrayUpdate();
	BOOL TrayShow();
	BOOL TrayHide();
	void TraySetToolTip(LPCTSTR lpszToolTip);
	void TraySetIcon(HICON hIcon);
	void TraySetIcon(UINT nResourceID);
	void TraySetIcon(LPCTSTR lpszResourceName);

	BOOL TrayIsVisible();
	CTrayDialog(UINT uIDD,CWnd* pParent = NULL);   // standard constructor
	
	virtual void OnTrayLButtonDown(CPoint pt);
	virtual void OnTrayLButtonDblClk(CPoint pt);
	
	virtual void OnTrayRButtonDown(CPoint pt);
	virtual void OnTrayRButtonDblClk(CPoint pt);

	virtual void OnTrayMouseMove(CPoint pt);
	

// Implementation
protected:
	int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void OnDestroy();
	void OnSysCommand(UINT nID, LPARAM lParam);	

private:
	LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);

protected:
	DECLARE_MESSAGE_MAP()
};

#endif
