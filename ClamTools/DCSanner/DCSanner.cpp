#include "stdafx.h"
#include "DCSanner.h"
#include "../ClamInclude/clamav.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

bool CDCScanner::Init()
{
	int nRet = cl_init(CL_INIT_DEFAULT);
	if(CL_SUCCESS != nRet)
	{
		return false;
	}

	return true;
}

bool CDCScanner::CreateEngine()
{
	if(NULL != m_pEngine)
	{
		return true;
	}

	m_pEngine = cl_engine_new();
	if(NULL == m_pEngine)
	{
		return false;
	}

	return true;
}

bool CDCScanner::FreeEngine()
{
	if(NULL == m_pEngine)
	{
		return true;
	}

	int nRet = cl_engine_free(m_pEngine);
	if(CL_SUCCESS != nRet)
	{
		return false;
	}

	return true;
}

bool CDCScanner::LoadDatabase(LPCSTR sDBPath)
{
	cl_cvd *dbInfo = cl_cvdhead(sDBPath);
	
	m_nDBVersion = dbInfo->version;
	cl_cvdfree(dbInfo);

	unsigned int nSignCount = 0;
	int nRet = cl_load(sDBPath, m_pEngine, &nSignCount, CL_DB_BYTECODE);

	if(CL_SUCCESS != nRet)
	{
		return false;
	}

	nRet = cl_engine_compile(m_pEngine);
	if(CL_SUCCESS != nRet)
	{
		return false;
	}

	return true;
}

bool CDCScanner::ScanFile(LPCSTR sFile, const char *sVirname)
{
	unsigned long lScanned;
	int nRet = cl_scanfile(sFile, &sVirname, &lScanned, m_pEngine, CL_SCAN_STDOPT);

	if(CL_VIRUS == nRet)
	{
		return true;
	}

	return false;
}

unsigned int CDCScanner::GetDBVersion()
{
	return m_nDBVersion;
}
