#ifndef _SCAN_VALIDATOR_H__
#define _SCAN_VALIDATOR_H__
#pragma once

#ifndef _PDH_H_
	#include <pdh.h>
#endif

#include "../Settings.h"

static LPCTSTR gszProcessorTime="\\Processor(_Total)\\% Processor Time";

class CScanValidator 
{
public:
	CScanValidator()
	{
		m_hQuery = NULL;
		m_hCounter = NULL;
		m_bCounterInit = false;

		if(PdhOpenQuery(NULL, 1, &m_hQuery) == ERROR_SUCCESS)
		{
			if(PdhAddCounter(m_hQuery, gszProcessorTime, NULL, &m_hCounter) == ERROR_SUCCESS)
			{
				m_bCounterInit = TRUE;
			}
		}

		CSettingsInfo info;
		if(settings_utils::Load(info))
		{
			m_nMaxCPULoad	= info.m_nCPULoad;
			m_nIdleTime		= info.m_nIdleTime * 60000;
		}
	}

	virtual ~CScanValidator()
	{
		//
	}
	
	bool ContinueScan()
	{
		LASTINPUTINFO li;
		li.cbSize = sizeof(LASTINPUTINFO);
		GetLastInputInfo(&li);
		int i = li.dwTime;
		UINT timeDuration = GetTickCount() - i;

		if(timeDuration > (UINT)m_nIdleTime)
		{
			return true;
		}

		return false;
	};

	bool IsCPULoaded()
	{
		if(m_bCounterInit)
		{
			long lCPULoad = GetCPUCycle(m_hQuery, m_hCounter);
			return m_nMaxCPULoad < lCPULoad;
		}

		return false;
	}

	LONG GetCPUCycle(HQUERY query, HCOUNTER counter)
	{
		// Collect the current raw data value for all counters in the 
		// specified query and updates the status code of each counter 
		if(PdhCollectQueryData(query) != ERROR_SUCCESS)
		{
			return -1;
		}

		PDH_RAW_COUNTER	ppdhRawCounter;

		// Get the CPU raw counter value	
		if(PdhGetRawCounterValue(counter, NULL, &ppdhRawCounter) == ERROR_SUCCESS)
		{
			PDH_FMT_COUNTERVALUE pdhFormattedValue;
			// Format the CPU counter
			if(PdhGetFormattedCounterValue(counter, PDH_FMT_LONG, NULL, &pdhFormattedValue) == ERROR_SUCCESS)
			{
				return pdhFormattedValue.longValue;
			}
		}

		return -1;
	}

private:
	HQUERY		m_hQuery;
	HCOUNTER	m_hCounter;
	bool		m_bCounterInit;

	int m_nMaxCPULoad;
	int m_nIdleTime;//must be miliseconds
};

#endif
