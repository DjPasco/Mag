#include "stdafx.h"
#include "XFileMonitor.Hook.h"
#include "..\detours\detours.h" // Detours header
#include <map>	// std::map
#include <atlstr.h> // for CString
using namespace std;

// Just a way for us to track what the client did, eg open/close/write to a file
enum FileAction
{
    File_Unknown = 0,
    File_CreateAlways,
    File_CreateNew,
    File_OpenAlways,
    File_OpenExisting,
    File_TruncateExisting,

    File_Write,
    File_Read,
    File_Close
};

// Just a way for us to track which API was called..
enum Win32API
{
    API_CreateFile = 1,
    API_WriteFile,
    API_ReadFile,
    API_CloseHandle
};

// This is the structure we'll send over to the GUI via WM_COPYDATA
struct FILECONTEXT
{
    HANDLE File;
    __int32 OriginalAPI;
};

map<HANDLE, CString> g_openFiles;

CRITICAL_SECTION g_CritSec;			// Guards access to the collection (thread synch)

HINSTANCE   g_hInstance = NULL;     // This instance

// Function pointer to the original (undetoured) WriteFile API.
HANDLE (WINAPI * Real_CreateFile)
(LPCWSTR lpFileName,DWORD dwDesiredAcces, DWORD dwShareMode,
 LPSECURITY_ATTRIBUTES lpSecurityAttributes,DWORD dwCreationDisposition,
 DWORD dwFlagsAndAttributes,HANDLE hTemplateFile) = CreateFileW;

// Function pointer to the original (undetoured) WriteFile API.
BOOL (WINAPI * Real_WriteFile)
(HANDLE hFile,LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
 LPDWORD lpNumberOfBytesWritten,LPOVERLAPPED lpOverlapped) = WriteFile;

// Function pointer to the original (undetoured) ReadFile API.
BOOL (WINAPI *Real_ReadFile)
(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,
LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped) = ReadFile;

// Function pointer to the original (undetoured) CloseHandle API.
BOOL (WINAPI * Real_CloseHandle)(HANDLE hObject) = CloseHandle;

// Transfer some textual data over to the GUI
void Transmit(FileAction action, Win32API sourceAPI, HANDLE hFile, LPCWSTR text)
{
    HWND hWnd = ::FindWindow(NULL, L"Coding the Wheel - XFileMonitor v1.0");
    if (!hWnd)
        return;

    COPYDATASTRUCT cds;
    ::ZeroMemory(&cds, sizeof(COPYDATASTRUCT));
    cds.dwData = action;
    cds.cbData = sizeof(FILECONTEXT) + ((wcslen(text)+1) * 2);

    // Allocate the outgoing array
    LPBYTE pOutData = new BYTE[cds.cbData];
    // Place a HANDLEANDTEXT structure at the front of the array
    FILECONTEXT ht;
    ht.File = hFile;
    ht.OriginalAPI = sourceAPI;
    memcpy(pOutData, &ht, sizeof(FILECONTEXT));
    // Place the text immediately following the structure
    wcscpy((LPWSTR)(pOutData + sizeof(FILECONTEXT)), text);
    // Send it off
    cds.lpData = pOutData;
    ::SendMessage(hWnd, WM_COPYDATA, (WPARAM)::GetDesktopWindow(), (LPARAM)&cds);
    delete [] pOutData;
}

// Check the first N bytes of data to see if they fall within printable ranges
// for Unicode character data. Quick and dirty and needs work.
bool IsUnicodeText(LPCWSTR buffer, int testLength)
{
    int validChars = 0;
    for (int index = 0; index < testLength; index++)
    {
		wchar_t c = buffer[index];

        if (c < 0 || !(iswprint(c) || iswspace(c)))
            return false;
    }

    return true;
}

// Check the first N bytes of data to see if they fall within printable ranges
// for ASCII/MBCS character data. Quick and dirty.
bool IsAsciiText(LPCSTR buffer, int testLength)
{
    int validChars = 0;
    for (int index = 0; index < testLength; index++)
    {
		char c = buffer[index];

        if (c < 0 || !(isprint(c) || isspace(c)))
            return false;
    }

    return true;
}

