#include "stdafx.h"
#include "Scanner.h"

#include "CLScanner.h"
#include "ScanValidatorObs.h"
#include "PrecisionTimer.h"
#include "FileHashDBUtils.h"
#include "../TraySendObj.h"
#include "../Registry.h"
#include "../Settings.h"
#include "../Log.h"

#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define LOAD_MAIN_DB

CScanner::CScanner()
{
	m_pFilesMap = NULL;

	m_bLoaded = false;

	Init();
	
	m_pMD5 = EVP_md5();

	CSettingsInfo info;
	if(settings_utils::Load(info))
	{
		SetScanSettings(info.m_bDeep, info.m_bOffice, info.m_bArchives, info.m_bPDF, info.m_bHTML);
		SetFilesTypes(info.m_sFilesTypes);
	}
}

CScanner::~CScanner()
{
	Free();	
}

bool CScanner::LoadDatabases()
{
	m_bLoaded = false;

#ifdef LOAD_MAIN_DB
	if(!m_pMainScan->LoadDatabase(path_utils::GetMainDBPath()))
	{
		return false;
	}

	m_pMainScan->GetInfo(m_pMainDBInfo);
#endif

	if(!m_pDailyScan->LoadDatabase(path_utils::GetDailyDBPathCVD()))
	{
		if(!m_pDailyScan->LoadDatabase(path_utils::GetDailyDBPathCLD()))
		{
			return false;
		}
	}

	m_pDailyScan->GetInfo(m_pDailyDBInfo);

	m_bLoaded = true;

	return true;
}

void CScanner::Init()
{
	m_pMainScan		= new CCLScanner;
	m_pDailyScan	= new CCLScanner;

	m_pMainDBInfo	= new CDBInfo;
	m_pDailyDBInfo	= new CDBInfo;

#ifdef LOAD_MAIN_DB
	if(m_pMainScan->Init())
	{
		m_pMainScan->CreateEngine();
	}
#endif

	if(m_pDailyScan->Init())
	{
		m_pDailyScan->CreateEngine();
	}
}

void CScanner::Free()
{
	if(m_bLoaded)
	{
#ifdef LOAD_MAIN_DB
		m_pMainScan->FreeEngine();	
#endif
		m_pDailyScan->FreeEngine();
	}
	
	delete m_pMainScan;
	delete m_pMainDBInfo;

	delete m_pDailyScan;
	delete m_pDailyDBInfo;
}

bool CScanner::ScanFile(LPCSTR sFile, CString &sVirus, DWORD PID, bool &bScanned, bool bCheckType)
{
	if(NULL == m_pFilesMap)
	{
		return false;
	}

	if(bCheckType)
	{
		scan_log_utils::LogHeader("ScanFile (realtime scan)", PID);
	}
	else
	{
		scan_log_utils::LogHeader("ScanFile (manual/memory scan)", PID);
	}

	if(!m_bLoaded)
	{
		scan_log_utils::WriteLine("DB not loaded.");
		return true;
	}

	scan_log_utils::WriteLine(sFile);

	std::string sFilePath(sFile);
	std::transform(sFilePath.begin(), sFilePath.end(), sFilePath.begin(), toupper);

	if(bCheckType)
	{
		if(!file_hash_DB_utils::FileIsSupported(sFilePath.c_str(), m_types))
		{
			scan_log_utils::WriteLine("File not supported (check file extension settings).");
			return true;
		}
	}

	bScanned = true;

	CDCHash hash;
	CDCHash pathHash;
	bool bScanDaily(true);
	bool bScanMain(true);
	if(file_hash_DB_utils::FileExistsInInternalDB(sFilePath.c_str(),
										  m_pFilesMap,
										  m_pMD5,
										  hash,
										  pathHash,
										  m_pMainDBInfo->m_nVersion,
										  m_pDailyDBInfo->m_nVersion,
										  bScanDaily,
										  bScanMain))
	{
		scan_log_utils::WriteLine("File exists in internal DB.");
		return true;
	}

	const char *sVirname;
	CPrecisionTimer timer;
	timer.Start();
#ifdef LOAD_MAIN_DB
	if(bScanMain)
	{
		scan_log_utils::WriteLine("Scanning Main DB.");
		if(m_pMainScan->ScanFile(sFilePath.c_str(), &sVirname))
		{
			sVirus.Format("%s", sVirname);
			scan_log_utils::LogVirus(sVirus, true);
			return false;
		}
	}
#endif

	if(bScanDaily)
	{
		scan_log_utils::WriteLine("Scanning Daily DB.");
		if(m_pDailyScan->ScanFile(sFilePath.c_str(), &sVirname))
		{
			sVirus.Format("%s", sVirname);
			scan_log_utils::LogVirus(sVirus, false);
			return false;
		}
	}
	double dSec = timer.Stop();
	scan_log_utils::LogTime("Scan time", dSec);

	file_hash_DB_utils::AddFileInfo(m_pFilesMap,
				1,
				m_pMainDBInfo->m_nVersion,
				m_pDailyDBInfo->m_nVersion,
				sFilePath.c_str(),
				hash,
				pathHash);

	sVirus.Empty();
	return true;
}

