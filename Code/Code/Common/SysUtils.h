#ifndef _SYSUTILS_H_
#define _SYSUTILS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Common.h"

static BOOL IsToolHelpSupported()
{

	BOOL    bResult(FALSE);
	HMODULE hModToolHelp;
	PROC    pfnCreateToolhelp32Snapshot;

	hModToolHelp = ::LoadLibrary( "KERNEL32.DLL" );
	if (NULL != hModToolHelp)
	{
		pfnCreateToolhelp32Snapshot = ::GetProcAddress( 
			hModToolHelp,
            "CreateToolhelp32Snapshot"
			);
		bResult = (NULL != pfnCreateToolhelp32Snapshot);
		::FreeLibrary(hModToolHelp);
	} // if

	return bResult;
}


static BOOL IsPsapiSupported()
{
	BOOL bResult = FALSE;
	HMODULE hModPSAPI = NULL;

	hModPSAPI = ::LoadLibrary( "PSAPI.DLL" );
	bResult = (NULL != hModPSAPI);
	if (NULL != hModPSAPI)
		::FreeLibrary(hModPSAPI);

	return bResult;
}

static BOOL WINAPI IsWindows9x() 
{
	BOOL bResult = FALSE;
	OSVERSIONINFO vi = { sizeof(vi) };

	::GetVersionEx(&vi);
	if (vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) 
		bResult = TRUE;

	return bResult;
}
	

