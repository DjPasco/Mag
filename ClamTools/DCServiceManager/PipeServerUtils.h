#pragma once

#include "../Utils/npipe.h"
#include "../Utils/Scanner/Scanner.h"
#include "../Utils/SendObj.h"
#include "../Utils/Settings.h"
#include "../Utils/TraySendObj.h"


//Scan pipe server utils
namespace pipe_server_utils
{
	namespace internal
	{
		static void DoAction(CSendObj *pData, CScanner *pScanner, CFileResult &result)
		{

			result.m_bOK = true;
			strcpy_s(result.m_sVirusName, MAX_PATH, "File is clean.");

			if(NULL == pScanner)
			{
				return;
			}

			switch(pData->m_nType)
			{
			case EScan:
				{
					CString sFile = pData->m_sPath;
					CString sVirusName;
					if(!pScanner->ScanFile(sFile, sVirusName, pData->m_PID, result.m_bScanned, true))
					{
						result.m_bOK = false;
						strcpy_s(result.m_sVirusName, MAX_PATH, sVirusName);
					}
				}
				break;
			case EReloadSettings:
				{
					CSettingsInfo info;
					if(settings_utils::Load(info))
					{
						pScanner->SetScanSettings(info.m_bDeep, info.m_bOffice, info.m_bArchives, info.m_bPDF, info.m_bHTML);
						pScanner->SetFilesTypes(info.m_sFilesTypes);
					}
				}
				break;
			case EManualScan:
				{
					int nOldPriority = GetThreadPriority(GetCurrentThread());
					SetThreadPriority(GetCurrentThread(), priority_utils::GetRealPriority(path_utils::GetPriority()));

					registry_utils::WriteProfileString(sgSection, sgVirusName, "");
					CString sFile = pData->m_sPath;
					CString sVirusName;
					bool bClean(true);
					if(pData->m_bUseInternalDB)
					{
						bClean = pScanner->ScanFile(sFile, sVirusName, pData->m_PID, result.m_bScanned, false); 
					}
					else
					{
						bClean = pScanner->ScanFileNoIntDB(sFile, sVirusName, pData->m_PID, result.m_bScanned); 
					}

					SetThreadPriority(GetCurrentThread(), nOldPriority);

					if(!bClean)
					{
						result.m_bOK = false;
						strcpy_s(result.m_sVirusName, MAX_PATH, sVirusName);
					}
				}
				break;
			case EReloadDB:
				{
					pScanner->ReloadDB();
				}
				break;
			case EIdleScan:
				{
					pScanner->ScanFilesForOptimisation();
				}
				break;
			}

			result.m_nFilesCount = pScanner->GetFilesCount();
		}

		bool CreateServer(CNamedPipe &serverPipe, LPCSTR sServerName)
		{
			//Security attributes changed by Wista/Win7 requirements.
			SECURITY_ATTRIBUTES sa;
			sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR)malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
			InitializeSecurityDescriptor(sa.lpSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
			// ACL is set as NULL in order to allow all access to the object.
			SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, NULL, FALSE);
			sa.nLength = sizeof(sa);
			sa.bInheritHandle = TRUE;
			if (!serverPipe.Create(_T(sServerName), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_WAIT, 1, 4096, 4096, 1, &sa))
			{
				return false;
			}

			return true;
		}
	}

	UINT RealScanServer(LPVOID pParam)
	{
		CScannedFileMap *pFilesMap = (CScannedFileMap *)pParam;

		if(NULL == pFilesMap)
		{
			return 0;
		}

		CScanner *pScanner = new CScanner;
		pScanner->LoadDatabases();
		pScanner->SetFilesMap(pFilesMap);

		CNamedPipe serverPipe;
		if(!internal::CreateServer(serverPipe, sgScanServer))
		{
			delete pScanner;
			return 0;
		}

		while (1)
		{
			if (!serverPipe.ConnectClient())
			{
				continue;
			}

			CSendObj obj;
			DWORD dwBytes;
			if (serverPipe.Read(&obj, sizeof(CSendObj), dwBytes, NULL))
			{
				if(EQuitServer == obj.m_nType)
				{
					serverPipe.DisconnectClient();
					break;
				}

				if(obj.m_nType == ERequest)
				{
					CTrayRequestData data;
					ZeroMemory(&data, sizeof(CTrayRequestData));
					pScanner->RequestData(data);
					serverPipe.Write(&data, sizeof(CTrayRequestData), dwBytes);
				}
				else
				{
					CFileResult result;
					ZeroMemory(&result, sizeof(CFileResult));

					internal::DoAction(&obj, pScanner, result);

					serverPipe.Write(&result, sizeof(CFileResult), dwBytes);
				}
			}

			if (!serverPipe.DisconnectClient())
			{
				continue;
			}
		}

		delete pScanner;

		return 0;
	};

	UINT ManualScanServer(LPVOID pParam)
	{
		CScannedFileMap *pFilesMap = (CScannedFileMap *)pParam;

		if(NULL == pFilesMap)
		{
			return 0;
		}

		CScanner *pScanner = NULL;

		CNamedPipe serverPipe;
		if(!internal::CreateServer(serverPipe, sgManualScanServer))
		{
			return 0;
		}

		while (1)
		{
			if (!serverPipe.ConnectClient())
			{
				continue;
			}

			CSendObj obj;
			DWORD dwBytes;
			if (serverPipe.Read(&obj, sizeof(CSendObj), dwBytes, NULL))
			{
				if(EStartManualScan == obj.m_nType)
				{
					if(NULL == pScanner)
					{
						pScanner = new CScanner;
						pScanner->LoadDatabases();
						pScanner->SetFilesMap(pFilesMap);
					}

					serverPipe.DisconnectClient();
					continue;
				}

				if(EQuitServer == obj.m_nType)
				{
					serverPipe.DisconnectClient();
					break;
				}

				if(EStopManualScan == obj.m_nType)
				{
					serverPipe.DisconnectClient();
					delete pScanner;
					pScanner = NULL;
					continue;
				}

				if(EManualScan != obj.m_nType)
				{
					serverPipe.DisconnectClient();
					delete pScanner;
					pScanner = NULL;
					continue;
				}

				CFileResult result;
				ZeroMemory(&result, sizeof(CFileResult));

				internal::DoAction(&obj, pScanner, result);

				serverPipe.Write(&result, sizeof(CFileResult), dwBytes);
			}

			serverPipe.DisconnectClient();
		}

		delete pScanner;

		return 0;
	};
}