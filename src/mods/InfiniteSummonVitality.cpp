#include "InfiniteSummonVitality.hpp"
#include "mods/PlayerTracker.hpp"
//jumps
uintptr_t InfiniteSummonVitality::jmp_return{NULL};
bool InfiniteSummonVitality::cheaton{NULL};
//variables
// clang-format off
// only in clang/icl mode on x64, sorry

  static naked void newmem_detour() {
__asm {
  validation:
    cmp [PlayerTracker::playerid], 2 //change this to the char number obviously
    jne code
    cmp byte ptr [InfiniteSummonVitality::cheaton], 1
    je cheatcode
    jmp code
  code:
    movss xmm0,[rax+0x10]
    jmp qword ptr [InfiniteSummonVitality::jmp_return]

  cheatcode:
    fld qword ptr [rax+0x14]
    fstp qword ptr [rax+0x10]
    movss xmm0,[rax+0x10]
    jmp qword ptr [InfiniteSummonVitality::jmp_return]
  }
}

// clang-format on

void InfiniteSummonVitality::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> InfiniteSummonVitality::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &InfiniteSummonVitality::cheaton;
  m_on_page    = Page_Gilver;
  m_full_name_string     = "Infinite Summon Vitality";
  m_author_string        = "Jessie Kazama";
  m_description_string   = "Pet health is locked to max.";

  set_up_hotkey();
  auto infsummonvitality_addr = m_patterns_cache->find_addr(base, "D2 EB 34 F3 0F 10 48 14 F3 0F 10 40 10");

  if (!infsummonvitality_addr) {
    return "Unable to find infsummonvitality pattern.";
  }
  if (!install_new_detour(infsummonvitality_addr.value()+0x08, m_infsummonvitality_detour, &newmem_detour, &jmp_return, 5)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize infsummonvitality";
  }

  return Mod::on_initialize();
}

// during load
// void InfiniteSummonVitality::on_config_load(const utility::Config &cfg) {}
// during save
// void InfiniteSummonVitality::on_config_save(utility::Config &cfg) {}
// do something every frame
// void InfiniteSummonVitality::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void InfiniteSummonVitality::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void InfiniteSummonVitality::on_draw_ui() {}