#pragma once

//File match checker.
class CMatchFileExtension  
{
	CString GetFileExtension(LPCTSTR lpzFile);
	CStringArray m_sArrayExt;
	char m_ctemp[1024];
	CString m_sCompare;
public:
	virtual bool IsMatchExtension(LPCTSTR lpzFile,LPCTSTR lpzExt);
	CMatchFileExtension();
	virtual ~CMatchFileExtension();

};
