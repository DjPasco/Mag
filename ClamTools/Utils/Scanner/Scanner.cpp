#include "stdafx.h"
#include "Scanner.h"

#include "CLScanner.h"
#include "ScanValidatorObs.h"
#include "../TraySendObj.h"
#include "../Registry.h"
#include "../Settings.h"
#include "../Log.h"

#include <stdio.h>
#include <hash_map>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define LOAD_MAIN_DB
#define DC_HASH_SIZE 16
#define DC_HASH_BUFFER 1048576

typedef std::vector<unsigned char> CDCHash;

class CFileInfo
{
public:
	unsigned int m_nMainDBVersion;
	unsigned int m_nDailyDBVersion;
	unsigned int m_nCount;
	CString m_sFilePath;
	CDCHash m_fileHash;
};

class CFileInfoEx: public CFileInfo
{
public:
	CDCHash m_pathHash;
};

class CScannedFileMap : public std::map<CDCHash, CFileInfo>
{
public:
	CScannedFileMap() { };
	~CScannedFileMap() { };
};

class CPrecisionTimer
{
	LARGE_INTEGER lFreq, lStart;

public:
	CPrecisionTimer()
	{
		QueryPerformanceFrequency(&lFreq);
	}

	inline void Start()
	{
		QueryPerformanceCounter(&lStart);
	}

	inline double Stop()
	{
		// Return duration in seconds...
		LARGE_INTEGER lEnd;
		QueryPerformanceCounter(&lEnd);
		return (double(lEnd.QuadPart - lStart.QuadPart) / lFreq.QuadPart);
	}
};

typedef CScannedFileMap::const_iterator CMapI;
typedef CScannedFileMap::iterator CMapEditI;

namespace file_utils
{
	namespace internal
	{
		bool GetFileHash(LPCSTR sFile, CDCHash &hash, CDCHash &pathHash, const EVP_MD *pMD5)
		{
			FILE *pFile = fopen(sFile, "rb");
			if(NULL == pFile)
			{
				return false;
			}

			EVP_MD_CTX mdctx;
			unsigned char md_value[EVP_MAX_MD_SIZE];
			unsigned int md_len;

			char *data = (char *)malloc(DC_HASH_BUFFER);
			unsigned int uRead(0);

			EVP_MD_CTX_init(&mdctx);
			EVP_DigestInit_ex(&mdctx, pMD5, NULL);

			unsigned int uFileSize(0);

			while(!feof(pFile))
			{
				uRead = fread(data, sizeof(char), DC_HASH_BUFFER, pFile);
				EVP_DigestUpdate(&mdctx, data, uRead);
				uFileSize += uRead;
			}

			double dFileSizeMB = uFileSize/1048576.0;
			scan_log_utils::LogFileSize("File Size", dFileSizeMB);
			
			EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
			EVP_MD_CTX_cleanup(&mdctx);

			fclose(pFile);
			free((void *)data);

			if(md_len > DC_HASH_SIZE)
			{
				return false;//Wrong data
			}

			hash.resize(DC_HASH_SIZE);
			for(unsigned int i = 0; i < md_len; ++i)
			{
				hash[i] = md_value[i];
			}

			EVP_MD_CTX_init(&mdctx);
			EVP_DigestInit_ex(&mdctx, pMD5, NULL);
			EVP_DigestUpdate(&mdctx, sFile, strlen(sFile));
			EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
			EVP_MD_CTX_cleanup(&mdctx);

			if(md_len > DC_HASH_SIZE)
			{
				return false;//Wrong data
			}
			pathHash.resize(DC_HASH_SIZE);
			for(unsigned int i = 0; i < md_len; ++i)
			{
				pathHash[i] = md_value[i];
			}

			return true;
		}
	};

	bool FileExistsInInternalDB(LPCSTR sFile,
								CScannedFileMap *pMapFiles,
								const EVP_MD *pMD5,
								CDCHash &hash,
								CDCHash &pathHash,
								unsigned int nMainDBVersion,
								unsigned int nDailyDBVersion,
								bool &bScanDaily,
								bool &bScanMain)
	{
		hash.resize(0);
		pathHash.resize(0);
		CPrecisionTimer timer;
		timer.Start();
		if(!internal::GetFileHash(sFile, hash, pathHash, pMD5))
		{
			return false;
		}
		
		double dSec = timer.Stop();
		scan_log_utils::LogTime("Hash time", dSec);

		CMapEditI it = pMapFiles->find(pathHash);
		if(it != pMapFiles->end())
		{
			CFileInfo &info = (*it).second;
			info.m_nCount++;

			if(info.m_fileHash == hash)
			{
				bool bOld = true;
				if(nMainDBVersion == info.m_nMainDBVersion)
				{
					bOld = false;
					bScanMain = false;
				}

				if(nDailyDBVersion == info.m_nDailyDBVersion)
				{
					bOld = false;
					bScanDaily = false;
				}

				if(bOld)
				{
					return false;
				}
				
				return true;
			}
		}
		
		return false;
	}

	bool ReadHash(FILE *pFile, CDCHash &hash)
	{
		hash.resize(DC_HASH_SIZE);
		int pt = 0;
		for(int i = 0; i < DC_HASH_SIZE; ++i)
		{
			if(1 != fscanf(pFile, "%x", &pt))//Error.
			{
				return false;
			}

			hash[i] = pt;
		}

		return true;
	}

	bool ReadPath(FILE *pFile, CString &sPath)
	{
		char s;
		while(true)
		{
			if(0 == fread(&s, sizeof(char), 1, pFile))
			{
				return false;
			}

			if(s == 13 || s == 10)
			{
				break;
			}

			sPath += s;
		}

		return true;
	}