static BOOL WINAPI IsWindowsNT4() 
{
   OSVERSIONINFO vi = { sizeof(vi) };

   ::GetVersionEx(&vi);
   
   return ( (vi.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
	        (vi.dwMajorVersion == 4) );
}

static HMODULE ModuleFromAddress(PVOID pv) 
{
	MEMORY_BASIC_INFORMATION mbi;

	return ((::VirtualQuery(pv, &mbi, sizeof(mbi)) != 0) 
	        ? (HMODULE) mbi.AllocationBase : NULL);
}

static BOOL GetProcessHostFullName(char* pszFullFileName)
{
	DWORD dwResult = 0;
	::ZeroMemory((PBYTE)pszFullFileName, MAX_PATH);
	if (TRUE != ::IsBadReadPtr((PBYTE)pszFullFileName, MAX_PATH))
		dwResult = ::GetModuleFileName(
			NULL,                   // handle to module
			pszFullFileName,        // file name of module
			MAX_PATH                // size of buffer
			);

	return (dwResult != 0);
}


static BOOL GetProcessHostName(char* pszFullFileName)
{
	BOOL  bResult;
	char  *pdest;

	int   ch = '\\';

	bResult = GetProcessHostFullName(pszFullFileName);
	if (bResult)
	{
		// Search backward 
		pdest = strrchr(pszFullFileName, ch);
		if( pdest != NULL )
			strcpy(pszFullFileName, &pdest[1]);
	} // if

	return bResult;
}

static BOOL ExtractModuleFileName(char* pszFullFileName)
{
	BOOL  bResult = FALSE;

	if (TRUE != ::IsBadReadPtr(pszFullFileName, MAX_PATH))
	{
		char  *pdest;
		int   ch = '\\';

		// Search backward 
		pdest = strrchr(pszFullFileName, ch);
		if( pdest != NULL )
			strcpy(pszFullFileName, &pdest[1]);

		bResult = TRUE;
	} // if

	return bResult;
}

static BOOL ReplaceFileName(
	char* pszOldFileName,
	char* pszBaseNewFileName,
	char* pszNewFileName
	)
{
	BOOL  bResult = FALSE;
	char *pdest;
	int   ch = '\\';

	if ( 
		(TRUE != ::IsBadReadPtr(pszOldFileName, strlen(pszOldFileName) + 1))  &&
		(TRUE != ::IsBadReadPtr(pszBaseNewFileName, strlen(pszBaseNewFileName) + 1))
		)
	{
		if (TRUE != ::IsBadReadPtr(pszNewFileName, MAX_PATH))
		{
			::ZeroMemory(pszNewFileName, sizeof(MAX_PATH));
			strcpy(pszNewFileName, pszOldFileName);
			// Search backward and replaces the dll name with the hook one
			pdest = strrchr(pszNewFileName, ch);
			if( pdest != NULL )
				strcpy(&pdest[1], pszBaseNewFileName);
			else
				strcpy(pszNewFileName, pszBaseNewFileName);

			bResult = TRUE;
		} // if
	} // if

	return bResult;
}

static BOOL ReplaceFileName(
	HINSTANCE hOldFile,
	char*     pszBaseNewFileName,
	char*     pszNewFileName
	)
{
	BOOL  bResult = FALSE;
	char  szFileName[MAX_PATH];
	
	if (TRUE != ::IsBadReadPtr(pszBaseNewFileName, strlen(pszBaseNewFileName) + 1))
	{
		if (TRUE != ::IsBadReadPtr(pszNewFileName, MAX_PATH))
		{
			::ZeroMemory(pszNewFileName, sizeof(MAX_PATH));
			if (NULL != hOldFile)
			{
				if (0 != ::GetModuleFileName(hOldFile, szFileName, MAX_PATH))
					bResult = 
						ReplaceFileName(
							szFileName, 
							pszBaseNewFileName, 
							pszNewFileName);
			} // if
		} // if
	} // if

	return bResult;
}

static BOOL StrToBool(const char* pszValue)
{
	return ( (0 == stricmp("YES", pszValue)) ||
		     (0 == stricmp("Y", pszValue)) ||
		     (0 == stricmp("TRUE", pszValue)) );
}

static void BoolToStr(BOOL bValue, char* pszResult)
{
	bValue ? strcpy(pszResult, "Yes") : 
	         strcpy(pszResult, "No");
}

static void TrimLeft(
	const char*  pszParam,
	char*        pszResult
	)
{
	char szBuffer[MAX_PATH];
	strcpy(szBuffer, "\0");
	if ( (TRUE != ::IsBadStringPtr(pszParam, MAX_PATH)) &&
		 (strlen(pszParam) > 0) )
	{
		DWORD dwIndex = 0;
		while ( (dwIndex < strlen(pszParam)) && (pszParam[dwIndex] == ' ') )
			dwIndex++;
		if (dwIndex < strlen(pszParam))
			strcpy(szBuffer, &pszParam[dwIndex]);
	} // if
	strcpy(pszResult, szBuffer);
}
	
static void TrimRight(
	const char*  pszParam,
	char*        pszResult
	)
{
	char szBuffer[MAX_PATH];
	strcpy(szBuffer, "\0");
	if ( (TRUE != ::IsBadStringPtr(pszParam, MAX_PATH)) &&
		 (strlen(pszParam) > 0) )
	{
		int nIndex = strlen(pszParam) - 1;
		while ( (nIndex >= 0) && (pszParam[nIndex] == ' ') )
			nIndex--;
		if (nIndex >= 0)
		{
			memcpy(
				(PBYTE)szBuffer, 
				(PBYTE)pszParam, 
				(nIndex + 1)
				); 
			szBuffer[nIndex+1] = 0;
		} // if
	} // if
	strcpy(pszResult, szBuffer);
}

static void Trim(
	const char*  pszParam,
	char*        pszResult
	)
{
	TrimLeft(pszParam, pszResult);
	TrimRight(pszParam, pszResult);
}



static BOOL GetNextCommaSeparatedString(
	const char*  pszParam,
	char*        pszResult,
	DWORD        dwLength,
	long*        pnCommaPos
	)
{
	*pnCommaPos = -1;
	BOOL   bResult = FALSE;
	char*  pdest;
	strcpy(pszResult, "\0");

	if (strlen(pszParam) > 0)
	{
		::ZeroMemory((PBYTE)pszResult, dwLength);

		pdest = strstr(pszParam, ",");
		if (pdest)
			*pnCommaPos = pdest - pszParam - 1;
		else
			*pnCommaPos = strlen(pszParam);
		memcpy(
			(PBYTE)pszResult, 
			(PBYTE)pszParam, 
			((*pnCommaPos) + 1)
			); 
		(*pnCommaPos)++;

		Trim(pszResult, pszResult);

		bResult = TRUE;
	} // if

	return bResult;
}


static BOOL UnicodeToAnsi(
	LPWSTR pszwUniString, 
	LPSTR  pszAnsiBuff,
	DWORD  dwAnsiBuffSize
	)
{
	int  iRet ;
    iRet = ::WideCharToMultiByte(
		CP_ACP,
		0,
		pszwUniString,
		-1,
		pszAnsiBuff,
		dwAnsiBuffSize,
		NULL,
		NULL
		);
	return (0 != iRet);
}


#endif
