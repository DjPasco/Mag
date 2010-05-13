#include "stdafx.h"
#include "CLScanner.h"
#include "../ClamInclude/clamav.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

bool CCLScanner::Init()
{
	int nRet = cl_init(CL_INIT_DEFAULT);
	if(CL_SUCCESS != nRet)
	{
		return false;
	}

	return true;
}

bool CCLScanner::CreateEngine()
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

bool CCLScanner::FreeEngine()
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

	cl_cvdfree(m_pDBInfo);

	return true;
}

bool CCLScanner::LoadDatabase(LPCSTR sDBPath)
{
	m_pDBInfo = cl_cvdhead(sDBPath);
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

bool CCLScanner::ScanFile(LPCSTR sFile, const char *sVirname)
{
	unsigned long lScanned;
	int nRet = cl_scanfile(sFile, &sVirname, &lScanned, m_pEngine, CL_SCAN_STDOPT);

	if(CL_VIRUS == nRet)
	{
		return true;
	}

	return false;
}

void CCLScanner::GetInfo(CDBInfo *pInfo)
{
	if(NULL == m_pDBInfo || NULL == pInfo)
	{
		return;
	}

	pInfo->m_sTime		= m_pDBInfo->time;
	pInfo->m_nVersion	= m_pDBInfo->version;
	pInfo->m_nSigs		= m_pDBInfo->sigs;
}