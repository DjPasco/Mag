#pragma once

#ifndef _TRAY_DIALOG_H__
	#include "TrayDialog.h"
#endif

class CDCAntiVirusDlg : public CTrayDialog
{
public:
	CDCAntiVirusDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CDCAntiVirusDlg();

protected:
	LRESULT OnHookSystem(WPARAM wParam, LPARAM lParam);


	virtual BOOL OnInitDialog();
	void OnPaint();
	void OnTimer(UINT nIDEvent);
	HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	HICON m_hIcon;

	int m_nProcCount;
	UINT_PTR m_nTimer;
};
