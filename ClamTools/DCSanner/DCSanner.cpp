#include "stdafx.h"
#include "DCSanner.h"
#include "../ClamInclude/clamav.h"

cl_engine *gpEngine = NULL;

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
	if(NULL != gpEngine)
	{
		return true;
	}

	gpEngine = cl_engine_new();
	if(NULL == gpEngine)
	{
		return false;
	}

	return true;
}

bool CDCScanner::FreeEngine()
{
	if(NULL == gpEngine)
	{
		return true;
	}

	int nRet = cl_engine_free(gpEngine);
	if(CL_SUCCESS != nRet)
	{
		return false;
	}

	return true;
}

bool CDCScanner::LoadDatabases()
{
	LPCSTR sDailyPath = "c:\\MAG_REPO\\LibClamAV\\daily.cvd";

	unsigned int nSignCount = 0;
	int nRet = cl_load(sDailyPath, gpEngine, &nSignCount, CL_DB_BYTECODE);

	if(CL_SUCCESS != nRet)
	{
		return false;
	}

	nRet = cl_engine_compile(gpEngine);
	if(CL_SUCCESS != nRet)
	{
		return false;
	}

	return true;
}

bool CDCScanner::ScanFile(LPCSTR sFile, const char *sVirname)
{
	unsigned long lScanned;
	int nRet = cl_scanfile(sFile, &sVirname, &lScanned, gpEngine, CL_SCAN_STDOPT);

	if(CL_VIRUS == nRet)
	{
		return true;
	}

	return false;
}
