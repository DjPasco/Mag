#ifndef _HOOK_H__
#define _HOOK_H__
#pragma once

namespace hook_utils
{
	void StartExeWithHookDll(LPCSTR sRunExe);
	void GlobalHook();
	void GlobalUnHook();
}

#endif
