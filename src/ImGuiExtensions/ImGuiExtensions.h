#pragma once
#include "imgui/imgui.h"
namespace ImGui
{
	void ShowHelpMarker(const char *desc, float textWrapPos = ImGui::GetFontSize() * 35.0f, bool isOnSameLine = true);

	void UnderlineText(const char* text, const ImVec4& textColor, const ImVec4& underlineColor, float thickness = 1.0f);

	//Win-only stuff if proton can't convert ShellExcecute macro(
	void TextHyperlink(const char* text, const char* url, const ImVec4& regularColor = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f }, const ImVec4& selectedColor = ImVec4{ 0.23f, 0.51f, 0.8f, 1.0f });
}