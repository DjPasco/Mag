#ifndef _FILE_HASH_DB_UTILS_H__
#define _FILE_HASH_DB_UTILS_H__
#pragma once

#include "../Log.h"
#include "openssl/evp.h"
#include "PrecisionTimer.h"
#include <vector>
#include <map>


//Internal hash DB utils.

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

public:
	//hash DB hols his critical section (to handle multiple threads)
	CRITICAL_SECTION secFilesHashDB;
};

//vevtor iterators
typedef CScannedFileMap::const_iterator CMapI;
typedef CScannedFileMap::iterator CMapEditI;

//Files types array
typedef std::vector<CString> CFilesTypes;

namespace file_hash_DB_utils
{
	namespace internal
	{
		//Calculates file hash
		static bool GetFileHash(LPCSTR sFile, CDCHash &hash, CDCHash &pathHash, const EVP_MD *pMD5)
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

	//Checks if file exists in internal hash DB
	static bool FileExistsInInternalDB(LPCSTR sFile,
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

		//Lock hash DB
		EnterCriticalSection(&pMapFiles->secFilesHashDB); 
		bool bRet(false);

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

				if(!bOld)
				{
					bRet = true;
				}
			}
		}

		//UnLock hash DB
		LeaveCriticalSection(&pMapFiles->secFilesHashDB); 
		return bRet;
	}

	//Reads hash value from hash DB file
	static bool ReadHash(FILE *pFile, CDCHash &hash)
	{
		hash.resize(DC_HASH_SIZE);
		int pt = 0;
		for(int i = 0; i < DC_HASH_SIZE; ++i)
		{
			if(1 != fscanf(pFile, "%x", &pt))//Error if no symbol readed
			{
				return false;
			}

			hash[i] = pt;
		}

		return true;
	}

	//Reads path from hash DB file
	static bool ReadPath(FILE *pFile, CString &sPath)
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

	//Reads data to memory from hash DB file.
	static void ReadPassData(CScannedFileMap *pMapFiles)
	{
		FILE *pFile = fopen(path_utils::GetDataFilePath(), "rb");
		if(NULL == pFile)
		{
			return;
		}

		char symbol;
		CString sBuffer;

		//read from all file
		while(!feof(pFile))
		{
			//File path hash
			CDCHash hash;
			if(!ReadHash(pFile, hash))
			{
				break;
			}

			//File hash
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

	//Writes hash DB info from memory to file (saving)
	static void WritePassData(CScannedFileMap *pMapFiles)
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

	//Chech if file is supported.
	static bool FileIsSupported(LPCSTR sFile, CFilesTypes &types)
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

	//Function to sort files by using count.
	static bool SortFilesByUsage(const CFileInfoEx &file1, const CFileInfoEx &file2)
	{
		return file1.m_nCount > file2.m_nCount;
	}

	//Add file to hash DB.
	static void AddFileInfo(CScannedFileMap *pMapFiles,
					 int nCount,
					 int nMainVersion,
					 int nDailyVersion,
					 LPCSTR sPath,
					 CDCHash hash,
					 CDCHash pathHash)
	{
		CFileInfo info;
		info.m_nCount = nCount;
		info.m_nMainDBVersion = nMainVersion;
		info.m_nDailyDBVersion = nDailyVersion;
		info.m_sFilePath = sPath;
		info.m_fileHash = hash;

		EnterCriticalSection(&pMapFiles->secFilesHashDB); 
		(*pMapFiles)[pathHash] = info;
		LeaveCriticalSection(&pMapFiles->secFilesHashDB);
	}

	static bool CreateHashDBCriticalSection(CScannedFileMap *pFilesMap)
	{
		// Initialize the critical section one time only.
		if (!InitializeCriticalSectionAndSpinCount(&pFilesMap->secFilesHashDB, 0x80000400))
		{
			return false;
		}

        return true;
	}
	
	static void DeleteHashDBCriticalSection(CScannedFileMap *pFilesMap)
	{
		// Release resources used by the critical section object.
		DeleteCriticalSection(&pFilesMap->secFilesHashDB);
	}
}

#endif
