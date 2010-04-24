#ifndef _SCANNER_H__
#define _SCANNER_H__
#pragma once

#ifndef HEADER_ENVELOPE_H
	#include "openssl/evp.h"
#endif

class CScannedFileMap;
class CDCScanner;

class CScanner  
{
public:
	CScanner();
	virtual ~CScanner();

public:
	bool LoadDatabases();
	bool ScanFile(LPCSTR sFile, CString &sVirus);
	
private:
	void Init();
	void Free();

private:
	CDCScanner *m_pMainScan;
	CDCScanner *m_pDailyScan;

	CScannedFileMap *m_pFilesMap;
	const EVP_MD *m_pMD5;
};

#endif
