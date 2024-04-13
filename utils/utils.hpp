#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <string>
#include <iostream>
#include <Windows.h>
#include <commdlg.h>

namespace App {
	namespace Utils {
		std::wstring StringToWString(std::string stringToConvert);
		void SetClipboardText(std::wstring text);

		void ImGuiTooltip(const char* fmt, int argc, ...);
		void RestartProcess(const char* pname);
	}
}