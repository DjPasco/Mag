#ifndef _SEND_OBJ_H__
#define _SEND_OBJ_H__

enum ESendInfoType
{
	EScan = 0,
	ERequest,
	EReloadSettings,
	EReloadDB,
	EManualScan,
	ESendCount
};

class CSendObj
{
public:
	char m_sPath[MAX_PATH];
	int  m_nType;
	bool m_bUseInternalDB;
};

#endif