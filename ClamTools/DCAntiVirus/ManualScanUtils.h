#pragma once

#include "ManualScanObs.h"
#include "EnumerateFiles.h"

#include "../Utils/SendObj.h"
#include "../Utils/Registry.h"
#include "../Utils/PipeClientUtils.h"

#include <vector>

class CScanItems : public std::vector<CString>
{
public:
	CScanItems(){};
	~CScanItems(){};
};

class CCountFiles : public CEnumerateFiles
{
public:
	CCountFiles(CScanEndingObs *pObs, CScanItems &items):
	  CEnumerateFiles(pObs, true),
	  m_Items(items) {}

public:
	virtual void OnFile(LPCTSTR lpzFile)
	{
		m_Items.push_back(lpzFile);
	}

public:
	CScanItems &m_Items;
};

class CScanOptions
{
public:
	CManualScanObs *m_pObs;
	CScanItems		m_Items;
	bool			m_bUseInternal;
};

namespace manual_scan_utils  
{
	namespace internal
	{
		static void ScanFile(LPCSTR sFile, CManualScanObs *pObs, bool bUseInternal)
		{
			pObs->ShowCurrentItem(sFile);

			CSendObj obj;
			strcpy_s(obj.m_sPath, MAX_PATH, sFile);
			obj.m_nType = EManualScan;
			obj.m_bUseInternalDB = bUseInternal;
			obj.m_PID = GetCurrentProcessId();

			CFileResult result;
			ZeroMemory(&result, sizeof(CFileResult));
			pipe_client_utils::SendFileToPipeServer(sgManualScanServer, &obj, result);

			if(!result.m_bOK)
			{
				CString sVirus = result.m_sVirusName;
				if(!sVirus.IsEmpty())
				{
					pObs->OnVirus(sFile, sVirus);
				}
			}
		}

		static void InitializeManualScanServer()
		{
			CSendObj obj;
			obj.m_nType = EStartManualScan;
			CFileResult result;
			ZeroMemory(&result, sizeof(CFileResult));
			pipe_client_utils::SendFileToPipeServer(sgManualScanServer, &obj, result);
		}

		static void ClearManualScanServer()
		{
			CSendObj obj;
			obj.m_nType = EStopManualScan;
			CFileResult result;
			ZeroMemory(&result, sizeof(CFileResult));
			pipe_client_utils::SendFileToPipeServer(sgManualScanServer, &obj, result);
		}
	}

	static UINT Scan(LPVOID pParam)
	{
		if(NULL != pParam)
		{
			CScanOptions *pOpt = (CScanOptions *)pParam;
			
			if(!pOpt->m_pObs->Continue())
			{
				pOpt->m_pObs->OnFinish("Scan stoped by user.");
				delete pOpt;
				return 0;
			}

			pOpt->m_pObs->OnMessage("Initializing scan server...");
			internal::InitializeManualScanServer();

			typedef CScanItems::const_iterator CIt;
			CIt begin = pOpt->m_Items.begin();
			CIt end = pOpt->m_Items.end();

			pOpt->m_pObs->OnMessage("Scanning...");

			bool bUseInternalDB = pOpt->m_bUseInternal;
			for(CIt it = begin; it != end; ++it)
			{
				internal::ScanFile((*it), pOpt->m_pObs, pOpt->m_bUseInternal);

				if(!pOpt->m_pObs->Continue())
				{
					pOpt->m_pObs->OnFinish("Scan stoped by user.");
					delete pOpt;
					return 0;
				}
			}

			pOpt->m_pObs->OnFinish("Scan completed");

			delete pOpt;

			internal::ClearManualScanServer();
		}

		return 0;
	}
};
