#include "NeroSuperMovesNoDT.hpp"
#include "mods/PlayerTracker.hpp"
//jumps
uintptr_t NeroSuperMovesNoDT::jmp_return{NULL};
uintptr_t NeroSuperMovesNoDT::jmp_return2{NULL};
bool NeroSuperMovesNoDT::cheaton{NULL};
//variables
// clang-format off
// only in clang/icl mode on x64, sorry

  static naked void newmem_detour() {
__asm {
  validation:
    cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
    jne code
    cmp byte ptr [NeroSuperMovesNoDT::cheaton], 1
    je cheatcode
    jmp code
  code:
    cmp dword ptr [rdx+0x000009B0],0x01
    jmp qword ptr [NeroSuperMovesNoDT::jmp_return]

  cheatcode:
    //cmp dword ptr [rdx+0x000009B0],0x01
    jmp qword ptr [NeroSuperMovesNoDT::jmp_return]
    

  }
}
  static naked void newmem2_detour() {
__asm {
  validation:
    cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
    jne code
    cmp byte ptr [NeroSuperMovesNoDT::cheaton], 1
    je cheatcode
    jmp code
  code:
    cmp dword ptr [rdx+0x000009B0],0x01
    jmp qword ptr [NeroSuperMovesNoDT::jmp_return2]

  cheatcode:
    //cmp dword ptr [rdx+0x000009B0],0x01
    jmp qword ptr [NeroSuperMovesNoDT::jmp_return2]
    

  }
}


// clang-format on

void NeroSuperMovesNoDT::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NeroSuperMovesNoDT::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  ischecked = &NeroSuperMovesNoDT::cheaton;
  onpage    = nero;
  full_name_string     = "Super Moves outside of DT";
  author_string        = "The Hitchhiker";
  description_string   = "Use Max Bet & Showdown outside of Devil Trigger.";
  auto maxbetnodt_addr = utility::scan(base, "83 BA B0 09 00 00 01 0F 85 CE");

  if (!maxbetnodt_addr) {
    return "Unable to find maxbetnodt pattern.";
  }
  if (!install_hook_absolute(maxbetnodt_addr.value(), m_maxbetnodt_hook, &newmem_detour, &jmp_return, 7)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize maxbetnodt";
  }
  auto showdownnodt_addr = utility::scan(base, "83 BA B0 09 00 00 01 0F 85 17");

  if (!showdownnodt_addr) {
    return "Unable to find showdownnodt pattern.";
  }
  if (!install_hook_absolute(showdownnodt_addr.value(), m_showdownnodt_hook, &newmem2_detour, &jmp_return2, 7)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize showdownnodt";
  }

  return Mod::on_initialize();
}

// during load
void NeroSuperMovesNoDT::on_config_load(const utility::Config &cfg) {}
// during save
void NeroSuperMovesNoDT::on_config_save(utility::Config &cfg) {}
// do something every frame
void NeroSuperMovesNoDT::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void NeroSuperMovesNoDT::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void NeroSuperMovesNoDT::on_draw_ui() {}