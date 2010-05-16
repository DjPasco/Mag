#include "stdafx.h"
#include "Scanner.h"

#include "CLScanner.h"
#include "ScanValidatorObs.h"
#include "../TraySendObj.h"
#include "../Registry.h"

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

			while(!feof(pFile))
			{
				uRead = fread(data, sizeof(char), DC_HASH_BUFFER, pFile);
				EVP_DigestUpdate(&mdctx, data, uRead);
			}
			
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
								unsigned int nDailyDBVersion)
	{
		hash.resize(0);
		pathHash.resize(0);
		if(!internal::GetFileHash(sFile, hash, pathHash, pMD5))
		{
			return false;
		}

		CMapEditI it = pMapFiles->find(pathHash);
		if(it != pMapFiles->end())
		{
			CFileInfo &info = (*it).second;
			info.m_nCount++;

			if(info.m_fileHash == hash)
			{
				if(nMainDBVersion == info.m_nMainDBVersion && nDailyDBVersion == info.m_nDailyDBVersion)
				{
					return true;
				}
			}
		}
		
		return false;
	}

	void AddFileHash(CScannedFileMap *pMapFiles, CDCHash &hash, CFileInfo &info)
	{
		(*pMapFiles)[hash] = info;
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
	}

	void WritePassData(CScannedFileMap *pMapFiles)
	{
		FILE *pFile = fopen(path_utils::GetDataFilePath(), "wb");
		if(NULL == pFile)
		{
			return;
		}

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

		fclose(pFile);
	}

	bool FileIsSupported(LPCSTR sFile)
	{
		FILE *pFile = fopen(sFile, "rb");
		if(NULL == pFile)
		{
			return false;
		}

		if(NULL == strstr(sFile, "\\\\"))//Named Pipe
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
	Init();
	
	m_pMD5 = EVP_md5();

	m_pFilesMap = new CScannedFileMap;

	file_utils::ReadPassData(m_pFilesMap);
}

CScanner::~CScanner()
{
	Free();	
	file_utils::WritePassData(m_pFilesMap);
	m_pFilesMap->clear();

	delete m_pFilesMap;
}

bool CScanner::LoadDatabases()
{
#ifdef LOAD_MAIN_DB
	if(!m_pMainScan->LoadDatabase(path_utils::GetMainDBPath()))
	{
		return false;
	}

	m_pMainScan->GetInfo(m_pMainDBInfo);
#endif

	if(!m_pDailyScan->LoadDatabase(path_utils::GetDailyDBPath()))
	{
		return false;
	}

	m_pDailyScan->GetInfo(m_pDailyDBInfo);


	SendInfoToTray(false, m_pDailyDBInfo);

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
#ifdef LOAD_MAIN_DB
	m_pMainScan->FreeEngine();	
#endif
	m_pDailyScan->FreeEngine();
	
	delete m_pMainScan;
	delete m_pMainDBInfo;

	delete m_pDailyScan;
	delete m_pDailyDBInfo;
}

bool CScanner::ScanFile(LPCSTR sFile, CString &sVirus)
{
	std::string sFilePath(sFile);
	std::transform(sFilePath.begin(), sFilePath.end(), sFilePath.begin(), toupper);

	if(!file_utils::FileIsSupported(sFilePath.c_str()))
	{
		return true;
	}

	CDCHash hash;
	CDCHash pathHash;
	if(file_utils::FileExistsInInternalDB(sFilePath.c_str(),
										  m_pFilesMap,
										  m_pMD5,
										  hash,
										  pathHash,
										  m_pMainDBInfo->m_nVersion,
										  m_pDailyDBInfo->m_nVersion))
	{
		return true;
	}

	const char *sVirname = NULL;

#ifdef LOAD_MAIN_DB
	if(m_pMainScan->ScanFile(sFilePath.c_str(), sVirname))
	{
		sVirus.Format("&s", sVirname);
		SendFileToTray(sFile, sVirname);
		return false;
	}
#endif

	if(m_pDailyScan->ScanFile(sFilePath.c_str(), sVirname))
	{
		sVirus.Format("&s", sVirname);
		SendFileToTray(sFile, sVirname);
		return false;
	}

	CFileInfo info;
	info.m_nCount = 1;
	info.m_nMainDBVersion = m_pMainDBInfo->m_nVersion;
	info.m_nDailyDBVersion = m_pDailyDBInfo->m_nVersion;
	info.m_sFilePath = sFilePath.c_str();
	info.m_fileHash = hash;
	file_utils::AddFileHash(m_pFilesMap, pathHash, info);

	SendFileToTray(sFile, NULL);

	sVirus.Empty();
	return true;
}

