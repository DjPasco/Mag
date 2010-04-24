#ifdef DCSANNER_EXPORTS
	#define DCSANNER_API __declspec(dllexport)
#else
	#define DCSANNER_API __declspec(dllimport)
#endif

struct cl_engine;

class DCSANNER_API CDCScanner
{
public:
	CDCScanner(): m_pEngine(NULL),m_nDBVersion(0){ };
	virtual ~CDCScanner() { };
	bool Init();
	bool CreateEngine();
	bool FreeEngine();
	bool LoadDatabase(LPCSTR sDBPath);
	bool ScanFile(LPCSTR sFile, const char *sVirname);

public:
	unsigned int GetDBVersion();

private:
	cl_engine *m_pEngine;
	unsigned int m_nDBVersion;
};