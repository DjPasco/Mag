// ForceVersion.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <detours.h>
#include <tchar.h>
#include <stdlib.h>
#include "ForceVersion.h"

BOOL FormPath(TCHAR *pBuffer, TCHAR *pFolder, TCHAR *pFilename)
{
	if (!pBuffer || !pFolder || !pFilename)
		return FALSE;

	// Init the buffer
	_tcsset(pBuffer, 0);
	_tcscpy(pBuffer, pFolder);
	if (pFolder[_tcslen(pFolder)-1] != _TCHAR('\\'))
	{
		_tcscat(pBuffer, _TEXT("\\"));
	}

	_tcscat(pBuffer, pFilename);
	_tcscat(pBuffer,_TEXT("\0"));

	// Formed the path.. success
	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{

	// This sample will take path to the executable which will invoke GetVersionEx
	// so that our detoured function will return them our custom version details
	if (argc != 5)
	{
		// Usage is:
		//
		// ForceVersion <path> <major ver> <minor ver> <build #>
		//
		// For e.g.
		//
		// ForceVersion c:\app.exe 4 1 2000
		// 
		// will return OS version as 4.1.2000 when the application calls GetVersionEx API

		printf("ForceVersion - returns a user defined OS version details to application\n");
		printf("by Gaurav Khanna - http://www.wintoolzone.com/\n\n");
		printf("Usage:\n\nForceVersion <app path> <major ver> <minor ver> <build #>\nwhere:\n");
		printf("\t<app path> - path to the executable which will request OS version\n");
		printf("\t<major ver> - OS major version to be returned to calling applications\n");
		printf("\t<minor ver> - OS minor version to be returned to calling applications\n");
		printf("\t<build #> - OS build version to be returned to calling applications\n");
		return 0;
	}

	// Get the version details
	VersionPayLoad payload;
	payload.iMajor = atoi(argv[2]);
	payload.iMinor = atoi(argv[3]);
	payload.iBuild = atoi(argv[4]);
	
	// Now, we will ask Detours to launch this executable and load our DetourDLL into the process
	STARTUPINFO startup;
	memset(&startup, 0, sizeof(startup));
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));
	startup.cb = sizeof(startup);
	
	// init the buffer that will hold the current working folder of the application
	TCHAR tCurDir[MAX_PATH];
	memset(tCurDir,0, sizeof(tCurDir));
	DWORD dwCount = GetCurrentDirectory(MAX_PATH,tCurDir);
	if (!dwCount)
	{
		printf("Unable to get the working folder!");
		return -1;
	}

	// Form the paths to the DETOURED.DLL and our Detouring DLL
	TCHAR szDetouredPath[MAX_PATH]; 
	memset(szDetouredPath, 0, sizeof(szDetouredPath));

	TCHAR szInjectDLLPath[MAX_PATH]; 
	memset(szInjectDLLPath, 0, sizeof(szInjectDLLPath));
	
	if (FormPath(szDetouredPath, tCurDir, _TEXT("detoured.dll")) == FALSE)
	{
		printf("Unable to form path to DETOURED.DLL!\n");
		return -1;
	}

	if (FormPath(szInjectDLLPath, tCurDir, _TEXT("ForceVersionDLL.dll")) == FALSE)
	{
		printf("Unable to form path to Injection DLL!\n");
		return -1;
	}

	// We create the process as suspended since we will copy payload
	// to the target process containing the versions we want it to get
	BOOL fLaunchApp = DetourCreateProcessWithDll(argv[1],
		0,0,0,CREATE_SUSPENDED|CREATE_DEFAULT_ERROR_MODE,0,0,0,&startup,&pi,
		szDetouredPath,
		szInjectDLLPath,
		NULL);
	
	if (!fLaunchApp)
	{
		printf("Error: %d\n",GetLastError());
		return -1;
	}

	// Send the payload data...
	BOOL fRetVal = DetourCopyPayloadToProcess(pi.hProcess, my_guid, (PVOID)&payload, sizeof(payload));
	if (!fRetVal)
	{
		printf("Unable to write version information in the target process!");
		return -1;
	}

	// Resume thread and wait on the process..
	ResumeThread(pi.hThread);

	WaitForSingleObject(pi.hProcess, INFINITE);
	return 0;
}

