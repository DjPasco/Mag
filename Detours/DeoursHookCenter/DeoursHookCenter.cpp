#include "stdafx.h"
#include "DeoursHookCenter.h"
#include "HookUtils/HookUtils.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		if(3 != argc)
		{
			return nRetCode;
		}

		std::wstring sCommand = argv[1];
		std::wstring sFileName = argv[2];

		if(0 == sCommand.compare(_T("hook")))
		{
			hook_utils::LoadNotepadWithHookDll(sFileName);
		}
		else if (0 == sCommand.compare(_T("run")))
		{
			std::wstring sExe = hook_utils::GetExePath(sFileName);

			STARTUPINFO info={sizeof(info)};
			PROCESS_INFORMATION processInfo;
			if (CreateProcess(sExe.c_str(), _T(""), NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
			{
				::WaitForSingleObject(processInfo.hProcess, INFINITE);
				CloseHandle(processInfo.hProcess);
				CloseHandle(processInfo.hThread);
			}
		}
	}

	return nRetCode;
}
