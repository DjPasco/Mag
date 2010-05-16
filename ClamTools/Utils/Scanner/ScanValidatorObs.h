#ifndef _SCAN_VALIDATOR_OBS_H__
#define _SCAN_VALIDATOR_OBS_H__
#pragma once

class CScanValidatorObs  
{
public:
	virtual bool ContinueScan() = 0;
	virtual bool IsCPULoaded()	= 0;
};

#endif
