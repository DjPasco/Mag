#ifndef _SCANNER_H__
#define _SCANNER_H__
#pragma once

#ifndef HEADER_ENVELOPE_H
	#include "openssl/evp.h"
#endif

#include <vector>

typedef std::vector<CString> CFilesTypes;

class CScannedFileMap;
class CCLScanner;
class CScanValidatorObs;
class CDBInfo;

class CScanner  
{
public:
	CScanner();
	virtual ~CScanner();

public:
	bool LoadDatabases();
	bool ScanFile(LPCSTR sFile, CString &sVirus, bool bCheckType = true);

	bool ScanFileNoIntDB(LPCSTR sFile, CString &sVirus);

	void ScanFilesForOptimisation(CScanValidatorObs *pValidatorsObs);

	void RequestData();

	void SetScanSettings(BOOL bDeep, BOOL bOffice, BOOL bArchives, BOOL bPDF, BOOL bHTML);
	void SetFilesTypes(CString sTypes);
	
private:
	void Init();
	void Free();

	void SendInfoToTray(bool bMain, CDBInfo *pDBInfo);
	void SendFileToTray(LPCSTR sFile, LPCSTR sVirus);

private:
	CCLScanner *m_pMainScan;
	CCLScanner *m_pDailyScan;

	CDBInfo *m_pMainDBInfo;
	CDBInfo *m_pDailyDBInfo;

	CScannedFileMap *m_pFilesMap;
	const EVP_MD *m_pMD5;

	CFilesTypes m_types;

	bool m_bLoaded;

	HANDLE m_hDataFile;
	DWORD m_high;
	DWORD m_low;
};

#endif
