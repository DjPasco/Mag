#ifdef SYSTEM_HOOK_EXPORTS
	#define SYSTEM_HOOK_API __declspec(dllexport)
#else
	#define SYSTEM_HOOK_API __declspec(dllimport)
#endif


SYSTEM_HOOK_API void DoMagic();
