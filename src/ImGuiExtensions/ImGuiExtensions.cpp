#include "ImGuiExtensions.h"

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
