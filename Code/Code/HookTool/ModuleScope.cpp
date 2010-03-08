#include "..\Common\Common.h"
#include "ModuleScope.h"
#include "..\Common\SysUtils.h"
#include "..\Common\IniFile.h"
#include "Injector.h"

CModuleScope* CModuleScope::sm_pInstance      = NULL;
CLogFile*     CModuleScope::sm_pLogFile       = NULL;
CApiHookMgr*  CModuleScope::sm_pHookMgr       = NULL;

CModuleScope::CModuleScope(
	HWND*  phwndServer,
	BOOL*  pbHookInstalled,
	HHOOK* pHook
	):
	m_phwndServer(phwndServer),
	m_bTraceEnabledInitialized(FALSE),
	m_bTraceEnabled(FALSE),
	m_bUseWindowsHookInitialized(FALSE),
	m_bUseWindowsHook(TRUE),
	m_pInjector(NULL),
	m_pbHookInstalled(pbHookInstalled),
	m_pWhenZero(NULL)
{
	//
	// Make sure we now where we are.
	//
	m_bIsThisServerProcess = (NULL == *phwndServer);
	// 
	// Instantiate the object that looks after DLL ref counting
	//
	m_pWhenZero = new CWhenZeroDword(m_bIsThisServerProcess);
	//
	// Get the name of the current process
	//
	GetProcessHostName(m_szProcessName);
	//
	// and its process id
	//
	m_dwProcessId = ::GetCurrentProcessId();
	//
	// Create instance of the log file manager
	//
	sm_pLogFile = new CLogFile(GetTraceEnabled());
	//
	// Instantiate the only one hook manager
	//
	sm_pHookMgr = new CApiHookMgr(this);

	//
	// Which kind of injection we would like to use?
	//
	if (IsWindows9x() || UseWindowsHook())
		m_pInjector = new CWinHookInjector(m_bIsThisServerProcess, pHook);
	else
		m_pInjector = new CRemThreadInjector(m_bIsThisServerProcess);
}

CModuleScope::~CModuleScope()
{
	delete m_pInjector;
	delete m_pWhenZero;
	delete sm_pHookMgr;
	delete sm_pLogFile;
}

CModuleScope::CModuleScope(const CModuleScope& rhs)
{
	
}

CModuleScope& CModuleScope::operator=(const CModuleScope& rhs)
{
	if (this == &rhs) 
		return *this;

	return *this; // return reference to left-hand object
}


CModuleScope* CModuleScope::GetInstance(
	HWND*  phwndServer,
	BOOL*  pbHookInstalled,
	HHOOK* pHook
	)
{
	if (!sm_pInstance)
	{
		CLockMgr<CCSWrapper> guard(g_ModuleSingeltonLock, TRUE);
		if (!sm_pInstance)
		{
			static CModuleScope instance(phwndServer, pbHookInstalled, pHook);
			sm_pInstance = &instance;

			char  szFileName[MAX_PATH];
			char  *pdest;
			::GetModuleFileName(
				ModuleFromAddress(CModuleScope::GetInstance), 
				szFileName, 
				MAX_PATH
				);
			pdest = &szFileName[strlen(szFileName) - 4];
			strcpy(pdest, ".log");
			sm_pLogFile->InitializeFileName(szFileName);
		}
	} // if

	return sm_pInstance;
}

BOOL CModuleScope::UseWindowsHook()
{
	if (!m_bUseWindowsHookInitialized)
	{
		char szIniFile[MAX_PATH];
		GetIniFile(szIniFile);
		CIniFile iniFile(szIniFile);
		m_bUseWindowsHook = iniFile.ReadBool(
			"Scope",
			"UseWindowsHook",
			TRUE
			);
		m_bUseWindowsHookInitialized = TRUE;
	}
	return m_bUseWindowsHook;
}

BOOL CModuleScope::ManageModuleEnlistment()
{
	BOOL bResult = FALSE;
	//
	// Check if it is the hook server we should allow mapping of the DLL into
	// its address space
	//
	if (FALSE == *m_pbHookInstalled)
	{
		LogMessage(	"------- Hook server loads HookTool library -------" );
		//
		// Set the flag, thus we will know that the server has been installed
		//
		*m_pbHookInstalled = TRUE;
		//
		// and return success error code
		//
		bResult = TRUE;
	}
	//
	// and any other process should be examined whether it should be
	// hooked up by the DLL
	//
	else
	{
		bResult = m_pInjector->IsProcessForHooking(m_szProcessName);

		if (bResult)
			InitializeHookManagement();
		//
		// DLL is about to be mapped
		//
		//
		// Notify the server process the DLL will be mapped
		//
//		::PostMessage(
//			*m_phwndServer, 
//			UWM_HOOKTOOL_DLL_LOADED, 
//			0, 
//			::GetCurrentProcessId()
//			);
	}

	return bResult;
}

