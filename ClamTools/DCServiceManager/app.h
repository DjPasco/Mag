#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "ntserv.h"


class CMyService : public CNTService
{
public:
	CMyService();
	virtual void WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
	virtual void OnStop();
	virtual void OnPause();
	virtual void OnContinue();
	virtual void ShowHelp();
	virtual void OnUserDefinedRequest(DWORD dwControl);

private:
	void StopScanServer(LPCSTR sServerName);

protected:
	BOOL  m_bWantStop;
	BOOL  m_bPaused;
	DWORD m_dwBeepInternal;
};


class CApp : public CWinApp
{
public:
	CApp(){};

protected:
	virtual BOOL InitInstance();
};
