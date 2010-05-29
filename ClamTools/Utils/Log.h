#pragma once

#include "Registry.h"
#include <math.h>
#include <Psapi.h>
#define _LOG_

namespace scan_log_utils
{
	static void Write(LPCSTR sLine)
	{
#ifdef _LOG_
		FILE *pFile = fopen(path_utils::GetScanLogFilePath(), "a+");
		if(NULL == pFile)
		{
			return;
		}

		fprintf(pFile, "%s\n", sLine);
		fflush(pFile);
		fclose(pFile);
#endif
	}

	static void WriteLine(LPCSTR sLine)
	{
#ifdef _LOG_
		CString s;
		s.Format("\t%s", sLine);
		Write(s);
#endif
	}

	static void LogHeader(LPCSTR sData, DWORD PID)
	{
#ifdef _LOG_
		char sOSTime[128];
		_strtime(sOSTime);

		
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, PID);

		CString s;
		if(NULL != hProcess)
		{
			char sProcName[MAX_PATH];
			GetModuleBaseName(hProcess, NULL, sProcName, MAX_PATH);

			s.Format("------- %s %s Process name: %s -------", sOSTime, sData, sProcName);
		}
		else
		{
			s.Format("------- %s %s -------", sOSTime, sData);
		}

		Write(s);
#endif
	}

	static void LogFileSize(LPCSTR sData, double dPar)
	{
#ifdef _LOG_
		CString s;
		s.Format("\t%s: %.2f MB", sData, dPar);
		Write(s);
#endif
	}

	static void LogVirus(LPCSTR sVirus, bool bMain)
	{
#ifdef _LOG_
		CString s;
		if(bMain)
		{
			s.Format("\tVirus found by Main DB: %s", sVirus);
		}
		else
		{
			s.Format("\tVirus found by Daily DB: %s", sVirus);
		}

		Write(s);
#endif
	}

	static void LogTime(LPCSTR sText, double dSec)
	{
#ifdef _LOG_
		int nSec = (int)floor(dSec);
		int hour=nSec/3600;
		nSec=nSec%3600;
		int min=nSec/60;
		nSec=nSec%60;
		int sec=nSec;
		int milisec(0);
		if(0 == (int)floor(dSec))
		{
			milisec = (int)(dSec * 1000);
		}
		else
		{
			milisec = (int)(dSec - (int)floor(dSec)) * 1000;
		}
		CString s;
		s.Format("\t%s: %d:%d:%d:%d", sText, hour, min, sec, milisec);
		//s.Format("\t%s: %.4f", sText, dSec);
		Write(s);
#endif
	}
}

//namespace service_log_utils
//{
//	static void WriteLine(LPCSTR sLine)
//	{
//		FILE *pFile = fopen(path_utils::GetServiceLogFilePath(), "a+");
//		if(NULL == pFile)
//		{
//			return;
//		}
//
//		fprintf(pFile, "%s\n", sLine);
//		fflush(pFile);
//		fclose(pFile);
//	}
//
//	static void LogData(LPCSTR sData)
//	{
//#ifdef _LOG_
//		WriteLine(sData);
//#endif
//	}
//}
