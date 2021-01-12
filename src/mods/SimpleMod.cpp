#include "SimpleMod.hpp"


// clang-format off
// clang-format on

std::optional<std::string> SimpleMod::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();
  return Mod::on_initialize();
}

// during load
//void SimpleMod::on_config_load(const utility::Config &cfg) {}
// during save
//void SimpleMod::on_config_save(utility::Config &cfg) {}
// do something every frame
//void SimpleMod::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void SimpleMod::on_draw_debug_ui() {
	ImGui::Text("SimpleMod debug data");
	// Animate a simple progress bar
	static float progress = 0.0f, progress_dir = 1.0f;

	progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
	if (progress >= +1.1f) { progress = +1.1f; progress_dir *= -1.0f; }
	if (progress <= -0.1f) { progress = -0.1f; progress_dir *= -1.0f; }

	// Typically we would use ImVec2(-1.0f,0.0f) or ImVec2(-FLT_MIN,0.0f) to use all available width,
	// or ImVec2(width,0.0f) for a specified width. ImVec2(0.0f,0.0f) uses ItemWidth.
	ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
}
// will show up in main window, dump ImGui widgets you want here
bool checkbox;
bool abox;
bool bbox;
void SimpleMod::on_draw_ui() {
    ImGui::Checkbox("##mod1", &checkbox);
    ImGui::SameLine();
	ImGui::Selectable("Mod 1");
    ImGui::Checkbox("##mod2", &abox);
    ImGui::SameLine();
    ImGui::Selectable("Mod 2");
    ImGui::Checkbox("##mod3", &bbox);
    ImGui::SameLine();
    ImGui::Selectable("Mod 3");
}