void CScanner::ScanFilesForOptimisation(CScanValidatorObs *pValidatorsObs)
{
	//if(!m_bLoaded)
	//{
	//	return;
	//}

	//std::vector<CFileInfoEx> arrFiles;

	//CFileInfoEx infoEx;
	//CFileInfo info;
	//CDCHash hash;

	//CMapI begin = m_pFilesMap->begin();
	//CMapI end = m_pFilesMap->end();
	//for(CMapI it = begin; it != end; ++it)
	//{
	//	hash	= it->first;
	//	info	= it->second;

	//	infoEx.m_pathHash			= hash;
	//	infoEx.m_nCount				= info.m_nCount;
	//	infoEx.m_nDailyDBVersion	= info.m_nDailyDBVersion;
	//	infoEx.m_nMainDBVersion		= info.m_nMainDBVersion;
	//	infoEx.m_sFilePath			= info.m_sFilePath;
	//	infoEx.m_fileHash			= info.m_fileHash;

	//	arrFiles.push_back(infoEx);
	//}

	//if(!pValidatorsObs->ContinueScan())
	//{
	//	return;
	//}

	//std::sort(arrFiles.begin(), arrFiles.end(), file_utils::SortFilesByUsage);

	//if(!pValidatorsObs->ContinueScan())
	//{
	//	return;
	//}

	//CString sVirus;
	//typedef std::vector<CFileInfoEx>::const_iterator CIt;
	//CIt f_end = arrFiles.end();
	//for(CIt f_it = arrFiles.begin(); f_it != f_end; ++f_it)
	//{
	//	infoEx = (*f_it);

	//	ScanFile(infoEx.m_sFilePath, sVirus);

	//	if(!pValidatorsObs->ContinueScan())
	//	{
	//		break;
	//	}

	//	if(pValidatorsObs->IsCPULoaded())
	//	{
	//		Sleep(10);
	//	}
	//}
}

void CScanner::RequestData(CTrayRequestData &data)
{
	if(NULL == m_pFilesMap)
	{
		return;
	}

	if(!m_bLoaded)
	{
		strcpy_s(data.m_sInfo, MAX_PATH, "DB not loaded");
		return;
	}

#ifdef LOAD_MAIN_DB
	data.m_nMainVersion = m_pMainDBInfo->m_nVersion;
	data.m_nMainSigCount = m_pMainDBInfo->m_nSigs;
	strcpy_s(data.m_sMainDate, MAX_PATH, m_pMainDBInfo->m_sTime);
#endif

	data.m_nDailyVersion = m_pDailyDBInfo->m_nVersion;
	data.m_nDailySigCount = m_pDailyDBInfo->m_nSigs;
	strcpy_s(data.m_sDailyDate, MAX_PATH, m_pDailyDBInfo->m_sTime);

	data.m_nFilesCount = m_pFilesMap->size();
}

