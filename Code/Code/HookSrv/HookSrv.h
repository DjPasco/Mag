#if !defined(_HOOKSRV_H_)
#define _HOOKSRV_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h" 
class CHookSrvApp : public CWinApp
{
public:
	CHookSrvApp(){};
	~CHookSrvApp(){}

public:
	virtual BOOL InitInstance();

public:
	void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

#endif
