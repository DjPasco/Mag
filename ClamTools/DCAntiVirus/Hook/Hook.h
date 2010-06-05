#ifndef _HOOK_H__
#define _HOOK_H__
#pragma once

namespace hook_utils
{
	void StartExeWithHookDll(LPCSTR sRunExe);
	void GlobalHook(bool bInitial = false);
	void GlobalUnHook();
	int GetProcessCount();
	bool ExistsModule(DWORD dwProcID, char *sDLLPath);
}

#endif
