#include "stdafx.h"
#include "EnumerateFiles.h"
#include "ScanEndingObs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CEnumerateFiles::CEnumerateFiles(CScanEndingObs *pObs, bool bPrepare):
m_pObs(pObs),
m_bPrepare(bPrepare)
{

}

CEnumerateFiles::~CEnumerateFiles()
{

}

void CEnumerateFiles::Execute(LPCTSTR lpzDir,LPCTSTR lpzExt, bool bRecurse)
{
	if(!m_pObs->Continue())
	{
		return;
	}

	CFileFind FindAllFile;
	CString sFileName(lpzDir);
	
	CFileFind f;
	if(f.FindFile(sFileName))
	{
		f.FindNextFile();
		if(f.IsDirectory())
		{
			sFileName += "\\*.*";
		}
	}
	else
	{
		sFileName += "*.*";
	}
	
	BOOL bFileExist=FindAllFile.FindFile(sFileName);

	while(bFileExist)
	{
		if(!m_pObs->Continue())
		{
			return;
		}

		bFileExist=FindAllFile.FindNextFile();

		if(FindAllFile.IsDots())
			continue;
		
		if(FindAllFile.IsDirectory() && bRecurse)
		{
			Execute(FindAllFile.GetFilePath(),lpzExt,bRecurse);
			continue;
		}

		if(m_MatchFile.IsMatchExtension(FindAllFile.GetFilePath(),lpzExt))
			OnFile(FindAllFile.GetFilePath());

		if(!m_bPrepare)
		{
			Sleep(50);//Low priority	
		}
	}

	FindAllFile.Close();

	ExecuteDirectory(lpzDir);		
}

void CEnumerateFiles::OnFile(LPCTSTR lpzFile)
{
	TRACE("FileName=%s\n",lpzFile);
}

void CEnumerateFiles::ExecuteDirectory(LPCTSTR lpzDir)
{

}
