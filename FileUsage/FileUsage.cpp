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

	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		printf("Klaida: MFC nestartavo.\n");
		nRetCode = 1;
	}
	else
	{
		for(int k = 0; k < 10; k++)
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
			printf("Bendras laikas     %.5f s. \n", dDiff);
			double dOneFile = dDiff / nCount;
			printf("Vieno failo laikas %.5f s. \n\n", dOneFile);
		}
	}

	return nRetCode;
}
