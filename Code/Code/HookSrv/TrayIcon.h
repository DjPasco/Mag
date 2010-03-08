#ifndef _TRAYICON_H_
#define _TRAYICON_H_

class CTrayIcon : public CCmdTarget 
{
protected:
	DECLARE_DYNAMIC(CTrayIcon)
	NOTIFYICONDATA m_nidEnabled;			// struct for Shell_NotifyIcon args
	NOTIFYICONDATA m_nidDisabled;

public:
	CTrayIcon(UINT uEnabled, UINT uDisabled);
	~CTrayIcon();

	// Call this to receive tray notifications
	void SetNotificationWnd(CWnd* pNotifyWnd, UINT uCbMsg);
	//
	// SetIcon functions. To remove icon, call SetIcon(0)
	//
	BOOL SetIcon(UINT uID); // main variant you want to use
	BOOL SetIcon(HICON hicon, char* lpTip);
	BOOL SetIcon(LPCTSTR lpResName, char* lpTip)
	{ 
		return SetIcon(lpResName ? 
			AfxGetApp()->LoadIcon(lpResName) : NULL, lpTip); 
	}
	BOOL SetStandardIcon(LPCTSTR lpszIconName, char* lpTip)
	{ 
		return SetIcon(::LoadIcon(NULL, lpszIconName), lpTip); 
	}

	virtual LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);

	void EnableScan(bool bEnable);

protected:
	UINT m_uEnabledIcon;
	UINT m_uDisabledIcon;
};

#endif