	void ReadPassData(CScannedFileMap *pMapFiles)
	{
		FILE *pFile = fopen(path_utils::GetDataFilePath(), "rb");
		if(NULL == pFile)
		{
			return;
		}

		char symbol;
		CString sBuffer;

		while(!feof(pFile))
		{
			CDCHash hash;
			if(!ReadHash(pFile, hash))
			{
				break;
			}

			CFileInfo info;
			if(!ReadHash(pFile, info.m_fileHash))
			{
				break;
			}

			fscanf(pFile, "%u", &info.m_nMainDBVersion);
			fscanf(pFile, "%u", &info.m_nDailyDBVersion);
			fscanf(pFile, "%u", &info.m_nCount);

			fread(&symbol, sizeof(char), 1, pFile);

			ReadPath(pFile, info.m_sFilePath);

			(*pMapFiles)[hash] = info;
		}

		fclose(pFile);

		scan_log_utils::LogInt("Loaded items", pMapFiles->size());
	}

	void WritePassData(CScannedFileMap *pMapFiles)
	{
		FILE *pFile = fopen(path_utils::GetDataFilePath(), "wb");
		if(NULL == pFile)
		{
			return;
		}

		scan_log_utils::LogInt("Saved items", pMapFiles->size());

		CDCHash hash;
		CFileInfo info;
		CMapI begin = pMapFiles->begin();
		CMapI end = pMapFiles->end();

		for(CMapI it = begin; it != end; ++it)
		{
			hash	= it->first;
			info	= it->second;

			int nHashSize = hash.size();
			
			if(DC_HASH_SIZE != nHashSize)
			{
				continue;//Corrupted record.
			}

			for(int i = 0; i < DC_HASH_SIZE; ++i)
			{
				fprintf(pFile, "%02x ", hash[i]);//8 ouputs like 08.
			}

			for(int i = 0; i < DC_HASH_SIZE; ++i)
			{
				fprintf(pFile, "%02x ", info.m_fileHash[i]);//8 ouputs like 08.
			}

			fprintf(pFile, "%u %u %u %s\n", info.m_nMainDBVersion, info.m_nDailyDBVersion, info.m_nCount, info.m_sFilePath);
		}

		fflush(pFile);
		fclose(pFile);
	}

	bool FileIsSupported(LPCSTR sFile, CFilesTypes &types)
	{
		FILE *pFile = fopen(sFile, "rb");
		if(NULL == pFile)
		{
			return false;
		}

		fclose(pFile);

		if(0 == types.size())
		{
			return true;
		}

		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];

		_splitpath(sFile, drive, dir, fname, ext);

		char extension[_MAX_EXT];
		strcpy(extension, &ext[1]);//Remowing point before extension

		if(types.end() != std::find(types.begin(), types.end(), extension))
		{
			return true;
		}

		return false;
	}

	bool SortFilesByUsage(const CFileInfoEx &file1, const CFileInfoEx &file2)
	{
		return file1.m_nCount > file2.m_nCount;
	}
}

CScanner::CScanner()
{
	m_bLoaded = false;

	Init();
	
	m_pMD5 = EVP_md5();

	m_pFilesMap = new CScannedFileMap;

	scan_log_utils::LogHeader("Loading hash DB", GetCurrentProcessId());
	CPrecisionTimer timer;
	timer.Start();
	file_utils::ReadPassData(m_pFilesMap);
	double dSec = timer.Stop();
	scan_log_utils::LogTime("Load time", dSec);

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

	scan_log_utils::LogHeader("Saving hash DB", GetCurrentProcessId());
	CPrecisionTimer timer;
	timer.Start();
	file_utils::WritePassData(m_pFilesMap);
	double dSec = timer.Stop();
	scan_log_utils::LogTime("Save time", dSec);

	m_pFilesMap->clear();

	delete m_pFilesMap;
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
		if(!file_utils::FileIsSupported(sFilePath.c_str(), m_types))
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
	if(file_utils::FileExistsInInternalDB(sFilePath.c_str(),
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

	CFileInfo info;
	info.m_nCount = 1;
	info.m_nMainDBVersion = m_pMainDBInfo->m_nVersion;
	info.m_nDailyDBVersion = m_pDailyDBInfo->m_nVersion;
	info.m_sFilePath = sFilePath.c_str();
	info.m_fileHash = hash;
	(*m_pFilesMap)[pathHash] = info;

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
		scan_log_utils::LogVirus(sVirus, true);
		return false;
	}
#endif

	scan_log_utils::WriteLine("Scanning Daily DB.");
	if(m_pDailyScan->ScanFile(sFilePath.c_str(), &sVirname))
	{
		sVirus.Format("%s", sVirname);
		scan_log_utils::LogVirus(sVirus, false);
		return false;
	}
	double dSec = timer.Stop();
	scan_log_utils::LogTime("Scan time", dSec);

	CDCHash hash;
	CDCHash pathHash;
	bool bScanDaily;
	bool bScanMain;
	if(!file_utils::FileExistsInInternalDB(sFilePath.c_str(),
										  m_pFilesMap,
										  m_pMD5,
										  hash,
										  pathHash,
										  m_pMainDBInfo->m_nVersion,
										  m_pDailyDBInfo->m_nVersion,
										  bScanDaily,
										  bScanMain))
	{
		CFileInfo info;
		info.m_nCount = 1;
		info.m_nMainDBVersion	= m_pMainDBInfo->m_nVersion;
		info.m_nDailyDBVersion	= m_pDailyDBInfo->m_nVersion;
		info.m_sFilePath		= sFilePath.c_str();
		info.m_fileHash			= hash;
		(*m_pFilesMap)[pathHash]= info;
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