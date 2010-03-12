// GetVersionClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <tchar.h>

int _tmain(int argc, _TCHAR* argv[])
{
	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(osinfo);
	BOOL fGotVersion = GetVersionEx(&osinfo);
	printf("OS Version: %d.%d\n",osinfo.dwMajorVersion, osinfo.dwMinorVersion);
	printf("Build: %d\n",osinfo.dwBuildNumber);
	printf("Platform ID: %d\n",osinfo.dwPlatformId);
	_tprintf(L"Description: %s\n",osinfo.szCSDVersion);
	return 0;
}

