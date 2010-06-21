#ifndef _PRECISION_TIMER_H__
#define _PRECISION_TIMER_H__
#pragma once


//class to handle miliseconds timer.
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
