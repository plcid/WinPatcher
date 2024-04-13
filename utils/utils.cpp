#include "utils.hpp"
#include "../render/imgui/imgui.hpp"
#include <cstdarg>
#include <sstream>
#include <ShlObj_core.h>

using namespace App;

std::wstring Utils::StringToWString(const std::string str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), nullptr, 0);
    std::wstring wideString(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &wideString[0], size_needed);
    return wideString;
}

void Utils::SetClipboardText(std::wstring text) {
    if (OpenClipboard(nullptr)) {
        EmptyClipboard();

        HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, (text.length() + 1) * sizeof(wchar_t));

        if (hClipboardData) {
            wchar_t* pchData = static_cast<wchar_t*>(GlobalLock(hClipboardData));
            if (pchData) {
                wcscpy_s(pchData, text.length() + 1, text.c_str());
                GlobalUnlock(hClipboardData);
                SetClipboardData(CF_UNICODETEXT, hClipboardData);
            }
        }
        CloseClipboard();
    }
}

void Utils::ImGuiTooltip(const char* fmt, int argc, ...) {
    va_list args = 0;
    if (argc) {
        va_start(args, fmt);
        vsprintf_s(const_cast<char*>(fmt), argc, fmt, args);
        va_end(args);
    }

    if (ImGui::BeginItemTooltip()) {
        if (argc)
            ImGui::SetTooltipV(fmt, args);
        else ImGui::SetTooltip(fmt);
        ImGui::EndTooltip();
    }
    
}

void Utils::RestartProcess(const char* pname) {
    system(std::string("taskkill /f /im ").append(pname).c_str());
    system(pname);
    system("exit");
}