#if !defined(AFX_SCAN_H__F8B126A2_9290_4A95_A827_268697130FE3__INCLUDED_)
#define AFX_SCAN_H__F8B126A2_9290_4A95_A827_268697130FE3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

class CScanApp : public CWinApp
{
public:
	CScanApp(){};

public:
	virtual BOOL InitInstance();
};

#endif
