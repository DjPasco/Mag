#ifndef _LOG_H__
#define _LOG_H__

#include "Registry.h"

#define _LOG_

namespace scan_log_utils
{
	static void WriteLine(LPCSTR sLine)
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

	static void LogData(LPCSTR sData)
	{
#ifdef _LOG_
		WriteLine(sData);
#endif
	}

	static void LogParameter(LPCSTR sData, double dPar)
	{
#ifdef _LOG_
		CString s;
		s.Format("%s: %.2f", sData, dPar);
		WriteLine(s);
#endif
	}

	static void LogTime(LPCSTR sText, CTime tBegin, CTime tEnd)
	{
#ifdef _LOG_
		CTimeSpan span = tEnd - tBegin;
		CString sLine;
		sLine.Format("%s: %d:%d:%d", sText, span.GetHours(), span.GetMinutes(), span.GetSeconds());
		WriteLine(sLine);
#endif
	}
}

namespace service_log_utils
{
	static void WriteLine(LPCSTR sLine)
	{
		FILE *pFile = fopen(path_utils::GetServiceLogFilePath(), "a+");
		if(NULL == pFile)
		{
			return;
		}

		fprintf(pFile, "%s\n", sLine);
		fflush(pFile);
		fclose(pFile);
	}

	static void LogData(LPCSTR sData)
	{
#ifdef _LOG_
		WriteLine(sData);
#endif
	}
}

#endif