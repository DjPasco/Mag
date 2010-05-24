#ifndef _LOG_H__
#define _LOG_H__

#include "Registry.h"

namespace log_utils
{
	void LogData(LPCSTR sData)
	{
		sData;
#ifdef _DEBUG
		FILE *pFile = fopen(path_utils::GetLogFilePath(), "a+");
		if(NULL == pFile)
		{
			return;
		}

		fprintf(pFile, "%s\n", sData);

		fflush(pFile);
		
		fclose(pFile);
#endif
	}
}

#endif