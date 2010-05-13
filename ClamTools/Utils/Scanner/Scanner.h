#ifndef _SCANNER_H__
#define _SCANNER_H__
#pragma once

#ifndef HEADER_ENVELOPE_H
	#include "openssl/evp.h"
#endif

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
	bool ScanFile(LPCSTR sFile, CString &sVirus);

	void ScanFilesForOptimisation(CScanValidatorObs *pValidatorsObs);

	void ClearAndSave();

	void RequestData();
	
private:
	void Init();
	void Free();

	void SendInfoToTray(bool bMain, CDBInfo *pDBInfo);
	void SendFileToTray(LPCSTR sFile);

private:
	CCLScanner *m_pMainScan;
	CCLScanner *m_pDailyScan;

	CDBInfo *m_pMainDBInfo;
	CDBInfo *m_pDailyDBInfo;

	CScannedFileMap *m_pFilesMap;
	const EVP_MD *m_pMD5;

	HWND m_TrayHwnd;
};

#endif
