#pragma once

class CManualScanObs  
{
public:
	virtual void ShowCurrentItem(LPCSTR sItem) = 0;
	virtual void OnVirus(LPCSTR sItem, LPCSTR sVirus) = 0;
	virtual bool Continue() = 0;
	virtual void OnFinish(LPCSTR sFinishText) = 0;
	virtual void OnMessage(LPCSTR sMessage) = 0;
};
