#include "DontHideWeaponWheel.hpp"
#include "mods/PlayerTracker.hpp"
//jumps
uintptr_t DontHideWeaponWheel::jmp_wagreturn{NULL};
bool DontHideWeaponWheel::cheaton{NULL};
//variables
// clang-format off
// only in clang/icl mode on x64, sorry

  static naked void wagnewmem_detour() {
__asm {
  validation:
    cmp byte ptr [DontHideWeaponWheel::cheaton], 1
    jne code
	cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
    je cheatcode
	cmp [PlayerTracker::playerid], 3 //change this to the char number obviously
    je cheatcode
    jmp code
  code:
    comiss xmm1, xmm0
    movss [rdi+0x40],xmm1
    jmp qword ptr [DontHideWeaponWheel::jmp_wagreturn]

  cheatcode:
    comiss xmm1, xmm0
    jmp qword ptr [DontHideWeaponWheel::jmp_wagreturn]
    

  }
}


// clang-format on

void DontHideWeaponWheel::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DontHideWeaponWheel::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  ischecked = &DontHideWeaponWheel::cheaton;
  onpage    = camera;
  full_name_string     = "Never Hide Guns/Weapons HUD";
  author_string        = "SSSiyan";
  description_string   = "The weapon wheel always stays out.";
  auto donthideweaponandgun_addr = patterns.find_addr(base, "F3 0F 11 4F 40 77");

  if (!donthideweaponandgun_addr) {
    return "Unable to find donthideweaponandgun pattern.";
  }
  if (!install_hook_absolute(donthideweaponandgun_addr.value(), m_donthideweaponandgun_hook, &wagnewmem_detour, &jmp_wagreturn, 5)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize donthideweaponandgun";
  }

  return Mod::on_initialize();
}

// during load
void DontHideWeaponWheel::on_config_load(const utility::Config &cfg) {}
// during save
void DontHideWeaponWheel::on_config_save(utility::Config &cfg) {}
// do something every frame
void DontHideWeaponWheel::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void DontHideWeaponWheel::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void DontHideWeaponWheel::on_draw_ui() {}