void CModuleScope::ManageModuleDetachment()
{
	//
	// Check if the request comes from hooked up application
	//
	if ( !m_bIsThisServerProcess )
	{
		FinalizeHookManagement();
	}
	else
	{
		//
		// attempt to eject the dll
		//
		m_pInjector->EjectModuleFromAllProcesses(m_pWhenZero->GetZeroHandle());
		LogMessage(	"------- Hook server shuts down and unloads HookTool library -------");
	}

}


BOOL CModuleScope::InstallHookMethod(BOOL bActivate, HWND hWndServer)
{
	BOOL bResult;
	if (bActivate)
	{
		*m_phwndServer = hWndServer;
		bResult = m_pInjector->InjectModuleIntoAllProcesses();
	}
	else
	{
		m_pInjector->EjectModuleFromAllProcesses(m_pWhenZero->GetZeroHandle());
		*m_phwndServer = NULL;
		bResult = TRUE;
	}
	return bResult;
}

void CModuleScope::LogMessage(const char* pszBuffer)
{
	char    szPrintBuffer[MAX_PATH];
	sprintf(
		szPrintBuffer, 
		"%s(%u) - %s", 
		m_szProcessName, 
		m_dwProcessId, 
		pszBuffer
		);
	sm_pLogFile->DoLogMessage(szPrintBuffer);
}

char* CModuleScope::GetProcessName() const
{
	return const_cast<char*>(m_szProcessName);
}

DWORD CModuleScope::GetProcessId() const
{
	return m_dwProcessId;
}

void CModuleScope::GetIniFile(char* pszIniFile)
{
	char  *pdest;
	::GetModuleFileName(
		ModuleFromAddress(CModuleScope::GetInstance), 
		pszIniFile, 
		MAX_PATH
		);
	pdest = &pszIniFile[strlen(pszIniFile) - 4];
	strcpy(pdest, ".ini");
}

BOOL CModuleScope::GetTraceEnabled()
{
	if (!m_bTraceEnabledInitialized)
	{
		m_bTraceEnabled = FALSE;
		char szIniFile[MAX_PATH];
		GetIniFile(szIniFile);
		CIniFile iniFile(szIniFile);
		m_bTraceEnabled = iniFile.ReadBool(
			"Trace",
			"Enabled",
			FALSE
			);
		m_bTraceEnabledInitialized = TRUE;
	} // if
	return m_bTraceEnabled;
}

BOOL CModuleScope::HookImport(
	PCSTR pszCalleeModName, 
	PCSTR pszFuncName, 
	PROC  pfnHook
	)
{
	return sm_pHookMgr->HookImport(
		pszCalleeModName,
		pszFuncName,
		pfnHook
		);
}

BOOL CModuleScope::UnHookImport(
	PCSTR pszCalleeModName, 
	PCSTR pszFuncName
	)
{
	return sm_pHookMgr->UnHookImport(
		pszCalleeModName,
		pszFuncName
		);
}

void CModuleScope::InitializeHookManagement()
{
	//
	// Initially we must hook a few important functions
	//
	sm_pHookMgr->HookSystemFuncs();
	//
	// and now we can set-up some custom (demonstration) hooks
	//
//	sm_pHookMgr->HookImport("Kernel32.DLL", "CreateFileA",	(PROC)CModuleScope::MyCreateFileA);
	sm_pHookMgr->HookImport("Kernel32.DLL", "CreateFileW",	(PROC)CModuleScope::MyCreateFileW);
//	sm_pHookMgr->HookImport("Kernel32.DLL", "OpenFile",		(PROC)CModuleScope::MyOpenFile);

	LogMessage("The hook engine has been activated.");
}

void CModuleScope::FinalizeHookManagement()
{
	if (sm_pHookMgr->AreThereHookedFunctions())
		LogMessage("The hook engine has been deactivated.");
	sm_pHookMgr->UnHookAllFuncs();
}

HANDLE WINAPI CModuleScope::MyCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	//Check if file not for querry
	if(dwDesiredAccess != 0)
	{
		char fname[MAX_PATH];
		WideCharToMultiByte( CP_ACP, 0, lpFileName, -1, fname, MAX_PATH,NULL,NULL); 

		if(!socket_utils::CheckFile(fname))
		{
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
	}

	return ::CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
		dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HFILE WINAPI CModuleScope::MyOpenFile(LPCSTR lpFileName, LPOFSTRUCT lpReOpenBuff, UINT uStyle)
{
//	char szBuffer[MAX_PATH];
	//sprintf(szBuffer, "Atidarytas [%s] failas.", lpFileName);
//	sm_pInstance->LogMessage(szBuffer);
	if(!socket_utils::CheckFile(lpFileName))
	{
		SetLastError(ERROR_ACCESS_DENIED);
		return HFILE_ERROR;
	}

	return ::OpenFile(lpFileName, lpReOpenBuff, uStyle);
}

HANDLE WINAPI CModuleScope::MyCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	//Check if file not for querry
	if(dwDesiredAccess != 0)
	{
		if(!socket_utils::CheckFile(lpFileName))
		{
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
	}
	
	return ::CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes,
		dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}