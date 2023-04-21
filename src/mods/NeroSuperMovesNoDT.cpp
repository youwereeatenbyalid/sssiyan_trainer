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
  m_is_enabled = &NeroSuperMovesNoDT::cheaton;
  m_on_page    = Page_Nero;
  m_full_name_string     = "Super Moves Outside Of DT";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Use Max Bet & Showdown outside of Devil Trigger.";

  set_up_hotkey();
  auto maxbetnodt_addr = m_patterns_cache->find_addr(base, "83 BA B0 09 00 00 01 0F 85 CE");

  if (!maxbetnodt_addr) {
    return "Unable to find maxbetnodt pattern.";
  }
  if (!install_new_detour(maxbetnodt_addr.value(), m_maxbetnodt_detour, &newmem_detour, &jmp_return, 7)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize maxbetnodt";
  }
  auto showdownnodt_addr = m_patterns_cache->find_addr(base, "83 BA B0 09 00 00 01 0F 85 17");

  if (!showdownnodt_addr) {
    return "Unable to find showdownnodt pattern.";
  }
  if (!install_new_detour(showdownnodt_addr.value(), m_showdownnodt_detour, &newmem2_detour, &jmp_return2, 7)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize showdownnodt";
  }

  return Mod::on_initialize();
}

// during load
// void NeroSuperMovesNoDT::on_config_load(const utility::Config &cfg) {}
// during save
// void NeroSuperMovesNoDT::on_config_save(utility::Config &cfg) {}
// do something every frame
// void NeroSuperMovesNoDT::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void NeroSuperMovesNoDT::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void NeroSuperMovesNoDT::on_draw_ui(){}