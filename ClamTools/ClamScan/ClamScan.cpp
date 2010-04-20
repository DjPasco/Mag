#include "stdafx.h"
#include "ClamScan.h"
#include "../../LibClamAV/clamav.h"

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
		int nRet = cl_init(CL_INIT_DEFAULT);
		if(CL_SUCCESS != nRet)
		{
			return 1;
		}


		cl_engine *pEngine = cl_engine_new();
		if(NULL == pEngine)
		{
			ASSERT(FALSE);
			return 1;
		}

		LPCSTR sPath = "c:\\MAG_REPO\\LibClamAV\\daily.cvd";

		printf("Loading Database...\n");

		unsigned int nSignCount = 0;
		time_t start, stop;

		time(&start);

		nRet = cl_load(sPath, pEngine, &nSignCount, CL_DB_BYTECODE);

		time(&stop);
		double dDiff = difftime(stop, start);
		printf("Duombazes loadinimo laikas: %.5f s. \n", dDiff);

		if(CL_SUCCESS != nRet)
		{
			return 1;
		}

		nRet = cl_engine_compile(pEngine);
		if(CL_SUCCESS != nRet)
		{
			return 1;
		}

		//cl_debug();

		printf("Scanning.\n");
		LPCSTR sFile = "c:\\WINDOWS\\system32\\shell32.dll";
		int nCount = 10;

		time(&start);
		//for(int i = 0; i < nCount; ++i)
		{
			const char *sVirname;
			unsigned long nScanned;
			cl_scanfile(sFile, &sVirname, &nScanned, pEngine, /*CL_SCAN_RAW*/CL_SCAN_STDOPT);
			//sFile = "c:\\WINDOWS\\regedit.exe";	
			//cl_scanfile(sFile, &sVirname, &nScanned, pEngine, CL_SCAN_RAW);
			//printf("%d\n", i);
		}
		time(&stop);

		dDiff = difftime(stop, start);
		printf("Bendras laikas     %.5f s. \n", dDiff);
		double dOneFile = dDiff / nCount;
		printf("Failo skenavimo laikas %.5f s. \n\n", dOneFile);

		printf("Scan ends.\n");

		nRet = cl_engine_free(pEngine);
		if(CL_SUCCESS != nRet)
		{
			return 1;
		}
	}

	return nRetCode;
}
