
#include "StyleRank.hpp"
uintptr_t	StyleRank::jmp_ret{NULL};
uintptr_t	StyleRank::rankaddress{NULL};
uint32_t	StyleRank::rank{0};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	styleranknewmem:
		lea rcx,[rax+0x000000B0]
		mov [StyleRank::rankaddress], rcx
		mov ecx,[rax+0x000000B0]
		mov [StyleRank::rank], ecx

	stylerankoriginalcode:
		mov ecx,[rax+0x000000B0]

	stylerankexit:
		jmp qword ptr [StyleRank::jmp_ret]
	}
}

// clang-format on

void StyleRank::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> StyleRank::on_initialize() {
  init_check_box_info();

  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

 if (!install_hook_offset(offsets::STYLE_RANK, m_function_hook, &detour, &jmp_ret, 6)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize StyleRank";
  }
  return Mod::on_initialize();
}

// during load
//void StyleRank::on_config_load(const utility::Config &cfg) {}
// during save
//void StyleRank::on_config_save(utility::Config &cfg) {}
// do something every frame
//void StyleRank::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void StyleRank::on_draw_debug_ui() {
  ImGui::Text("Style Rank: %X", StyleRank::rank);
}
  // will show up in main window, dump ImGui widgets you want here
void StyleRank::on_draw_ui() {

}
