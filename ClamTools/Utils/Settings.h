#pragma once

#include "Registry.h"

class CSettingsInfo
{
public:
	CSettingsInfo():
	  m_bScan(FALSE),
	  m_bDeny(FALSE),
	  m_bDeep(FALSE),
	  m_bOffice(FALSE),
	  m_bArchives(FALSE),
	  m_bPDF(FALSE),
	  m_bHTML(FALSE),
	  m_bIdle(FALSE),
	  m_nIdleTime(0),
	  m_nCPULoad(0){};

public:
	BOOL m_bScan;
	BOOL m_bDeny;
	BOOL m_bDeep;
	BOOL m_bOffice;
	BOOL m_bArchives;
	BOOL m_bPDF;
	BOOL m_bHTML;

	BOOL m_bIdle;
	int m_nIdleTime;
	int m_nCPULoad;
};

namespace settings_utils
{
	static bool Load(CSettingsInfo &info)
	{
		CString sSettings = registry_utils::GetProfileString(sgSection, sgUserEntry, "");

		if(sSettings.IsEmpty())
		{
			return false;
		}

		sscanf(sSettings, sgSettingsReadTemplate, &info.m_bScan,
												  &info.m_bDeny,
												  &info.m_bDeep,
												  &info.m_bOffice,
												  &info.m_bArchives,
												  &info.m_bPDF,
												  &info.m_bHTML,
												  &info.m_bIdle,
												  &info.m_nIdleTime,
												  &info.m_nCPULoad);

		return true;
	};

	static void Save(const CSettingsInfo &info)
	{
		CString sData;
		sData.Format(sgSettingsWriteTemplate, info.m_bScan,
											  info.m_bDeny,
											  info.m_bDeep,
											  info.m_bOffice,
											  info.m_bArchives,
											  info.m_bPDF,
											  info.m_bHTML,
											  info.m_bIdle,
											  info.m_nIdleTime,
											  info.m_nCPULoad);

		registry_utils::WriteProfileString(sgSection, sgUserEntry, sData);
	}
}