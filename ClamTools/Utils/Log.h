#pragma once

#include "Registry.h"
#include "Settings.h"
#include <math.h>
#include <Psapi.h>

namespace scan_log_utils
{
	static void Write(LPCSTR sLine)
	{
		CSettingsInfo info;
		settings_utils::Load(info);
		if(info.m_bLog)
		{
			FILE *pFile = fopen(path_utils::GetScanLogFilePath(), "a+");
			if(NULL == pFile)
			{
				return;
			}

			fprintf(pFile, "%s\n", sLine);
			fflush(pFile);
			fclose(pFile);
		}
	}

	static void WriteLine(LPCSTR sLine)
	{
		CString s;
		s.Format("\t%s", sLine);
		Write(s);
	}

	static void LogHeader(LPCSTR sData, DWORD PID)
	{
		char sOSTime[128];
		_strtime(sOSTime);

		
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, PID);

		CString s;
		if(NULL != hProcess)
		{
			char sProcName[MAX_PATH];
			GetModuleBaseName(hProcess, NULL, sProcName, MAX_PATH);

			s.Format("------- %s -- %s -- Process name: %s -------", sOSTime, sData, sProcName);
		}
		else
		{
			s.Format("------- %s -- %s -------", sOSTime, sData);
		}

		Write(s);
	}

	static void LogFileSize(LPCSTR sData, double dPar)
	{
		CString s;
		s.Format("\t%s: %.2f MB", sData, dPar);
		Write(s);
	}

	static void LogInt(LPCSTR sData, int nValue)
	{
		CString s;
		s.Format("\t%s: %d", sData, nValue);
		Write(s);
	}

	static void LogVirus(LPCSTR sVirus, bool bMain)
	{
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
	}

	static void LogTime(LPCSTR sText, double dSec)
	{
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
		Write(s);
	}
}

namespace hook_log_utils
{
	static void Write(LPCSTR sLine)
	{
		CSettingsInfo info;
		settings_utils::Load(info);
		if(info.m_bLog)
		{
			FILE *pFile = fopen(path_utils::GetHookLogFilePath(), "a+");
			if(NULL == pFile)
			{
				return;
			}

			fprintf(pFile, "%s\n", sLine);
			fflush(pFile);
			fclose(pFile);
		}
	}

	static void WriteLine(LPCSTR sLine)
	{
		CString s;
		s.Format("\t%s", sLine);
		Write(s);
	}

	static void LogHeader(LPCSTR sData, DWORD PID)
	{
		char sOSTime[128];
		_strtime(sOSTime);

		
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, PID);

		CString s;
		if(NULL != hProcess)
		{
			char sProcName[MAX_PATH];
			GetModuleBaseName(hProcess, NULL, sProcName, MAX_PATH);

			s.Format("------- %s -- %s -- Process name: %s -------", sOSTime, sData, sProcName);
		}
		else
		{
			s.Format("------- %s -- %s -------", sOSTime, sData);
		}

		Write(s);
	}

	static void LogString(LPCSTR sData, LPCSTR sValue)
	{
		CString s;
		s.Format("\t%s: %s", sData, sValue);
		Write(s);
	}

	static void LogString(LPCSTR sData, LPVOID sValue)
	{
		CString s;
		s.Format("\t%s: %s", sData, sValue);
		Write(s);
	}
}
