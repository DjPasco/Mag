#include "stdafx.h"
#include "MatchFileExtension.h"
#include "../Utils/Registry.h"

CMatchFileExtension::CMatchFileExtension()
{

}

CMatchFileExtension::~CMatchFileExtension()
{

}

bool CMatchFileExtension::IsMatchExtension(LPCTSTR lpzFile, LPCTSTR lpzExt)
{
	strcpy(m_ctemp,lpzExt);
	char *token;
	token = strtok(m_ctemp, sgFileExtSeparator);
	
	m_sArrayExt.RemoveAll();
		
	while(token!=NULL)
	{
		m_sArrayExt.Add(token);
	    token = strtok(NULL, sgFileExtSeparator);
	}
	bool bFound=false;

	for (int i=0;i<m_sArrayExt.GetSize();i++) 
	{
		m_sCompare=m_sArrayExt[i];
		if(!m_sCompare.CompareNoCase("*.*"))
		{
			bFound=true;
			break;
		}

		m_sCompare=GetFileExtension(m_sCompare);
		if(!m_sCompare.CompareNoCase(GetFileExtension(lpzFile)))
		{
			bFound=true;
			break;
		}
	}
	return bFound;

}

CString CMatchFileExtension::GetFileExtension(LPCTSTR lpzFile)
{
	CString sFile(lpzFile);
	CString sExtension;
	int i=sFile.ReverseFind('.');
	sExtension=sFile.Mid(i+1);
//	TRACE("Extension=%s\n",sExtension);
	
	return sExtension;

}
