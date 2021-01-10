#if 0
#include "MoveID.hpp"

// clang-format off
// only in clang/icl mode on x64, sorry
/*
static naked void detour() {
	__asm {
		mov qword ptr [MoveID::variable], rbx
		mov rax, 0xDEADBEEF
		jmp qword ptr [jmp_ret]
	}
}
*/
// clang-format on

std::optional<std::string> MoveID::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  //if (!install_hook_offset(0xBADF00D, m_function_hook, &detour, &jmp_ret, 5)) {
  //  return a error string in case something goes wrong
  //  spdlog::error("[{}] failed to initialize", get_name());
  //  return "Failed to initialize MoveID";
  //}
  return Mod::on_initialize();
}

// during load
//void MoveID::on_config_load(const utility::Config &cfg) {}
// during save
//void MoveID::on_config_save(utility::Config &cfg) {}
// do something every frame
//void MoveID::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void MoveID::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
//void MoveID::on_draw_ui() {}
#endif
