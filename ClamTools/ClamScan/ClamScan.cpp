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

		for(int ci = 0; ci < 3; ++ci)
		{
			const EVP_MD *md = EVP_get_digestbyname(sCipherName[ci].c_str());
			printf("%s\n", sCipherName[ci].c_str());

			for(int fi = 0; fi < 1; ++fi)
			{
				std::wstring sFileName[4];

				
				sFileName[0] = _T("c:\\Sondering\\Sondering\\Bin\\stlportd.5.1.dll");

				//sFileName[0] = _T("c:\\strobist8-1.mkv");
				//sFileName[1] = _T("c:\\Seinfeld-801-The Foundation.avi");
				//sFileName[2] = _T("c:\\WINDOWS\\system32\\shell32.dll");
				//sFileName[3] = _T("c:\\CSAntivirus.log");

				EVP_MD_CTX mdctx;

				unsigned char md_value[EVP_MAX_MD_SIZE];
				unsigned int md_len;
				CFile file(sFileName[fi].c_str(), CFile::modeRead);

				ULONGLONG nSize = file.GetLength();

				char *data = new char[(unsigned int)nSize];
				file.Read(data, (unsigned int)nSize);

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

				delete data;

				double dDiff = (difftime(stop, start) / nCount) * 1000000.0;
				double dSize = (double)nSize / (1024*1024);

				printf("Laikas: %.10f | Failo dydis: %.3f\n", dDiff, dSize);
			}
		}

	}

	return nRetCode;
}
