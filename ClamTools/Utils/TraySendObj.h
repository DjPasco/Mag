#ifndef _TRAY_SEND_OBJ_H__
#define _TRAY_SEND_OBJ_H__

enum EInfoType
{
	EFile = 0,
	EData,
	ECount
};

class CTraySendObj
{
public:
	int m_nType;
	bool m_bMain;
	char m_sText[MAX_PATH];
    unsigned int m_nVersion;
    unsigned int m_nSigs;
};

#endif