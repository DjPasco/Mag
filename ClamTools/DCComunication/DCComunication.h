#ifdef DCCOMUNICATION_EXPORTS
	#define DCCOMUNICATION_API __declspec(dllexport)
#else
	#define DCCOMUNICATION_API __declspec(dllimport)
#endif

class DCCOMUNICATION_API CDCClient
{
public:
	static bool Execute(LPCWSTR sFile);
};