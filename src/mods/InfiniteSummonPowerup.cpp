#include "InfiniteSummonPowerup.hpp"
#include "mods/PlayerTracker.hpp"
//jumps
uintptr_t InfiniteSummonPowerup::jmp_return{NULL};
bool InfiniteSummonPowerup::cheaton{NULL};
//variables
// clang-format off
// only in clang/icl mode on x64, sorry

  static naked void newmem_detour() {
__asm {
  validation:
    cmp [PlayerTracker::playerid], 2 //change this to the char number obviously
    jne code
    cmp byte ptr [InfiniteSummonPowerup::cheaton], 1
    je cheatcode
    jmp code
  code:
    movss [rdi+0x0000016C],xmm0
    jmp qword ptr [InfiniteSummonPowerup::jmp_return]

  cheatcode:
    jmp qword ptr [InfiniteSummonPowerup::jmp_return]
    

  }
}


// clang-format on

void InfiniteSummonPowerup::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> InfiniteSummonPowerup::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &InfiniteSummonPowerup::cheaton;
  m_on_page    = gilver;
  m_full_name_string     = "Infinite Summon Powerup Duration";
  m_author_string        = "Jessie Kazama";
  m_description_string   = "Summon powerup state lasts forever.";

  set_up_hotkey();
  auto infinitesummonpowerupduration_addr = patterns->find_addr(base, "F3 0F 11 87 6C 01 00 00 48 8B 43");

  if (!infinitesummonpowerupduration_addr) {
    return "Unable to find infinitesummonpowerupduration pattern.";
  }
  if (!install_hook_absolute(infinitesummonpowerupduration_addr.value(), m_infinitesummonpowerupduration_hook, &newmem_detour, &jmp_return, 8)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize infinitesummonpowerupduration";
  }

  return Mod::on_initialize();
}

// during load
void InfiniteSummonPowerup::on_config_load(const utility::Config &cfg) {}
// during save
void InfiniteSummonPowerup::on_config_save(utility::Config &cfg) {}
// do something every frame
void InfiniteSummonPowerup::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void InfiniteSummonPowerup::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void InfiniteSummonPowerup::on_draw_ui() {}