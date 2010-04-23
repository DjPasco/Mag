#include "stdafx.h"
#include "Scanner.h"

#include "DCSanner\DCSanner.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CScanner::CScanner()
{
	Init();
	CDCScanner::CreateEngine();

	m_FilesMap.clear();

	m_FilesMap["as"] = "kaszkas";

	CString sValue = m_FilesMap["as"]; 
}

CScanner::~CScanner()
{
	Free();
}

bool CScanner::LoadDatabases()
{
	return CDCScanner::LoadDatabases();
}

void CScanner::Init()
{
	if(!CDCScanner::Init())
	{
		//
	}
}

void CScanner::Free()
{
	CDCScanner::FreeEngine();
}

bool CScanner::ScanFile(LPCSTR sFile, CString &sVirus)
{
	const char *sVirname = NULL;
	if(CDCScanner::ScanFile(sFile, sVirname))
	{
		sVirus.Format("&s", sVirname);
		return true;
	}

	sVirus.Empty();
	return false;
}