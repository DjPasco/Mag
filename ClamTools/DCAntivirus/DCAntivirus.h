#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

class CDCAntiVirusApp : public CWinApp
{
public:
	CDCAntiVirusApp(LPCTSTR lpszAppName = NULL);

public:
	virtual BOOL InitInstance();
};

extern CDCAntiVirusApp theApp;