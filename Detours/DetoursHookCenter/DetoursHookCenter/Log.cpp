#include "StdAfx.h"
#include "Log.h"


static const char * const _strPrefix =	"{\\rtf1\\ansi\\deff0\\deftab720{\\fonttbl{\\f0\\fswiss MS Sans Serif;}}"
										"{\\colortbl\\red0\\green0\\blue0;}"
										"\\deflang1033\\pard\\plain\\f0\\fs8\\cf0{";
static const char * const _strPrefixBold = "\\b\n";
static const char * const _strPostfixBold = "\\b0\n";
static const char * const _strPostfix = "}}";

CLog::CLog()
{
	//
}

CLog::~CLog()
{
	//
}

void CLog::AddRichText(LPCSTR sText)
{
	CString str(sText);
	AddText(str);
}

void CLog::AddRichText(LPCWSTR sText)
{
	CString str(sText);
	AddText(str);
}

void CLog::AddRichText(CString &sText)
{
	AddText(sText);
}

void CLog::SetLastPos()
{
	LONG lMax = GetTextLength();
	SetSel(lMax,lMax);
}

void CLog::AddBoldText(LPCSTR sText)
{
	CString str(sText);
	str =_strPrefixBold + str + _strPostfixBold;
	str =_strPrefix +  str + _strPostfix;
	AddText(str);
}

void CLog::AddText(CString &str)
{
	CString s;
	s.Format(_T("%s\n"), str);
	LockWindowUpdate();
	HideSelection(TRUE,FALSE);
	SetLastPos();
	ReplaceSel(s);
	SetSel(0,0);
	HideSelection(FALSE,FALSE);
	LineScroll(2);
	UnlockWindowUpdate();
	UpdateWindow();
}

void CLog::EraseLog()
{
	LONG lMax = GetTextLength();
	SetSel(0, lMax);
	ReplaceSel(_T(""));
}

