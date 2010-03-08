#include "stdafx.h"
#include "HookSrv.h"
#include "LimitSingleInstance.h"

CLimitSingleInstance::CLimitSingleInstance(char* pszMutexName)
{
	// be sure to use a name that is unique for this application otherwise
	// two apps may think they are the same if they are using same name for
	// 3rd parm to CreateMutex
	m_hMutex = ::CreateMutex(NULL, FALSE, pszMutexName); //do early
	m_dwLastError = ::GetLastError(); //save for use later...
}
  
CLimitSingleInstance::~CLimitSingleInstance() 
{
	if (m_hMutex)  //don't forget to close handles...
	{
		::CloseHandle(m_hMutex); //do as late as possible
		m_hMutex = NULL;       //good habit to be in
	}
}

BOOL CLimitSingleInstance::IsAnotherInstanceRunning() 
{
	return (ERROR_ALREADY_EXISTS == m_dwLastError);
}
