// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FORCEVERSIONDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FORCEVERSIONDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SYSTEM_HOOK_EXPORTS
	#define SYSTEM_HOOK_API __declspec(dllexport)
#else
	#define SYSTEM_HOOK_API __declspec(dllimport)
#endif


SYSTEM_HOOK_API void DoNothing();

namespace utils
{
	void SetHwnd(HWND Hwnd);
}
