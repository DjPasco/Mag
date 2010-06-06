#ifndef _SEND_OBJ_H__
#define _SEND_OBJ_H__

enum ESendInfoType
{
	EScan = 0,
	ERequest,
	EReloadSettings,
	EReloadDB,
	EManualScan,
	EQuitServer,
	EStopManualScan,
	EStartManualScan,
	ESendCount
};

class CSendObj
{
public:
	CSendObj(){};
	CSendObj(const CSendObj &b)
	{
		strcpy_s(m_sPath, MAX_PATH, b.m_sPath);
		m_nType				= b.m_nType;
		m_bUseInternalDB	= b.m_bUseInternalDB;
		m_PID				= b.m_PID;
	}

public:
	char m_sPath[MAX_PATH];
	int  m_nType;
	bool m_bUseInternalDB;
	DWORD m_PID;
};

class CFileResult
{
public:
	char m_sVirusName[MAX_PATH];
	bool m_bOK;
	int m_nFilesCount;
	bool m_bScanned;
};

#endif