#if !defined(_LIMITSINGLEINSTANCE_H_)
#define _LIMITSINGLEINSTANCE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLimitSingleInstance
{
protected:
	DWORD  m_dwLastError;
	HANDLE m_hMutex;

public:
	CLimitSingleInstance(char* pszMutexName);
	~CLimitSingleInstance(); 
	BOOL IsAnotherInstanceRunning();
};

#endif