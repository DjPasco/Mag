#include "..\Common\Common.h"
#include "..\Common\SysUtils.h"
#include "IniFile.h"

CIniFile::CIniFile(char* pszFileName)
{
	strcpy(m_szFileName, pszFileName);
}

CIniFile::~CIniFile()
{

}

void CIniFile::ReadString(
	const char* pszSection, 
	const char* pszIdent, 
	const char* pszDefault,
	char*       pszResult
	)
{
    DWORD dwResult = ::GetPrivateProfileString(
		pszSection,        // section name
		pszIdent,          // key name
		NULL,              // default string
		pszResult,         // destination buffer  
		MAX_PATH,          // size of destination buffer
		m_szFileName       // initialization file name
		);

	if (!dwResult)
		strcpy(pszResult, pszDefault);
}

BOOL CIniFile::ReadBool(
	const char* pszSection, 
	const char* pszIdent, 
	BOOL        bDefault
	)
{
	char    szResult[MAX_PATH];
	BOOL    bResult   = bDefault;
	char    szDefault[MAX_PATH];
		
	BoolToStr(bDefault, szDefault);

	ReadString(
		pszSection, 
		pszIdent,
		szDefault,
		szResult
		);

	bResult = StrToBool(szResult);

	return bResult;
}
