#pragma once

class CLog : public CRichEditCtrl
{
public:
	CLog();
	virtual ~CLog();

public:
	void AddRichText(LPCSTR sText);
	void AddRichText(CString &sText);
	void EraseLog	();
	void AddBoldText(LPCSTR sText);

private:
	void SetLastPos	();
	void AddText(CString &str);
};
