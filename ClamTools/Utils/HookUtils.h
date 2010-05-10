#ifndef _HOOK_UTILS_MAIN_H__
#define _HOOK_UTILS_MAIN_H__

namespace hook_utils_main
{
	void RunLoadLibraryInProcess(HANDLE hProcess, LPVOID LoadLibraryAddr, char *sDLLPath)
	{
		LPVOID LLParam = (LPVOID)VirtualAllocEx(hProcess, NULL, strlen(sDLLPath), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		WriteProcessMemory(hProcess, LLParam, sDLLPath, strlen(sDLLPath), NULL);
		HANDLE hLoadLibrary = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryAddr, LLParam, NULL, NULL);
		WaitForSingleObject(hLoadLibrary, INFINITE);
	}

	void DebugMessage(char *sInfo)
	{
		MessageBox(NULL, sInfo, "InfoMessage", MB_OKCANCEL);
	}
}
#endif