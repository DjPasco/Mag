#include "stdafx.h"
#include "FileUsage.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		time_t start, stop;
		int nCount = 20000;

		time(&start);
		
		for(int i = 0; i < nCount; ++i)
		{
			CFile file(_T("Test.txt"), CFile::modeCreate);
		}

		time(&stop);

		double dDiff = difftime(stop, start);
		printf("Finished in about %.5f seconds. \n", dDiff);
		double dOneFile = dDiff / nCount;
		printf("Finished in about %.5f seconds. \n", dOneFile);
		int k;
		cin>>k;
	}

	return nRetCode;
}
