#ifndef _TRAY_SEND_OBJ_H__
#define _TRAY_SEND_OBJ_H__

enum EInfoType
{
	EFile = 0,
	EData,
	EMessage,
	ECount
};

class CTraySendObj
{
public:
	int m_nType;
	bool m_bMain;
	char m_sText[MAX_PATH];//File name;database time
	char m_sText2[MAX_PATH];//Virus name
    unsigned int m_nVersion;
    unsigned int m_nSigs;
	int m_nFilesCount;
};

class CTrayRequestData
{
public://Main info
	int m_nMainVersion;
	int m_nMainSigCount;
	char m_sMainDate[MAX_PATH];

public://Daily info
	int m_nDailyVersion;
	int m_nDailySigCount;
	char m_sDailyDate[MAX_PATH];

public:
	int m_nFilesCount;
	char m_sInfo[MAX_PATH];
};

#endif