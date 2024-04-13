#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include "../imgui/imgui.hpp"
#include "../imgui/imguiraw.hpp"
#include "../imgui/imconfig.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <vector>
#include "module.hpp"

namespace App {
	namespace Menu {
		void SetupStyle();
		void SetupModPF();
		void RenderMenu();

		void RenderOutput();
		
		inline int ActiveCategory	= 1;
		inline bool MenuActive		= true;
		inline bool InitModule		= false;
		inline ImFont* RegFont		= nullptr;
		inline ImFont* SubTFont		= nullptr;
		inline ImFont* TitleFont	= nullptr;
		inline ImFont* IconsFont	= nullptr;
		inline HWND WindowHandle	= nullptr;

		inline std::vector<bool> hovercache(32);
		inline std::vector<Module*> moduleprefab;
	}
}