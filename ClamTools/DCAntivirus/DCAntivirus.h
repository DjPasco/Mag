#ifndef _DC_ANTI_VIRUS_H__
#define _DC_ANTI_VIRUS_H__
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

class CDCAntiVirusApp : public CWinApp
{
public:
	CDCAntiVirusApp() { };

public:
	virtual BOOL InitInstance();
};

#endif
