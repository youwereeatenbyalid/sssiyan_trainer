
#include "GilverNoDTCooldown.hpp"
#include "mods/PlayerTracker.hpp"
//jumps
uintptr_t GilverNoDTCooldown::jmp_return{NULL};
uintptr_t GilverNoDTCooldown::ja_return{NULL};
bool GilverNoDTCooldown::cheaton{NULL};
//variables
// clang-format off
// only in clang/icl mode on x64, sorry

  static naked void newmem_detour() {
__asm {
  validation:
    cmp [PlayerTracker::playerid], 2 //change this to the char number obviously
    jne code
    cmp byte ptr [GilverNoDTCooldown::cheaton], 1
    je cheatcode
    jmp code
  code:
    comiss xmm0, xmm6
    ja ja_return
    jmp qword ptr [GilverNoDTCooldown::jmp_return]

  ja_return:
	jmp qword ptr [GilverNoDTCooldown::ja_return]

  cheatcode:
    jmp qword ptr [GilverNoDTCooldown::jmp_return]
    

  }
}


// clang-format on

void GilverNoDTCooldown::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> GilverNoDTCooldown::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  
  m_is_enabled = &GilverNoDTCooldown::cheaton;
  m_on_page    = gilver;
  m_full_name_string     = "No DT Cooldown";
  m_author_string        = "SSSiyan";
  m_description_string   = "Remove the delay when summoning/unsummoning Nightmare.";

  set_up_hotkey();
  auto nodtcooldown_addr = patterns->find_addr(base, "0F 87 A6 00 00 00 48 8B 15");

  GilverNoDTCooldown::ja_return = nodtcooldown_addr.value()+0xAC;


  if (!nodtcooldown_addr) {
    return "Unable to find nodtcooldown pattern.";
  }
  if (!install_hook_absolute(nodtcooldown_addr.value(), m_nodtcooldown_hook, &newmem_detour, &jmp_return, 6)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize nodtcooldown";
  }

  return Mod::on_initialize();
}

// during load
void GilverNoDTCooldown::on_config_load(const utility::Config &cfg) {}
// during save
void GilverNoDTCooldown::on_config_save(utility::Config &cfg) {}
// do something every frame
void GilverNoDTCooldown::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void GilverNoDTCooldown::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void GilverNoDTCooldown::on_draw_ui() {}