#if 0
#include "NeroSuperNoDT.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t NeroSuperNoDT::jmp_ret{NULL};
bool NeroSuperNoDT::cheaton{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry
/*
static naked void detour() {
	__asm {
	    validation:
            cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
            jne code
            cmp byte ptr [NeroSuperNoDT::cheaton], 1
            je cheatcode
            jmp code
        code:

            jmp qword ptr [NeroSuperNoDT::jmp_ret]        
        cheatcode:
		
            jmp qword ptr [NeroSuperNoDT::jmp_ret]
	}
}
*/
// clang-format on

void NeroSuperNoDT::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NeroSuperNoDT::on_initialize() {
  init_check_box_info();

  // auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  //ischecked = &NeroSuperNoDT::cheaton;
  //onpage    = commonpage;
  //full_name_string     = "NeroSuperNoDT Full Name";
  //author_string        = "Author";
  //description_string   = "This is the description of NeroSuperNoDT.";

  //auto addr = utility::scan(base, "F3 0F 10 8F 14 1A 00 00 BA");
  //if (!addr) {
  //  return "Unable to find NeroSuperNoDT pattern.";
  //}
  //if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
  //  return a error string in case something goes wrong
  //  spdlog::error("[{}] failed to initialize", get_name());
  //  return "Failed to initialize NeroSuperNoDT";
  //}
  return Mod::on_initialize();
}

// during load
//void NeroSuperNoDT::on_config_load(const utility::Config &cfg) {}
// during save
//void NeroSuperNoDT::on_config_save(utility::Config &cfg) {}
// do something every frame
//void NeroSuperNoDT::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void NeroSuperNoDT::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
//void NeroSuperNoDT::on_draw_ui() {}
#endif
