#if !defined(_APPLICATIONSCOPE_H_)
#define _APPLICATIONSCOPE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Common\LockMgr.h"

static CCSWrapper g_AppSingeltonLock;

typedef BOOL (WINAPI *PFN_INSTALLHOOK)(BOOL bActivate,HWND hWndServer);

class CApplicationScope  
{
private:
	//
	// Intentionally hide the defualt constructor,
	// copy constructor and assignment operator 
	//

	//
	// Default constructor
	//
	CApplicationScope();
	//
	// Copy constructor
	//
	CApplicationScope(const CApplicationScope& rhs);
	//
	// Assignment operator
	//
	CApplicationScope& operator=(const CApplicationScope& rhs);
public:
	//
	// Destructor - we must declare it as public in order to provide
	// enough visibility for the GetInstance().
	// However the destructor shouldn't be called directly by the 
	// application's code.
	//
	virtual ~CApplicationScope();
	//
	// Implements the "double-checking" locking pattern combined with 
	// Scott Meyers single instance
	// For more details see - 
	// 1. "Modern C++ Design" by Andrei Alexandrescu - 6.9 Living in a 
	//     Multithreaded World
	// 2. "More Effective C++" by Scott Meyers - Item 26
	//
	static CApplicationScope& GetInstance();
	//
	// Delegates the call to the DLL InstallHook function
	//
	void InstallHook(BOOL bActivate, HWND hwndServer);
	//
	// Enables / Disables on-access file scan
	//
	void EnableScan(BOOL bEnable);

private:
	//
	// Instance's pointer holder
	//
	static CApplicationScope* sm_pInstance;
	//
	// HookTool handle
	//
	HMODULE m_hmodHookTool;
	//
	//
	//
	PFN_INSTALLHOOK m_pfnInstallHook;
};

#endif