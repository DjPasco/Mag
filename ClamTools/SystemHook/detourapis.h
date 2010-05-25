HANDLE (WINAPI * pTrueCreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess,
								   DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								   DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
								   HANDLE hTemplateFile) = CreateFileW;

HANDLE WINAPI TransCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,
							   DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							   DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
							   HANDLE hTemplateFile);

HANDLE (WINAPI *pTrueCreateFileA)(LPCSTR lpFileName, DWORD dwDesiredAccess,
								  DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
								  DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
								  HANDLE hTemplateFile) = CreateFileA;

HANDLE WINAPI TransCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess,
							   DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
							   DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
							   HANDLE hTemplateFile);

BOOL (WINAPI *pTrueCreateProcessW)(LPCWSTR lpszImageName,
								   LPWSTR lpszCmdLine,
								   LPSECURITY_ATTRIBUTES lpsaProcess,
								   LPSECURITY_ATTRIBUTES lpsaThread,
								   BOOL fInheritHandles,
								   DWORD fdwCreate,
								   LPVOID lpvEnvironment,
								   LPCWSTR lpszCurDir,
								   LPSTARTUPINFOW lpsiStartInfo,
								   LPPROCESS_INFORMATION lppiProcInfo) = CreateProcessW; 

BOOL WINAPI TransCreateProcessW(LPCWSTR lpszImageName,
								LPWSTR lpszCmdLine,
								LPSECURITY_ATTRIBUTES lpsaProcess,
								LPSECURITY_ATTRIBUTES lpsaThread,
								BOOL fInheritHandles,
								DWORD fdwCreate,
								LPVOID lpvEnvironment,
								LPCWSTR lpszCurDir,
								LPSTARTUPINFOW lpsiStartInfo,
								LPPROCESS_INFORMATION lppiProcInfo); 

BOOL (WINAPI *pTrueCreateProcessA)(LPCSTR lpApplicationName,
								   LPSTR lpCommandLine,
								   LPSECURITY_ATTRIBUTES lpProcessAttributes,
								   LPSECURITY_ATTRIBUTES lpThreadAttributes,
								   BOOL bInheritHandles,
								   DWORD dwCreate,
								   LPVOID lpEnvironment,
								   LPCSTR lpCurrentDirectory,
								   LPSTARTUPINFOA lpStartupInfo,
								   LPPROCESS_INFORMATION lpProcessInformation) = CreateProcessA; 

BOOL WINAPI TransCreateProcessA(LPCSTR lpApplicationName,
								   LPSTR lpCommandLine,
								   LPSECURITY_ATTRIBUTES lpProcessAttributes,
								   LPSECURITY_ATTRIBUTES lpThreadAttributes,
								   BOOL bInheritHandles,
								   DWORD dwCreate,
								   LPVOID lpEnvironment,
								   LPCSTR lpCurrentDirectory,
								   LPSTARTUPINFOA lpStartupInfo,
								   LPPROCESS_INFORMATION lpProcessInformation); 
