#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui_internal.hpp"
#include <functional>

inline static int idinc = 1;

struct Module {
private:
	const char* name;
	const char* desc;
	int		moduleid;

	std::function<void()> onpatch;
	std::function<void()> onreverse;
public:
	Module(
		const char* n,
		const char* d,
		std::function<void()> p,
		std::function<void()> r
	) :
		name(n),
		desc(d),
		onpatch(p),
		onreverse(r),
		moduleid(idinc)
	{
		idinc++;
	};
	
	void displayModule(ImFont* font);
	
	static bool customButtonImpl(const char* label, int idcount, const ImVec2& size_arg, ImGuiButtonFlags flags);
};