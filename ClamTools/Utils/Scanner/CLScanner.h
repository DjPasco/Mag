#pragma once

struct cl_engine;
struct cl_cvd;

class CDBInfo
{
public:
    char *m_sTime;
    unsigned int m_nVersion;
    unsigned int m_nSigs;
};

class CCLScanner
{
public:
	CCLScanner();
	virtual ~CCLScanner() { };
	bool Init();
	bool CreateEngine();
	bool FreeEngine();
	bool LoadDatabase(LPCSTR sDBPath);
	bool ScanFile(LPCSTR sFile, const char *sVirname);

	void SetScanSettings(BOOL bDeep, BOOL bOffice, BOOL bArchives, BOOL bPDF, BOOL bHTML);

public:
	void GetInfo(CDBInfo *pInfo);

private:
	cl_engine *m_pEngine;
	cl_cvd *m_pDBInfo;

	unsigned int m_nScanOptions;
};