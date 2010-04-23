#ifdef DCSANNER_EXPORTS
	#define DCSANNER_API __declspec(dllexport)
#else
	#define DCSANNER_API __declspec(dllimport)
#endif

class DCSANNER_API CDCScanner
{
public:
	static bool Init();
	static bool CreateEngine();
	static bool FreeEngine();
	static bool LoadDatabases();
	static bool ScanFile(LPCSTR sFile, const char *sVirname);
};