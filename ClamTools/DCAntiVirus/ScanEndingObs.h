#pragma once


//Observer to check if user press stop button when running scan
class CScanEndingObs
{
public:
	virtual bool Continue() = 0;
};