int CScanner::GetFilesCount()
{ 
	if(NULL == m_pFilesMap)
	{
		return -1;
	}

	return m_pFilesMap->size();
};

void CScanner::SetScanSettings(BOOL bDeep, BOOL bOffice, BOOL bArchives, BOOL bPDF, BOOL bHTML)
{
	if(!m_bLoaded)
	{
		return;
	}

#ifdef LOAD_MAIN_DB
	m_pMainScan->SetScanSettings(bDeep, bOffice, bArchives, bPDF, bHTML);
#endif

	m_pDailyScan->SetScanSettings(bDeep, bOffice, bArchives, bPDF, bHTML);
}

void CScanner::SetFilesTypes(CString sTypes)
{
	m_types.clear();

	if(!sTypes.IsEmpty())
	{
		char *token;
		char *str = sTypes.GetBuffer(0);
		token = strtok(str, sgFileExtSeparator);
		if(NULL != token)
		{
			m_types.push_back(_strupr(token));
		}

		while(token != NULL)
		{
			token = strtok(NULL, sgFileExtSeparator);
			if(NULL != token)
			{
				m_types.push_back(_strupr(token));
			}
		}
	}
}

bool CScanner::ScanFileNoIntDB(LPCSTR sFile, CString &sVirus, DWORD PID, bool &bScanned)
{
	if(NULL == m_pFilesMap)
	{
		return true;;
	}

	scan_log_utils::LogHeader("ScanFileNoIntDB (manual/memory scan)", PID);

	if(!m_bLoaded)
	{
		scan_log_utils::WriteLine("DB not loaded.");
		return true;
	}

	bScanned = true;

	scan_log_utils::WriteLine(sFile);

	std::string sFilePath(sFile);
	std::transform(sFilePath.begin(), sFilePath.end(), sFilePath.begin(), toupper);

	const char *sVirname;
	CPrecisionTimer timer;
	timer.Start();
#ifdef LOAD_MAIN_DB
	scan_log_utils::WriteLine("Scanning Main DB.");
	if(m_pMainScan->ScanFile(sFilePath.c_str(), &sVirname))
	{
		sVirus.Format("%s", sVirname);
		double dSec = timer.Stop();
		scan_log_utils::LogTime("Scan time", dSec);
		scan_log_utils::LogVirus(sVirus, true);
		return false;
	}
#endif

	scan_log_utils::WriteLine("Scanning Daily DB.");
	if(m_pDailyScan->ScanFile(sFilePath.c_str(), &sVirname))
	{
		sVirus.Format("%s", sVirname);
		double dSec = timer.Stop();
		scan_log_utils::LogTime("Scan time", dSec);
		scan_log_utils::LogVirus(sVirus, false);
		return false;
	}
	double dSec = timer.Stop();
	scan_log_utils::LogTime("Scan time", dSec);

	CDCHash hash;
	CDCHash pathHash;
	bool bScanDaily;
	bool bScanMain;
	if(!file_hash_DB_utils::FileExistsInInternalDB(sFilePath.c_str(),
										  m_pFilesMap,
										  m_pMD5,
										  hash,
										  pathHash,
										  m_pMainDBInfo->m_nVersion,
										  m_pDailyDBInfo->m_nVersion,
										  bScanDaily,
										  bScanMain))
	{
		file_hash_DB_utils::AddFileInfo(m_pFilesMap,
					1,
					m_pMainDBInfo->m_nVersion,
					m_pDailyDBInfo->m_nVersion,
					sFilePath.c_str(),
					hash,
					pathHash);
	}

	sVirus.Empty();
	return true;
}

void CScanner::ReloadDB()
{
	Free();
	Init();

	LoadDatabases();
}

void CScanner::SetFilesMap(CScannedFileMap *pFilesMap)
{
	m_pFilesMap = pFilesMap;
}