// Figure out if this is a binary file based on some common binary file extensions
// Quick and dirty.
bool IsBinaryFile(CString& file)
{
    int indexOfExtension = file.ReverseFind(L'.');
    if (indexOfExtension > -1)
    {
        CString sExt = file.Mid(indexOfExtension);
        return (sExt == L".dll" || sExt == L".exe" || sExt == L".bmp" ||
            sExt == L".png" || sExt == L".gif" || sExt == L".ico" ||
            sExt == L".jpg" || sExt == L".jpeg" || sExt == L".mp3" ||
            sExt == L".drv" || sExt == L".wmp" || sExt == L".ra");
    }

    return false;
}

// Our custom version of the CreateFile Windows API.
// http://msdn.microsoft.com/en-us/library/aa363858.aspx
HANDLE WINAPI Mine_CreateFile(LPCWSTR lpFileName,DWORD dwDesiredAccess,
                              DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecAttr,
                              DWORD dwCreateDisp, DWORD dwFlagsAttr,HANDLE hTemplate)
{
	// First, all the original CreateFile provided by the operating system.
    HANDLE hFile = Real_CreateFile(lpFileName,dwDesiredAccess,dwShareMode,lpSecAttr,
                                    dwCreateDisp,dwFlagsAttr,hTemplate);

    if (lpFileName && hFile)
    {
        CString sFileName = lpFileName;
        if (!sFileName.IsEmpty())
        {
			// Store the handle/filename...
			::EnterCriticalSection(&g_CritSec);
            g_openFiles.insert(pair<HANDLE, CString>(hFile, sFileName));
            ::LeaveCriticalSection(&g_CritSec);

			// Convert from creation disposition to our enumeration vals
            FileAction fa;
            switch(dwCreateDisp)
            {
            case CREATE_ALWAYS: fa = File_CreateAlways; break;
            case CREATE_NEW: fa = File_CreateNew; break;
            case OPEN_ALWAYS: fa = File_OpenAlways; break;
            case OPEN_EXISTING: fa = File_OpenExisting; break;
            case TRUNCATE_EXISTING: fa = File_TruncateExisting; break;
            default: fa = File_Unknown; break;
            }

			// Send a notification off to the GUI
            Transmit(fa, API_CreateFile, hFile, lpFileName);
        }
    }

    return hFile;
}

// Our ReadFile and WriteFile detours both call this function to handle the buffer data
// and transmit it off to the GUI.
void ReadOrWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytes, bool isRead)
{
    if (!hFile || !lpBuffer || !nNumberOfBytes)
        return;

    ::EnterCriticalSection(&g_CritSec);

	// Does the file being written to/read from exist in our map?
    map<HANDLE, CString>::const_iterator iter = g_openFiles.find(hFile);
    if (iter != g_openFiles.end())
    {
        CString msg, strFileName;
        strFileName = (*iter).second;
        strFileName.MakeLower();

		// Check the file extension
        if (IsBinaryFile(strFileName))
        {
            msg.Format(L"Target %s %d bytes %s '%s'\nBINARY\n\n", isRead ? L"read" : L"wrote", 
                       nNumberOfBytes, isRead ? L"from" : L"to", (*iter).second);
        }
		// Check for ASCII text first..
        else if (IsAsciiText((LPSTR)lpBuffer, min(100, nNumberOfBytes)))
        {
            // Convert from ASCII to Unicode text
            LPWSTR lpTempBuffer = new WCHAR[nNumberOfBytes+1];
            ::ZeroMemory(lpTempBuffer, (nNumberOfBytes+1) * 2);
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lpBuffer, nNumberOfBytes, lpTempBuffer, nNumberOfBytes+1);
            msg.Format(L"Target %s %d bytes %s '%s'\n%s\n\n", isRead ? L"read" : L"wrote", 
                       nNumberOfBytes, isRead ? L"from" : L"to", (*iter).second, lpTempBuffer);
            delete [] lpTempBuffer;
        }
		// If not ASCII, maybe it's Unicode?
        else if (IsUnicodeText((LPWSTR)lpBuffer, min(100, nNumberOfBytes / 2)))
        {
            msg.Format(L"Target %s %d bytes %s '%s'\n", isRead ? L"read" : L"wrote", 
                       nNumberOfBytes, isRead ? L"from" : L"to", (*iter).second);
            msg.Append((LPWSTR)lpBuffer, nNumberOfBytes/2);
            msg.Append(L"\n\n");
        }
		// Nope. Binary.
        else
        {
            msg.Format(L"Target %s %d bytes %s '%s'\nBINARY\n\n", isRead ? L"read" : L"wrote",
                nNumberOfBytes, isRead ? L"from" : L"to", (*iter).second);
        }

		// Notify the GUI.
        Transmit(isRead ? File_Read : File_Write, isRead ? API_ReadFile : API_WriteFile, hFile, msg);
    }

    ::LeaveCriticalSection(&g_CritSec);
}

