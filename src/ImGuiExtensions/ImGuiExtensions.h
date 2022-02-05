#pragma once
#include "imgui/imgui.h"
namespace ImGui
{
	void ShowHelpMarker(const char *desc, float textWrapPos = ImGui::GetFontSize() * 35.0f, bool isOnSameLine = true);
}