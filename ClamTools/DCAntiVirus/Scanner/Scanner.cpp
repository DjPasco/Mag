#include "stdafx.h"
#include "Scanner.h"

#include "DCSanner\DCSanner.h"
#include "ScanValidatorObs.h"

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

//#ifndef _DEBUG
	#define LOAD_MAIN_DB
//#endif

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
};

class CFileInfoEx: public CFileInfo
{
public:
	CDCHash m_Hash;
};

class CScannedFileMap : public std::map<CDCHash, CFileInfo/*, std::hash<unsigned char> */>
{
public:
	CScannedFileMap() { };
	~CScannedFileMap() { };
};

typedef CScannedFileMap::const_iterator CMapI;
typedef CScannedFileMap::iterator CMapEditI;

//#ifdef _DEBUG
//	CString gsDataFile = _T("PassDataD.dat");
//#else
	CString gsDataFile = _T("PassData.dat");
//#endif

namespace file_utils
{
	namespace internal
	{
		bool GetFileHash(LPCSTR sFile, CDCHash &hash, const EVP_MD *pMD5)
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

			return true;
		}
	};

	bool FileExistsInInternalDB(LPCSTR sFile,
								CScannedFileMap *pMapFiles,
								const EVP_MD *pMD5,
								CDCHash &hash,
								unsigned int nMainDBVersion,
								unsigned int nDailyDBVersion)
	{
		hash.resize(0);
		if(!internal::GetFileHash(sFile, hash, pMD5))
		{
			return false;
		}

		CMapEditI it = pMapFiles->find(hash);
		if(it != pMapFiles->end())
		{
			CFileInfo &info = (*it).second;
			info.m_nCount++;
			if(nMainDBVersion == info.m_nMainDBVersion && nDailyDBVersion == info.m_nDailyDBVersion)
			{
				return true;
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
		unsigned char pt = 0;
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
		FILE *pFile = fopen(gsDataFile, "rb");
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
		FILE *pFile = fopen(gsDataFile, "wb");
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

		if(NULL == strstr(sFile, "\\\\.\\"))//Named Pipe
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
	if(m_pMainScan->LoadDatabase("c:\\MAG_REPO\\LibClamAV\\main.cvd"))
#endif
	{
		if(m_pDailyScan->LoadDatabase("c:\\MAG_REPO\\LibClamAV\\daily.cvd"))
		{
			return true;
		}

		return false;
	}

	return false;
}

void CScanner::Init()
{
	m_pMainScan		= new CDCScanner;
	m_pDailyScan	= new CDCScanner;

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
	delete m_pDailyScan;
}

bool CScanner::ScanFile(LPCSTR sFile, CString &sVirus)
{
	if(!file_utils::FileIsSupported(sFile))
	{
		return true;
	}

	CDCHash hash;
	if(file_utils::FileExistsInInternalDB(sFile,
										  m_pFilesMap,
										  m_pMD5,
										  hash,
										  m_pMainScan->GetDBVersion(),
										  m_pDailyScan->GetDBVersion()))
	{
		return true;
	}

	const char *sVirname = NULL;

#ifdef LOAD_MAIN_DB
	if(m_pMainScan->ScanFile(sFile, sVirname))
	{
		sVirus.Format("&s", sVirname);
		return false;
	}
#endif

	if(m_pDailyScan->ScanFile(sFile, sVirname))
	{
		sVirus.Format("&s", sVirname);
		return false;
	}

	CFileInfo info;
	info.m_nCount = 1;
	info.m_nMainDBVersion = m_pMainScan->GetDBVersion();
	info.m_nDailyDBVersion = m_pDailyScan->GetDBVersion();
	info.m_sFilePath = sFile;
	file_utils::AddFileHash(m_pFilesMap, hash, info);

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

		infoEx.m_Hash				= hash;
		infoEx.m_nCount				= info.m_nCount;
		infoEx.m_nDailyDBVersion	= info.m_nDailyDBVersion;
		infoEx.m_nMainDBVersion		= info.m_nMainDBVersion;
		infoEx.m_sFilePath			= info.m_sFilePath;

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

		if(20 < pValidatorsObs->GetCPUUsage())
		{
			Sleep(10);
		}
	}
}