// Our custom version of the WriteFile Windows API.
// http://msdn.microsoft.com/en-us/library/aa365747(VS.85).aspx
BOOL WINAPI Mine_WriteFile(HANDLE hFile,LPCVOID lpBuffer,DWORD nNumberOfBytesToWrite,LPDWORD lpNumberOfBytesWritten,LPOVERLAPPED lpOverlapped)
{
    BOOL bSuccess = Real_WriteFile( hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
    ReadOrWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, false);
    return bSuccess;
}


// Our custom version of the WriteFile Windows API.
// http://msdn.microsoft.com/en-us/library/aa365747(VS.85).aspx
BOOL WINAPI Mine_ReadFile(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,
                            LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    BOOL bSuccess = Real_ReadFile(hFile, lpBuffer, nNumberOfBytesToRead,lpNumberOfBytesRead, lpOverlapped);
    ReadOrWriteFile(hFile, lpBuffer, *lpNumberOfBytesRead, true);
    return bSuccess;
}

// Our custom version of the CloseHandle Windows API.
// http://msdn.microsoft.com/en-us/library/ms724211(VS.85).aspx
BOOL WINAPI Mine_CloseHandle(HANDLE hObject)
{
    BOOL bSuccess = Real_CloseHandle(hObject);

	::EnterCriticalSection(&g_CritSec);

    map<HANDLE, CString>::iterator iter = g_openFiles.find(hObject);
    if (iter != g_openFiles.end())
    {
        Transmit(File_Close, API_CloseHandle, hObject, L"");
        g_openFiles.erase(iter);
    }

    ::LeaveCriticalSection(&g_CritSec);
    return bSuccess;
}

// DLL Entry Point. This DLL will be loaded in two places on any given run:
// in the XFileMonitor.Gui.EXE application, and in the target application's
// address space.
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (::GetModuleHandle(L"XFILEMONITOR.GUI.EXE") == NULL)
        {
            InitializeCriticalSection(&g_CritSec);
            DetourTransactionBegin(); 
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)Real_CreateFile, Mine_CreateFile);
            DetourAttach(&(PVOID&)Real_CloseHandle, Mine_CloseHandle);
            DetourAttach(&(PVOID&)Real_WriteFile, Mine_WriteFile);
            DetourAttach(&(PVOID&)Real_ReadFile, Mine_ReadFile);
            DetourTransactionCommit();
        }
        break;

    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    case DLL_PROCESS_DETACH:
        if (::GetModuleHandle(L"XFILEMONITOR.GUI.EXE") == NULL)
        {
            DetourTransactionBegin(); 
            DetourUpdateThread(GetCurrentThread());
            DetourDetach(&(PVOID&)Real_CreateFile, Mine_CreateFile);
            DetourDetach(&(PVOID&)Real_CloseHandle, Mine_CloseHandle);
            DetourDetach(&(PVOID&)Real_WriteFile, Mine_WriteFile);			
            DetourDetach(&(PVOID&)Real_ReadFile, Mine_ReadFile);
            DetourTransactionCommit();
        }
        break;
    }
    return TRUE;
}

//  Install the global window procedure hook, causing this DLL to be mapped
//  into the address space of every process on the machine.
bool XFILEMONITORHOOK_API InstallHook(LPCWSTR targetExePath)
{
    // Initialize some data structures...
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    // The following call loads the target executable with our DLL attached
    BOOL bSuccess = DetourCreateProcessWithDll( targetExePath, NULL, NULL, NULL, TRUE, 
                                CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED, NULL, NULL,
                                &si, &pi, "detoured.dll", "XFileMonitor.Hook.dll", NULL);
    if (bSuccess)
        ResumeThread(pi.hThread);

    return TRUE == bSuccess;
}
