#include "stdafx.h"
#include "ClamScan.h"
#include "openssl/evp.h"


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
		OpenSSL_add_all_digests();

		std::string sCipherName[3];
		sCipherName[0] = "md5";
		sCipherName[1] = "sha256";
		sCipherName[2] = "ripemd160";

		std::string sFileName[4];
		
		sFileName[0] = "c:\\strobist8-1.mkv";
		sFileName[1] = "c:\\Seinfeld-801-The Foundation.avi";
		sFileName[2] = "c:\\WINDOWS\\system32\\shell32.dll";
		sFileName[3] = "c:\\Sondering\\Sondering\\Bin\\stlportd.5.1.dll";

		for(int ci = 0; ci < 3; ++ci)
		{
			const EVP_MD *md = EVP_get_digestbyname(sCipherName[ci].c_str());
			printf("%s\n", sCipherName[ci].c_str());

			for(int fi = 0; fi < 4; ++fi)
			{
				EVP_MD_CTX mdctx;

				unsigned char md_value[EVP_MAX_MD_SIZE];
				unsigned int md_len;

				FILE *pFile;
				fopen_s(&pFile, sFileName[fi].c_str(), "rb");
				fseek(pFile, 0L, SEEK_END);
				long lSize = ftell(pFile);
				fseek(pFile, 0L, SEEK_SET);
				
				char *data = (char *)malloc(lSize);
				fread(data, sizeof(char), lSize, pFile);
				fclose(pFile);
	
				int nCount = 2000000;
				time_t start, stop;
				time(&start);
				for(int i = 0; i < nCount; ++i)
				{
					EVP_MD_CTX_init(&mdctx);
					EVP_DigestInit_ex(&mdctx, md, NULL);
					EVP_DigestUpdate(&mdctx, data, strlen(data));
					EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
					EVP_MD_CTX_cleanup(&mdctx);
				}
				time(&stop);

				free((void *)data);

				double dDiff = (difftime(stop, start) / nCount) * 1000000.0;
				double dSize = (double)lSize / (1024*1024);

				printf("Laikas: %.10f | Failo dydis: %.3f\n", dDiff, dSize);
			}
		}

	}

	return nRetCode;
}