void CScanner::ScanFilesForOptimisation(CScanValidatorObs *pValidatorsObs)
{
	std::vector<CFileInfoEx> arrFiles;

	CFileInfoEx infoEx;
	CFileInfo info;
	CDCHash hash;

	CMapI begin = m_pFilesMap->begin();
	CMapI end = m_pFilesMap->end();
	for(CMapI it = begin; it != end; ++it)
	{
		hash	= it->first;
		info	= it->second;

		infoEx.m_pathHash			= hash;
		infoEx.m_nCount				= info.m_nCount;
		infoEx.m_nDailyDBVersion	= info.m_nDailyDBVersion;
		infoEx.m_nMainDBVersion		= info.m_nMainDBVersion;
		infoEx.m_sFilePath			= info.m_sFilePath;
		infoEx.m_fileHash			= info.m_fileHash;

		arrFiles.push_back(infoEx);
	}

	if(!pValidatorsObs->ContinueScan())
	{
		return;
	}

	std::sort(arrFiles.begin(), arrFiles.end(), file_utils::SortFilesByUsage);

	if(!pValidatorsObs->ContinueScan())
	{
		return;
	}

	CString sVirus;
	typedef std::vector<CFileInfoEx>::const_iterator CIt;
	CIt f_end = arrFiles.end();
	for(CIt f_it = arrFiles.begin(); f_it != f_end; ++f_it)
	{
		infoEx = (*f_it);

		ScanFile(infoEx.m_sFilePath, sVirus);

		if(!pValidatorsObs->ContinueScan())
		{
			break;
		}

		if(pValidatorsObs->IsCPULoaded())
		{
			Sleep(10);
		}
	}
}

void CScanner::SendInfoToTray(bool bMain, CDBInfo *pDBInfo)
{
	HWND trayHwnd = FindWindow(NULL, "DCAntiVirus");

	if(NULL == trayHwnd)
	{
		return;
	}

	CTraySendObj obj;
	obj.m_nType = EData;
	obj.m_bMain = bMain;
	strcpy_s(obj.m_sText, MAX_PATH, pDBInfo->m_sTime);
	obj.m_nVersion = pDBInfo->m_nVersion;
	obj.m_nSigs = pDBInfo->m_nSigs;
	obj.m_nFilesCount = m_pFilesMap->size();

	COPYDATASTRUCT copy;
	copy.dwData = 1;
	copy.cbData = sizeof(obj);
	copy.lpData = &obj;

	SendMessage(trayHwnd, WM_COPYDATA, 0, (LPARAM) (LPVOID) &copy);
}

void CScanner::SendFileToTray(LPCSTR sFile, LPCSTR sVirus)
{
	HWND trayHwnd = FindWindow(NULL, "DCAntiVirus");

	if(NULL == trayHwnd)
	{
		return;
	}

	CTraySendObj obj;
	obj.m_nType = EFile;
	strcpy_s(obj.m_sText, MAX_PATH, sFile);
	if(NULL != sVirus)
	{
		strcpy_s(obj.m_sText2, MAX_PATH, sVirus);
	}
	else
	{
		strcpy_s(obj.m_sText2, MAX_PATH, "File is clean.");
	}

	obj.m_nFilesCount = m_pFilesMap->size();

	COPYDATASTRUCT copy;
	copy.dwData = 1;
	copy.cbData = sizeof(obj);
	copy.lpData = &obj;

	SendMessage(trayHwnd, WM_COPYDATA, 0, (LPARAM) (LPVOID) &copy);
}

void CScanner::RequestData()
{
#ifdef LOAD_MAIN_DB
	SendInfoToTray(true, m_pMainDBInfo);
#endif

	SendInfoToTray(false, m_pDailyDBInfo);
}

void CScanner::SetScanSettings(BOOL bDeep, BOOL bOffice, BOOL bArchives, BOOL bPDF, BOOL bHTML)
{
#ifdef LOAD_MAIN_DB
	m_pMainScan->SetScanSettings(bDeep, bOffice, bArchives, bPDF, bHTML);
#endif

	m_pDailyScan->SetScanSettings(bDeep, bOffice, bArchives, bPDF, bHTML);
}