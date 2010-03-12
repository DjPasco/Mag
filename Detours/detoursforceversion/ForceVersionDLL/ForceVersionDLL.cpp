// ForceVersionDLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "../../detours/include/detours.h"
#include <tchar.h>
#include <stdio.h>
#include "ForceVersionDLL.h"

// To get the payload structure information
#include "..\ForceVersion\ForceVersion.h"

extern BOOL (WINAPI *pfuncGetVersionEx)(LPOSVERSIONINFO pOSVersionInfo);

// Global variables that will hold the version details
// that our client app will send to us
int g_iMajor = 0;
int g_iMinor = 0;
int g_iBuild = 0;
BOOL g_fGotDetails = FALSE;

// This is our function to which all calls to GetVersionEx will be detoured to
BOOL WINAPI ForceGetVersionEx(LPOSVERSIONINFO pOSVersionInfo)
{
	// Have we got the details?
	if (!g_fGotDetails)
	{
		// Get the version details from the payload in the process
		// by enumerating the modules in the process
		struct VersionPayLoad *pPayload = NULL;
		DWORD dwSize = 0 ;
		HMODULE hMod = DetourEnumerateModules(NULL);
		while(hMod != INVALID_HANDLE_VALUE)
		{
			// Got a valid module handle - check it for the payload
			dwSize = 0;
			pPayload = (struct VersionPayLoad *)DetourFindPayload(hMod,my_guid,&dwSize);
			if (pPayload != NULL)
			{
				// Found the payload - copy the details to the globals and set the flag
				// that we have the version details..
				g_iMajor = pPayload->iMajor;
				g_iMinor = pPayload->iMinor;
				g_iBuild = pPayload->iBuild;
				g_fGotDetails = TRUE;
				break;
			}

			// move to the next module..
			hMod = DetourEnumerateModules(hMod);
		}
	}

	// Set the version to be returned
	pOSVersionInfo->dwMajorVersion = g_iMajor;
	pOSVersionInfo->dwMinorVersion = g_iMinor;
	pOSVersionInfo->dwBuildNumber = g_iBuild;
	pOSVersionInfo->dwPlatformId = VER_PLATFORM_WIN32_NT;
	_tcscpy(pOSVersionInfo->szCSDVersion, L"");
	
	/*OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(osinfo);
	BOOL fOrigStatus = pfuncGetVersionEx(&osinfo);
	if (fOrigStatus)
	{
		printf("Orig OS Version: %d.%d\n",osinfo.dwMajorVersion, osinfo.dwMinorVersion);
		printf("Orig Platform ID: %d\n", osinfo.dwPlatformId);
		_tprintf(L"Description: %s\n",osinfo.szCSDVersion);
	}*/
	return TRUE;	
}

// This is the exported function that will tell us what version details are
// to be returned
FORCEVERSIONDLL_API int SetVersion(int iMajor, int iMinor, int iBuild)
{
	g_iMajor = iMajor;
	g_iMinor = iMinor;
	g_iBuild = iBuild;
	return 0;
}