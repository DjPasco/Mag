#include "..\Common\Common.h"
#include "..\Common\SysUtils.h"
#include "ModuleScope.h"

#pragma data_seg(".HKT")
// The hook handle
HHOOK sg_hGetMsgHook       = NULL;

// Indicates whether the hook has been installed
BOOL  sg_bHookInstalled    = FALSE;

// We get this from the application who calls SetWindowsHookEx()'s wrapper
HWND  sg_hwndServer        = NULL; 

#pragma data_seg()


static CModuleScope* g_pModuleScope = NULL;

BOOL WINAPI InstallHook(BOOL bActivate, HWND hWndServer);

LRESULT CALLBACK GetMsgProc(
	int code,       // hook code
	WPARAM wParam,  // removal option
	LPARAM lParam   // message
	);

BOOL APIENTRY DllMain( 
	HANDLE hModule, 
	DWORD  ul_reason_for_call, 
	LPVOID lpReserved
	)
{
	BOOL bResult = TRUE;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			// We disable thread notifications
			// Prevent the system from calling DllMain
			// when threads are created or destroyed.
			::DisableThreadLibraryCalls( (HINSTANCE)hModule );

			g_pModuleScope = CModuleScope::GetInstance(
				&sg_hwndServer, 
				&sg_bHookInstalled,
				&sg_hGetMsgHook
				);
			g_pModuleScope->ManageModuleEnlistment();
			break;
		}
	case DLL_PROCESS_DETACH:
		{
			//
			// The DLL is being unmapped from the process's address space.
			//
			g_pModuleScope->ManageModuleDetachment();
			break;
		}
	} // switch

	return TRUE;
}

BOOL WINAPI InstallHook(
	BOOL bActivate, 
	HWND hWndServer
	)
{
	return g_pModuleScope->InstallHookMethod(bActivate, hWndServer);
}

LRESULT CALLBACK GetMsgProc(
	int code,       // hook code
	WPARAM wParam,  // removal option
	LPARAM lParam   // message
	)
{
	return ::CallNextHookEx(sg_hGetMsgHook, code, wParam, lParam);
}