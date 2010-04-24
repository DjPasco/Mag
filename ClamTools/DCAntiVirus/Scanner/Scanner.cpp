#include "stdafx.h"
#include "Scanner.h"

#include "DCSanner\DCSanner.h"
#include <stdio.h>
#include <hash_map>
#include <string>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

class CFileInfo
{
public:
	unsigned int m_nMainDBVersion;
	unsigned int m_nDailyDBVersion;
};

class CScannedFileMap : public std::hash_map<LPCSTR, CFileInfo, std::hash<LPCSTR> >
{
public:
	CScannedFileMap() { };
	~CScannedFileMap() { };
};

typedef CScannedFileMap::const_iterator CMapI;

CString gsDataFile = _T("PassData.dat");

namespace file_utils
{
	namespace internal
	{
		bool GetFileHash(LPCSTR sFile, CString &sHash, const EVP_MD *pMD5)
		{
			FILE *pFile = fopen(sFile, "rb");
			if(NULL == pFile)
			{
				return false;
			}

			EVP_MD_CTX mdctx;
			unsigned char md_value[EVP_MAX_MD_SIZE];
			unsigned int md_len;

			fseek(pFile, 0L, SEEK_END);
			long lSize = ftell(pFile);
			fseek(pFile, 0L, SEEK_SET);
			
			char *data = (char *)malloc(lSize);
			fread(data, sizeof(char), lSize, pFile);
			fclose(pFile);

			EVP_MD_CTX_init(&mdctx);
			EVP_DigestInit_ex(&mdctx, pMD5, NULL);
			EVP_DigestUpdate(&mdctx, data, strlen(data));
			EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
			EVP_MD_CTX_cleanup(&mdctx);

			free((void *)data);

			
			sHash.Empty();
			for(unsigned int i = 0; i < md_len; ++i)
			{
				sHash += md_value[i];
			}

			return true;
		}
	};

	bool FileExistsInInternalDB(LPCSTR sFile,
								CScannedFileMap *pMapFiles,
								const EVP_MD *pMD5,
								CString &sHash,
								unsigned int nMainDBVersion,
								unsigned int nDailyDBVersion)
	{
		if(!internal::GetFileHash(sFile, sHash, pMD5))
		{
			return false;
		}
		
		CMapI it = pMapFiles->find(sHash);
		if(it != pMapFiles->end())
		{
			CFileInfo info = (*it).second;
			if(nMainDBVersion == info.m_nMainDBVersion && nDailyDBVersion == info.m_nDailyDBVersion)
			{
				return true;
			}
		}
		
		return false;
	}

	void AddFileHash(CScannedFileMap *pMapFiles, CString &sHash, int nMainVersion, int nDailyVersion)
	{
		CFileInfo info;
		info.m_nMainDBVersion = nMainVersion;
		info.m_nDailyDBVersion = nDailyVersion;
		(*pMapFiles)[sHash] = info;
	}
}

CScanner::CScanner()
{
	Init();
	
	m_pMD5 = EVP_md5();

	m_pFilesMap = new CScannedFileMap;
	m_pFilesMap->clear();
}

CScanner::~CScanner()
{
	Free();
	m_pFilesMap->clear();
	delete m_pFilesMap;
}

bool CScanner::LoadDatabases()
{
#ifndef _DEBUG
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

#ifndef _DEBUG
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
#ifndef _DEBUG
	m_pMainScan->FreeEngine();
	
#endif
	m_pDailyScan->FreeEngine();
	
	delete m_pMainScan;
	delete m_pDailyScan;
}

bool CScanner::ScanFile(LPCSTR sFile, CString &sVirus)
{
	CString sHash;
	if(file_utils::FileExistsInInternalDB(sFile,
										  m_pFilesMap,
										  m_pMD5,
										  sHash,
										  m_pMainScan->GetDBVersion(),
										  m_pDailyScan->GetDBVersion()))
	{
		return true;
	}

	const char *sVirname = NULL;

#ifndef _DEBUG
	if(m_pMainScan->ScanFile(sFile, sVirname))
	{
		sVirus.Format("&s", sVirname);
		return true;
	}
#endif

	if(m_pDailyScan->ScanFile(sFile, sVirname))
	{
		sVirus.Format("&s", sVirname);
		return true;
	}

	file_utils::AddFileHash(m_pFilesMap, sHash, m_pMainScan->GetDBVersion(), m_pDailyScan->GetDBVersion());

	sVirus.Empty();
	return false;
}