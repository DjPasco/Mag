#pragma once

#include "resource.h"

class CDetoursHookCenterApp : public CWinApp
{
public:
	CDetoursHookCenterApp() { };

public:
	virtual BOOL InitInstance();
};

extern CDetoursHookCenterApp theApp;