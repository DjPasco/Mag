#pragma once

static const char *sgSettingsWriteTemplate = "%d %d %d %d %d %d %d %d %d %d";
static const char *sgSettingsReadTemplate = "%d%d%d%d%d%d%d%d%d%d";
static const char *sgSection = "Settings";
static const char *sgUserEntry = "ScanOptions";

static const char *sgShedScanTaskName = "DCAntiVirusScan";
static const char *sgShedScanTaskInfo = "SheduledScanInfo";
static const char *sgShedScanItems	  = "ItemsToScan";
static const char *sgFileTypes		  = "FileTypes";

static const char *sgShedUpdTaskName = "DCAntiVirusUpdate";
static const char *sgShedUpdTaskInfo = "SheduledUpdateInfo";

static const char *sgAppName = "DCAntiVirus";

static const char *sgBaseDir = "DCAntiVirusBaseDir";

static const char *sgDBDir		= "DB";
static const char *sgDBMain		= "main.cvd";
static const char *sgDBDaily	= "daily.cvd";

static const char *sgDataFile	= "PassData.dat";

static const char *sgFreshClam		= "freshclam.exe";
static const char *sgFreshClamConf	= "freshclam.conf";

static const char *sgServiceName		= "DCAntiVirus";
static const char *sgServiceDisplayName = "DCAntiVirus";
static const char *sgServiceDescription = "DCAntiVirus: Protection against Virus";

static const char *sgVirusName = "VirusName";

static const char *sgLogFileName = "Log.txt";

static const char *sgServerName = "DCAntiVirusScan";

namespace registry_utils
{
	namespace internal 
	{
		static HKEY GetAppRegistryKey()
		{
			HKEY hAppKey = NULL;
			HKEY hSoftKey = NULL;
			HKEY hCompanyKey = NULL;
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("software"), 0, KEY_WRITE|KEY_READ,
				&hSoftKey) == ERROR_SUCCESS)
			{
				DWORD dw;
				if (RegCreateKeyEx(hSoftKey, sgAppName, 0, REG_NONE,
					REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
					&hCompanyKey, &dw) == ERROR_SUCCESS)
				{
					RegCreateKeyEx(hCompanyKey, sgAppName, 0, REG_NONE,
						REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
						&hAppKey, &dw);
				}
			}
			if (hSoftKey != NULL)
				RegCloseKey(hSoftKey);
			if (hCompanyKey != NULL)
				RegCloseKey(hCompanyKey);

			return hAppKey;
		};

		static HKEY GetSectionKey(LPCTSTR lpszSection)
		{
			ASSERT(lpszSection != NULL);

			HKEY hSectionKey = NULL;
			HKEY hAppKey = GetAppRegistryKey();
			if (hAppKey == NULL)
				return NULL;

			DWORD dw;
			RegCreateKeyEx(hAppKey, lpszSection, 0, REG_NONE,
				REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
				&hSectionKey, &dw);
			RegCloseKey(hAppKey);
			return hSectionKey;
		}
	};

	static CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
	{
		ASSERT(lpszSection != NULL);
		ASSERT(lpszEntry != NULL);

		HKEY hSecKey = internal::GetSectionKey(lpszSection);
		if (hSecKey == NULL)
			return lpszDefault;
		CString strValue;
		DWORD dwType=REG_NONE;
		DWORD dwCount=0;
		LONG lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
			NULL, &dwCount);
		if (lResult == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_SZ);
			lResult = RegQueryValueEx(hSecKey, (LPTSTR)lpszEntry, NULL, &dwType,
				(LPBYTE)strValue.GetBuffer(dwCount/sizeof(TCHAR)), &dwCount);
			strValue.ReleaseBuffer();
		}
		RegCloseKey(hSecKey);
		if (lResult == ERROR_SUCCESS)
		{
			ASSERT(dwType == REG_SZ);
			return strValue;
		}
		return lpszDefault;
	};

	static BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
	{
		ASSERT(lpszSection != NULL);

		LONG lResult;
		if (lpszEntry == NULL) //delete whole section
		{
			HKEY hAppKey = internal::GetAppRegistryKey();
			if (hAppKey == NULL)
				return FALSE;
			lResult = ::RegDeleteKey(hAppKey, lpszSection);
			RegCloseKey(hAppKey);
		}
		else if (lpszValue == NULL)
		{
			HKEY hSecKey = internal::GetSectionKey(lpszSection);
			if (hSecKey == NULL)
				return FALSE;
			// necessary to cast away const below
			lResult = ::RegDeleteValue(hSecKey, (LPTSTR)lpszEntry);
			RegCloseKey(hSecKey);
		}
		else
		{
			HKEY hSecKey = internal::GetSectionKey(lpszSection);
			if (hSecKey == NULL)
				return FALSE;
			lResult = RegSetValueEx(hSecKey, lpszEntry, NULL, REG_SZ,
				(LPBYTE)lpszValue, (lstrlen(lpszValue)+1)*sizeof(TCHAR));
			RegCloseKey(hSecKey);
		}
		return lResult == ERROR_SUCCESS;
	};

	static void CheckBaseDir()
	{
		CString sBaseDir = GetProfileString(sgSection, sgBaseDir, "");
		if(sBaseDir.IsEmpty())
		{
			char dirPath[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, dirPath);
			WriteProfileString(sgSection, sgBaseDir, dirPath);
		}
	}
}

namespace path_utils
{
	static CString GetDailyDBPath()
	{
		CString sBaseDir = registry_utils::GetProfileString(sgSection, sgBaseDir, "");
		CString sPath;
		sPath.Format("%s\\%s\\%s", sBaseDir, sgDBDir, sgDBDaily);
		return sPath;
	}

	static CString GetMainDBPath()
	{
		CString sBaseDir = registry_utils::GetProfileString(sgSection, sgBaseDir, "");
		CString sPath;
		sPath.Format("%s\\%s\\%s", sBaseDir, sgDBDir, sgDBMain);
		return sPath;
	}

	static CString GetDataFilePath()
	{
		CString sBaseDir = registry_utils::GetProfileString(sgSection, sgBaseDir, "");
		CString sPath;
		sPath.Format("%s\\%s", sBaseDir, sgDataFile);
		return sPath;
	}

	static CString GetDBDir()
	{
		CString sBaseDir = registry_utils::GetProfileString(sgSection, sgBaseDir, "");
		CString sPath;
		sPath.Format("%s\\%s", sBaseDir, sgDBDir);
		return sPath;
	}

	static CString GetFreshClamPath()
	{
		CString sBaseDir = registry_utils::GetProfileString(sgSection, sgBaseDir, "");
		CString sPath;
		sPath.Format("%s\\%s", sBaseDir, sgFreshClam);
		return sPath;
	}

	static CString GetFreshClamConfPath()
	{
		CString sBaseDir = registry_utils::GetProfileString(sgSection, sgBaseDir, "");
		CString sPath;
		sPath.Format("%s\\%s", sBaseDir, sgFreshClamConf);
		return sPath;
	}

	static CString GenerateFrechClamParameters()
	{
		CString sFreshClamPathConf	= path_utils::GetFreshClamConfPath();
		CString sDBDir				= path_utils::GetDBDir();

		CString sParameters;
		sParameters.Format("--datadir=\"%s\" --config-file=\"%s\"", sDBDir, sFreshClamPathConf);

		return sParameters;
	}

	static CString GetLogFilePath()
	{
		CString sBaseDir = registry_utils::GetProfileString(sgSection, sgBaseDir, "");
		CString sPath;
		sPath.Format("%s\\%s", sBaseDir, sgLogFileName);
		return sPath;
	}
}