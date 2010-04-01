#pragma once

#include <iostream>

namespace hook_utils
{
	void LoadNotepadWithHookDll(std::wstring sRunExe);
	std::wstring GetExePath(std::wstring sExeName);

	template<class string_type>
	string_type GetDllPath(string_type sExeName);
}