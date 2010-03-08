#include "stdafx.h"
#include "HookSrv.h"
#include "ApplicationScope.h"

CApplicationScope* CApplicationScope::sm_pInstance = NULL;

CApplicationScope::CApplicationScope():
	m_hmodHookTool(NULL),
	m_pfnInstallHook(NULL)
{

}

CApplicationScope::~CApplicationScope()
{
	if (m_hmodHookTool)
		::FreeLibrary( m_hmodHookTool );
}

CApplicationScope::CApplicationScope(const CApplicationScope& rhs)
{

}

CApplicationScope& CApplicationScope::operator=(const CApplicationScope& rhs)
{
	if (this == &rhs) 
		return *this;

	return *this; // return reference to left-hand object
}


//---------------------------------------------------------------------------
// GetInstance
//
// Implements the "double-checking" locking pattern combined with 
// Scott Meyers single instance
// For more details see - 
// 1. "Modern C++ Design" by Andrei Alexandrescu - 6.9 Living in a 
//     Multithreaded World
// 2. "More Effective C++" by Scott Meyers - Item 26
//---------------------------------------------------------------------------
CApplicationScope& CApplicationScope::GetInstance()
{
	if (!sm_pInstance)
	{
		CLockMgr<CCSWrapper> guard(g_AppSingeltonLock, TRUE);
		if (!sm_pInstance)
		{
			static CApplicationScope instance;
			sm_pInstance = &instance;
		}
	} // if

	return *sm_pInstance;
}

void CApplicationScope::InstallHook(BOOL bActivate, HWND hwndServer)
{
	if (NULL == m_hmodHookTool)
	{
		m_hmodHookTool = ::LoadLibrary( "HookTool.Dll" );
		if (NULL != m_hmodHookTool)
		{
			m_pfnInstallHook = (PFN_INSTALLHOOK)::GetProcAddress(m_hmodHookTool, "InstallHook");
		}
	}

	if (m_pfnInstallHook)
	{
		m_pfnInstallHook(bActivate, hwndServer);
	}
}