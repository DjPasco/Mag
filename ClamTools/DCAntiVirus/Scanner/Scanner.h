#ifndef _SCANNER_H__
#define _SCANNER_H__
#pragma once

#include <hash_map>

typedef std::hash_map<LPCSTR, CString, std::hash<LPCSTR> > CScannedFileMap;

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
	CScannedFileMap m_FilesMap;
};

#endif
