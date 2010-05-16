#ifndef _SEND_OBJ_H__
#define _SEND_OBJ_H__

enum ESendInfoType
{
	EScan = 0,
	ERequest,
	EReloadSettings,
	ESendCount
};

class CSendObj
{
public:
	char m_sPath[MAX_PATH];
	int  m_nType;
};

#endif