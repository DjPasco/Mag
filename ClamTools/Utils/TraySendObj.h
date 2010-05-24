#ifndef _TRAY_SEND_OBJ_H__
#define _TRAY_SEND_OBJ_H__

enum EInfoType
{
	EFile = 0,
	EData,
	EError,
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

#endif