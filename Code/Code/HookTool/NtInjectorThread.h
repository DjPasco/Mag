#if !defined(_NTINJECTORTHREAD_H_)
#define _NTINJECTORTHREAD_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NtProcessMonitor.h"

class CRemThreadInjector;

class CNtInjectorThread: public CNtProcessMonitor  
{
public:
	CNtInjectorThread(CRemThreadInjector* pInjector);
	virtual ~CNtInjectorThread();
private:
	virtual void OnCreateProcess(DWORD dwProcessId);
	virtual void OnTerminateProcess(DWORD dwProcessId);
	CRemThreadInjector* m_pInjector;
};

#endif
