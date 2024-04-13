#include "module.hpp"

#include <string>
#include <vector>
#include <windows.h>

void Module::displayModule(ImFont* font) {
	ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2{ ImGui::GetWindowSize().x / 2.f - ImGui::CalcTextSize(name).x / 2.f, 0 });
	ImGui::Text(name);
	ImGui::PushFont(font);
	ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2{ ImGui::GetWindowSize().x / 2.f - ImGui::CalcTextSize(desc).x / 2.f, 0 });
	ImGui::BeginDisabled();
	ImGui::Text(desc);
	ImGui::EndDisabled();
	ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2{ ImGui::GetWindowSize().x / 2.f - (ImGui::CalcTextSize("Start Process").x / 2.f + ImGui::GetStyle().FramePadding.x * 2.f + ImGui::CalcTextSize("Restore Old").x / 2.f + ImGui::GetStyle().ItemInnerSpacing.x / 2.f), 0 });
	if (customButtonImpl("Start Process", moduleid, { 0,0 }, ImGuiButtonFlags_None)) {
		onpatch();
	}
	ImGui::SameLine();
	if (customButtonImpl("Restore Old", moduleid, { 0,0 }, ImGuiButtonFlags_None)) {
		onreverse();
	}
	ImGui::PopFont();
}

bool Module::customButtonImpl(const char* label, int idcount, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
	using namespace ImGui;
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(std::string(label).append(std::to_string(idcount)).c_str());
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderNavHighlight(bb, id);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

	if (g.LogEnabled)
		LogSetNextTextDecoration("[", "]");
	RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	// Automatically close popups
	//if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
	//    CloseCurrentPopup();

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
	return pressed;
}