#if !defined(_NTPROCESSMONITOR_H_)
#define _NTPROCESSMONITOR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Common\LockMgr.h"

// Structure for Process callback information
typedef struct _CallbackInfo
{
    HANDLE  ParentId;
    HANDLE  ProcessId;
    BOOLEAN bCreate;
} CALLBACK_INFO, *PCALLBACK_INFO;

class CNtDriverController;

class CNtProcessMonitor  
{
public:
	CNtProcessMonitor();
	virtual ~CNtProcessMonitor();
	//
	// Activate / Stop the thread which gets the notification from the 
	// device driver
	//
	void SetActive(BOOL bVal);
	HANDLE Get_ShutdownEvent() const;
	HANDLE Get_ProcessEvent() const;
	//
	// Retrieves data from the driver after received notification 
	//
	void RetrieveProcessInfo(
		CALLBACK_INFO& callbackInfo,
		CALLBACK_INFO& callbackTemp
		);
protected:
	virtual void OnCreateProcess(DWORD dwProcessId) = 0;
	virtual void OnTerminateProcess(DWORD dwProcessId) = 0;
private:
	BOOL Get_ThreadActive();
	void Set_ThreadActive(BOOL val);
	static unsigned __stdcall ThreadFunc(void* pvParam);

	CNtDriverController* m_pDriverCtl;
	CCSWrapper           m_CritSec;
	HANDLE               m_hShutdownEvent;
	HANDLE               m_hProcessEvent; 
	BOOL                 m_bThreadActive;
	DWORD                m_dwThreadId;
	HANDLE               m_hDriver;
};

#endif
