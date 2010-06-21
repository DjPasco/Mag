#ifdef SYSTEM_HOOK_EXPORTS
	#define SYSTEM_HOOK_API __declspec(dllexport)
#else
	#define SYSTEM_HOOK_API __declspec(dllimport)
#endif

//Hook DLL must have one exported function.
SYSTEM_HOOK_API void DoMagic();
