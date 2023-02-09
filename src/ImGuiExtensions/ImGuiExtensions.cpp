#include "ImGuiExtensions.h"
#include <Windows.h>
#include <shellapi.h>

//clang-format off
void ImGui::ShowHelpMarker(const char* desc, float textWrapPos, bool isOnSameLine)
{
    if (isOnSameLine)
        ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(textWrapPos);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ImGui::UnderlineText(const char* text, const ImVec4& textColor, const ImVec4& underlineColor, float thickness)
{
    ImGui::PushStyleColor(ImGuiCol_Text, textColor);
    ImGui::TextWrapped(text);
    ImGui::PopStyleColor();
    ImVec2 min = GetItemRectMin();
    ImVec2 max = GetItemRectMax();
    min.y = max.y;
    GetWindowDrawList()->AddLine(min, max, ImGui::GetColorU32(underlineColor), 1.0f);
}

void ImGui::TextHyperlink(const char* text, const char* url, const ImVec4& regularColor, const ImVec4& selectedColor)
{
    auto txtSize = ImGui::CalcTextSize(text);
    ImGui::InvisibleButton(text, txtSize);
    ImGui::SameLine(0, 0);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() - txtSize.x);
    if (!ImGui::IsItemHovered())
        ImGui::UnderlineText(text, regularColor, regularColor);
    else
    {
        ImGui::UnderlineText(text, selectedColor, selectedColor);
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            ShellExecute(0, 0, url, 0, 0, SW_SHOW);
        const float textWrapPos = ImGui::GetFontSize() * 35.0f;
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(textWrapPos);
        ImGui::TextUnformatted(url);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
//clang-format on