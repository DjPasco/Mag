#include "stdafx.h"
#include "Scanner.h"

#include "DCSanner\DCSanner.h"
#include <stdio.h>
#include <map>
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
	CString m_sFilePath;
};

//typedef char CDCHash[16];

struct eqHash
{
	bool operator()(const std::string &s1, const std::string &s2) const
	{
		if(16 != s1.size() || 16 != s2.size())
		{
			return false;
		}

		for(int i = 0; i < 16; ++i)
		{
			if(s1[i] != s2[i])
			{
				return false;
			}
		}

		return true;
	}
};

class CScannedFileMap : public std::map<std::string, CFileInfo/*, eqHash*/>
{
public:
	CScannedFileMap() { };
	~CScannedFileMap() { };
};

typedef CScannedFileMap::const_iterator CMapI;

#ifdef _DEBUG
	CString gsDataFile = _T("PassDataD.dat");
#else
	CString gsDataFile = _T("PassData.dat");
#endif

namespace file_utils
{
	namespace internal
	{
		bool GetFileHash(LPCSTR sFile, std::string &hash, const EVP_MD *pMD5)
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
			EVP_DigestUpdate(&mdctx, data, lSize);
			EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
			EVP_MD_CTX_cleanup(&mdctx);

			free((void *)data);

			if(md_len > 16)
			{
				return false;//Wrong data
			}

			hash.resize(16);
			
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
								std::string &hash,
								unsigned int nMainDBVersion,
								unsigned int nDailyDBVersion)
	{
		hash.resize(0);
		if(!internal::GetFileHash(sFile, hash, pMD5))
		{
			return false;
		}

		CMapI it = pMapFiles->find(hash);
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

	void AddFileHash(CScannedFileMap *pMapFiles, std::string &hash, int nMainVersion, int nDailyVersion, LPCSTR sFilePath)
	{
		CFileInfo info;
		info.m_nMainDBVersion = nMainVersion;
		info.m_nDailyDBVersion = nDailyVersion;
		info.m_sFilePath = sFilePath;
		(*pMapFiles)[hash] = info;
	}

	bool ReadHash(FILE *pFile, std::string &hash)
	{
		char sHashBuffer[16];
		int nRead = fread(sHashBuffer, sizeof(char), 16, pFile); 
		if(0 == nRead || 16 != nRead)
		{
			return false;
		}
		
		hash.resize(16);
		
		for(int i = 0; i < 16; ++i)
		{
			hash[i] = sHashBuffer[i];
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
			std::string hash;
			if(!ReadHash(pFile, hash))
			{
				break;
			}

			CFileInfo info;
			fscanf(pFile, "%u", &info.m_nMainDBVersion);
			fscanf(pFile, "%u", &info.m_nDailyDBVersion);

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

		std::string sHash;
		CFileInfo info;
		CMapI begin = pMapFiles->begin();
		CMapI end = pMapFiles->end();
		for(CMapI it = begin; it != end; ++it)
		{
			sHash	= it->first;
			info	= it->second;

			int nHashSize = sHash.size();
			
			if(16 != nHashSize)
			{
				continue;//Corrupted data.
			}
				
			for(int i = 0; i < nHashSize; ++i)
			{
				unsigned char ps(sHash[i]);
				fwrite(&ps, sizeof(unsigned char), 1, pFile);
			}

			fprintf(pFile, " %u %u %s\n", info.m_nMainDBVersion, info.m_nDailyDBVersion, info.m_sFilePath);
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
	if(!file_utils::FileIsSupported(sFile))
	{
		return true;
	}

	std::string hash;
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

	file_utils::AddFileHash(m_pFilesMap, hash, m_pMainScan->GetDBVersion(), m_pDailyScan->GetDBVersion(), sFile);

	sVirus.Empty();
	return false;
}