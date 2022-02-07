#include "HoldToMash.hpp"
#include "mods/PlayerTracker.hpp"
//jumps
uintptr_t HoldToMash::jmp_return{NULL};
bool HoldToMash::cheaton{NULL};
//variables
// clang-format off
// only in clang/icl mode on x64, sorry

  static naked void newmem_detour() {
__asm {
  validation:
    cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
    jne code
    cmp byte ptr [HoldToMash::cheaton], 1
    je cheatcode
    jmp code
  code:
    test [r8+0x48], eax
    seta al
    jmp qword ptr [HoldToMash::jmp_return]

  cheatcode:
    test [r8+0x3C], eax
    seta al
    jmp qword ptr [HoldToMash::jmp_return]
    

  }
}


// clang-format on

void HoldToMash::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> HoldToMash::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &HoldToMash::cheaton;
  m_on_page    = qol;
  m_full_name_string     = "Hold To Mash";
  m_author_string        = "The Hitchhiker";
  m_description_string   = "Hold button for inputs like twosometime, million stab, rising dragon, etc.";

  set_up_hotkey();
  auto holdtomash_addr = utility::scan(base, "41 85 40 48 0F 97 C0");

  if (!holdtomash_addr) {
    return "Unable to find holdtomash pattern.";
  }
  if (!install_hook_absolute(holdtomash_addr.value(), m_holdtomash_hook, &newmem_detour, &jmp_return, 7)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize holdtomash";
  }

  return Mod::on_initialize();
}

// during load
void HoldToMash::on_config_load(const utility::Config &cfg) {}
// during save
void HoldToMash::on_config_save(utility::Config &cfg) {}
// do something every frame
void HoldToMash::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void HoldToMash::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void HoldToMash::on_draw_ui() {}