#include "EmpoweredCane.hpp"
#include "mods/PlayerTracker.hpp"
//jumps
uintptr_t EmpoweredCane::jmp_return{NULL};
uintptr_t EmpoweredCane::ja_return{NULL};
bool EmpoweredCane::cheaton{NULL};
//variables
// clang-format off
// only in clang/icl mode on x64, sorry

  static naked void newmem_detour() {
__asm {
  validation:
    cmp [PlayerTracker::playerid], 2 //change this to the char number obviously
    jne code
    cmp byte ptr [EmpoweredCane::cheaton], 1
    je cheatcode
    jmp code
  code:
    //+0x5D
    comiss xmm2, xmm3
    ja ja_return
    mov [rbx+0x20], edx
    jmp qword ptr [EmpoweredCane::jmp_return]

  cheatcode:
  ja_return:
	jmp qword ptr [EmpoweredCane::ja_return]
  }
}


// clang-format on

void EmpoweredCane::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> EmpoweredCane::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  m_is_enabled = &EmpoweredCane::cheaton;
  m_on_page    = gilver;
  m_full_name_string     = "Infinite Empowered Cane";
  m_author_string        = "SSSiyan";
  m_description_string   = "Extends the duration of V's BP Taunt to infinity.";

  set_up_hotkey();
  
  auto bettercane_addr = patterns->find_addr(base, "77 5B 89 53 20");

  EmpoweredCane::ja_return = bettercane_addr.value()+0x5D;
  if (!bettercane_addr) {
    return "Unable to find bettercane pattern.";
  }
  if (!install_hook_absolute(bettercane_addr.value(), m_bettercane_hook, &newmem_detour, &jmp_return, 5)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize bettercane";
  }

  return Mod::on_initialize();
}

// during load
// void EmpoweredCane::on_config_load(const utility::Config &cfg) {}
// during save
// void EmpoweredCane::on_config_save(utility::Config &cfg) {}
// do something every frame
//void EmpoweredCane::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void EmpoweredCane::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void EmpoweredCane::on_draw_ui() {}