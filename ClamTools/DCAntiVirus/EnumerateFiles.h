#pragma once

#include "MatchFileExtension.h"

class CScanEndingObs;

class CEnumerateFiles  
{
public:
	virtual void Execute(LPCTSTR lpzDir,LPCTSTR lpzExt, bool bRecurse=true);
	CEnumerateFiles(CScanEndingObs *pObs);
	virtual ~CEnumerateFiles();
protected:
	CMatchFileExtension m_MatchFile;
	virtual void ExecuteDirectory(LPCTSTR lpzDir);
	virtual void OnFile(LPCTSTR lpzFile)=0;

private:
	CScanEndingObs *m_pObs;	
};