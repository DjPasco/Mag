#pragma once

#include "resource.h"		// main symbols


// CDetoursHookCenterApp:
// See DetoursHookCenter.cpp for the implementation of this class
//

class CDetoursHookCenterApp : public CWinApp
{
public:
	CDetoursHookCenterApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDetoursHookCenterApp theApp;