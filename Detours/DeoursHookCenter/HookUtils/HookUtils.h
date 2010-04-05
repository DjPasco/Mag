#pragma once

#include <iostream>

namespace hook_utils
{
	void StartExeWithHookDll(std::wstring sRunExe);

	std::wstring GetExePathW(std::wstring sExeName);
	std::string GetDllPath(std::string sExeName);
	std::wstring GetDllPath(std::wstring sExeName);
}