#include <vector>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include "menu.hpp"
#include "../../utils/utils.hpp"
#include "../imgui/imgui_internal.hpp"

using namespace App;

void Menu::RenderMenu() {
	if (!InitModule) {
		SetupModPF();
		InitModule = true;
	}
	ImGui::PushFont(RegFont);
	ImGui::PushFont(IconsFont);
    {
        RenderOutput();
    }
	ImGui::PopFont();
	ImGui::PopFont();
}

void LineSeparator(int pad = 0, int y = 1) {
	ImGui::GetForegroundDrawList()->AddLine({ 0, ImGui::GetCursorScreenPos().y + pad }, { ImGui::GetIO().DisplaySize.x, ImGui::GetCursorScreenPos().y + pad }, IM_COL32(255, 255, 255, 50), y);
}

void Menu::RenderOutput() {
	ImGui::Begin("##main", NULL,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollWithMouse
	);
	{
		
		ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);
		ImGui::SetWindowPos({ 0,0 }, ImGuiCond_Once);

		ImGui::PushFont(TitleFont);
		ImGui::Text("WinPatcher v1.0");
		ImGui::PopFont();
		
		ImGui::BeginDisabled();
		ImGui::PushFont(SubTFont);
		ImGui::Text("Simple & Effective Windows 11 Customization Tool");
		ImGui::PopFont();
		ImGui::EndDisabled();

		LineSeparator(ImGui::GetStyle().ItemSpacing.y*2.f);

		ImGui::Text("");

		ImVec2 winsize = ImGui::GetWindowSize();
		// determines the number of columns there should be based on the window x (width)
		int columns = ImGui::GetIO().DisplaySize.x < 1100 /*1100=monitorsz*/ ? 2 : 3;
		columns = ImGui::GetIO().DisplaySize.x < 700 ? 1 : columns;

		// parent for 3 columns
		ImGui::BeginChild(1, winsize - ImVec2{ 0, ImGui::GetCursorScreenPos().y }, 0);
		{
			// first column
			// window flags ternary statement to determine if there should just be 1 scroll for only 1 column (imgui obscurity)
			ImGui::BeginChild(2, ImVec2{ winsize.x / columns, winsize.y }, 0, columns-1 ? ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse : 0);
			{
				for (int i = 0; i < moduleprefab.size() / columns; i++) {
					moduleprefab[i]->displayModule(RegFont);
					ImGui::Text("");
				}
			}
			ImGui::EndChild();

			ImGui::SameLine();

			// if should be more than 1 columns, begin render 2nd column
			if (columns - 1) {
				ImGui::BeginChild(3, ImVec2{ winsize.x / columns, winsize.y }, 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				{
					for (int i = moduleprefab.size() / columns; i < (moduleprefab.size() / columns) * 2; i++) {
						moduleprefab[i]->displayModule(RegFont);
						ImGui::Text("");
					}
				}
				ImGui::EndChild();
			}

			// if should be more than 2 columns, begin render 3rd column
			if (columns - 2) {
				ImGui::SameLine();

				ImGui::BeginChild(4, ImVec2{ winsize.x / columns, winsize.y }, 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				{
					for (int i = (moduleprefab.size() / columns) * 2; i < moduleprefab.size(); i++) {
						moduleprefab[i]->displayModule(RegFont);
						ImGui::Text("");
					}
				}
				ImGui::EndChild();
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void Menu::SetupModPF() {
	moduleprefab.push_back(
		new Module(
			"Old Right-Click",
			"Revert To Windows 10 Right Click Menu",
			[]() {
				system("reg.exe add \"HKCU\\Software\\Classes\\CLSID\\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2}\\InprocServer32\" /f"); /// good
				Utils::RestartProcess("explorer.exe");
			},
			[]() {
				system("reg.exe delete \"HKCU\\Software\\Classes\\CLSID\\{86ca1aa0-34aa-4e8b-a509-50c905bae2a2}\" /f");
				Utils::RestartProcess("explorer.exe");
			}
		)
	);
	moduleprefab.push_back(
		new Module(
			"Old Terminal",
			"Revert To Windows 10 Command Prompt",
			[]() {
				//system("reg.exe delete \"HKCU\\Console\\%%Startup\" /v DelegationConsole /f");
				//system("reg.exe delete \"HKCU\\Console\\%%Startup\" /v DelegationTerminal /f");
				system("reg.exe add \"HKCU\\Console\\%%Startup\" /v DelegationConsole /t REG_SZ /d {B23D10C0-E52E-411E-9D5B-C09FDF709C7D} /f");
				system("reg.exe add \"HKCU\\Console\\%%Startup\" /v DelegationTerminal /t REG_SZ /d {B23D10C0-E52E-411E-9D5B-C09FDF709C7D} /f");
			},
			[]() {
				//system("reg.exe delete \"HKCU\\Console\\%%Startup\" /v DelegationConsole /f");
				//system("reg.exe delete \"HKCU\\Console\\%%Startup\" /v DelegationTerminal /f");
				system("reg.exe add \"HKCU\\Console\\%%Startup\" /v DelegationConsole /t REG_SZ /d {2EACA947-7F5F-4CFA-BA87-8F7FBEEFBE69} /f");
				system("reg.exe add \"HKCU\\Console\\%%Startup\" /v DelegationTerminal /t REG_SZ /d {E12CFF52-A866-4C77-9A90-F570A7AA2C6B} /f");
			}
		)
	);
	moduleprefab.push_back(
		new Module(
			"0ms Thumbnail",
			"Instant Taskbar Thumbnails When Hovering",
			[]() {
				system("reg.exe delete \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" /v ExtendedUIHoverTime /f");
				system("reg.exe add \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" /v ExtendedUIHoverTime /t REG_DWORD /d 1 /f");
				Utils::RestartProcess("explorer.exe");
			},
			[]() {
				system("reg.exe delete \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" /v ExtendedUIHoverTime /f");
				Utils::RestartProcess("explorer.exe");
			}
		)
	);
	moduleprefab.push_back(
		new Module(
			"ActiveReport",
			"Verbose Boot Messages (Needs Restart)",
			[]() {
				system("reg.exe add \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\" /v VerboseStatus /t REG_DWORD /d 1 /f");
				system("echo You Must Restart For Changes To Take Place & pause");
			},
			[]() {
				system("reg.exe delete \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\" /v VerboseStatus /f");
				system("echo You Must Restart For Changes To Take Place & pause");
			}
		)
	);
	/*moduleprefab.push_back(
		new Module(
			"NoImagePreview",
			"Force Explorer Image Previewing Off",
			[]() {
				system("reg.exe add \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" /v DisableThumbnailCache /t REG_DWORD /d 1 /f & pause");
				Utils::RestartProcess("explorer.exe");
			},
			[]() {
				system("reg.exe add \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" /v DisableThumbnailCache /t REG_DWORD /d 0 /f & pause");
				Utils::RestartProcess("explorer.exe");
			}
		)
	);*/
	moduleprefab.push_back(
		new Module(
			"NoAutoUpdate",
			"Prevent Updates From Automatically Downloading",
			[]() {
				system("reg.exe add \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU\" /v NoAutoUpdate /t REG_DWORD /d 1 /f");
			},
			[]() {
				system("reg.exe delete \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate\\AU\" /v NoAutoUpdate /f");
			}
		)
	);
	moduleprefab.push_back(
		new Module(
			"NoAcrylicBg",
			"No Windows 11 Acrylic (Needs Restart)",
			[]() {
				system("reg.exe add \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\System\" /v DisableAcrylicBackgroundOnLogon /t REG_DWORD /d 1 /f");
				system("reg.exe add \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\Dwm\" /v ForceEffectMode /t REG_DWORD /d 1 /f");
				system("echo You Must Restart For Changes To Take Place & pause");
			},
			[]() {
				system("reg.exe delete \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\System\" /v DisableAcrylicBackgroundOnLogon /f");
				system("reg.exe delete \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\Dwm\" /v ForceEffectMode /f");
				system("echo You Must Restart For Changes To Take Place & pause");
			}
		)
	);
	moduleprefab.push_back(
		new Module(
			"TinyTaskbar",
			"Tiny Taskbar Icons (Needs W10 Taskbar)",
			[]() {
				system("reg.exe add \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" /v TaskbarSmallIcons /t REG_DWORD /d 1 /f");
				Utils::RestartProcess("explorer.exe");
			},
			[]() {
				system("reg.exe add \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" /v TaskbarSmallIcons /t REG_DWORD /d 0 /f");
				Utils::RestartProcess("explorer.exe");
			}
		)
	);
	moduleprefab.push_back(
		new Module(
			"NoFilePeek",
			"Remove Content Previewing In File Icons",
			[]() {
				system("reg.exe add \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" /v IconsOnly /t REG_DWORD /d 1 /f");
				Utils::RestartProcess("explorer.exe");
			},
			[]() {
				system("reg.exe add \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" /v IconsOnly /t REG_DWORD /d 0 /f");
				Utils::RestartProcess("explorer.exe");
			}
		)
	);
	moduleprefab.push_back(
		new Module(
			"SuperHidden",
			"Reveal \"Super Hidden\" Files In Explorer",
			[]() {
				system("reg.exe add \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" /v ShowSuperHidden /t REG_DWORD /d 1 /f");
				Utils::RestartProcess("explorer.exe");
			},
			[]() {
				system("reg.exe add \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\" /v ShowSuperHidden /t REG_DWORD /d 0 /f");
				Utils::RestartProcess("explorer.exe");
			}
		)
	);
	moduleprefab.push_back(
		new Module(
			"Godmode",
			"Create A Godmode Utility Folder",
			[]() {
				system("echo Godmode Will Be Placed In & cd & pause");
				system("mkdir GodMode.{ED7BA470-8E54-465E-825C-99712043E01C}");
			},
			[]() {
				system("echo Godmode Will Be Attempted To Be Removed From & cd & pause");
				system("rmdir GodMode.{ED7BA470-8E54-465E-825C-99712043E01C}");
			}
		)
	);
	moduleprefab.push_back(
		new Module(
			"NoDynamicSearch",
			"Remove Suggested Articles During Search",
			[]() {
				system("reg.exe add \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\SearchSettings\" /v IsDynamicSearchBoxEnabled /t REG_DWORD /d 0 /f");
				Utils::RestartProcess("explorer.exe");
			},
			[]() {
				system("reg.exe delete \"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\SearchSettings\" /v IsDynamicSearchBoxEnabled /f");
				Utils::RestartProcess("explorer.exe");
			}
		)
	);
	moduleprefab.push_back(
		new Module(
			"OldTaskbar",
			"Revert To Windows 10 Styled Taskbar",
			[]() {
				system("reg.exe add \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell\\Update\\Packages\" /v UndockingDisabled /t REG_DWORD /d 1 /f");
				Utils::RestartProcess("explorer.exe");
			},
			[]() {
				system("reg.exe delete \"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell\\Update\\Packages\" /v UndockingDisabled /f");
				Utils::RestartProcess("explorer.exe");
			}
		)
	);
}

void Menu::SetupStyle() {
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6000000238418579f;
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.WindowRounding = 0.f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 4.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 4.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(10.0f, 2.5f);
	style.FrameRounding = 3.0f;
	style.FrameBorderSize = 1.0f;
	style.ItemSpacing = ImVec2(6.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2(10.0f, 10.0f);
	style.CellPadding = ImVec2(6.0f, 6.0f);
	style.IndentSpacing = 25.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 15.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 3.0f;
	style.TabRounding = 0.0f;
	style.TabBorderSize = 0.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.1882352977991104f, 0.9200000166893005f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.1882352977991104f, 0.2899999916553497f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.239999994635582f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0470588244497776f, 0.0470588244497776f, 0.0470588244497776f, 0.5400000214576721f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.1882352977991104f, 0.5400000214576721f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2274509817361832f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.05882352963089943f, 0.05882352963089943f, 0.05882352963089943f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0470588244497776f, 0.0470588244497776f, 0.0470588244497776f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3372549116611481f, 0.3372549116611481f, 0.3372549116611481f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.4000000059604645f, 0.4000000059604645f, 0.4000000059604645f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5568627715110779f, 0.5568627715110779f, 0.5568627715110779f, 0.5400000214576721f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.3294117748737335f, 0.6666666865348816f, 0.8588235378265381f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.3372549116611481f, 0.3372549116611481f, 0.3372549116611481f, 0.5400000214576721f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5568627715110779f, 0.5568627715110779f, 0.5568627715110779f, 0.5400000214576721f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.0470588244497776f, 0.0470588244497776f, 0.0470588244497776f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.1882352977991104f, 0.5400000214576721f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2274509817361832f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.5199999809265137f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.3600000143051147f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2274509817361832f, 0.3300000131130219f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 0.2899999916553497f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.4392156898975372f, 0.4392156898975372f, 0.4392156898975372f, 0.2899999916553497f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.4000000059604645f, 0.4392156898975372f, 0.4666666686534882f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 0.2899999916553497f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.4392156898975372f, 0.4392156898975372f, 0.4392156898975372f, 0.2899999916553497f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.4000000059604645f, 0.4392156898975372f, 0.4666666686534882f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.0f, 0.0f, 0.0f, 0.5199999809265137f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2000000029802322f, 0.3600000143051147f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0f, 0.0f, 0.0f, 0.5199999809265137f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5199999809265137f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0f, 0.0f, 0.0f, 0.5199999809265137f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2784313857555389f, 0.2784313857555389f, 0.2784313857555389f, 0.2899999916553497f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2000000029802322f, 0.2196078449487686f, 0.2274509817361832f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.3294117748737335f, 0.6666666865348816f, 0.8588235378265381f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 0.0f, 0.0f, 0.699999988079071f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.2000000029802322f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.3499999940395355f);
}
