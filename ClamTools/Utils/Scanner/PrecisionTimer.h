#ifndef _PRECISION_TIMER_H__
#define _PRECISION_TIMER_H__
#pragma once

class CPrecisionTimer
{
	LARGE_INTEGER lFreq, lStart;

public:
	CPrecisionTimer()
	{
		QueryPerformanceFrequency(&lFreq);
	}

	inline void Start()
	{
		QueryPerformanceCounter(&lStart);
	}

	inline double Stop()
	{
		// Return duration in seconds...
		LARGE_INTEGER lEnd;
		QueryPerformanceCounter(&lEnd);
		return (double(lEnd.QuadPart - lStart.QuadPart) / lFreq.QuadPart);
	}
};

#endif
