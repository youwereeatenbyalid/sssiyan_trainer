#include "InfiniteGambits.hpp"
#include "mods/PlayerTracker.hpp"
//jumps
uintptr_t InfiniteGambits::jmp_return{NULL};
bool InfiniteGambits::cheaton{NULL};
//variables
// clang-format off
// only in clang/icl mode on x64, sorry

  static naked void newmem_detour() {
__asm {
  validation:
    cmp [PlayerTracker::playerid], 2 //change this to the char number obviously
    jne code
    cmp byte ptr [InfiniteGambits::cheaton], 1
    je cheatcode
    jmp code
  code:
    inc dword ptr [rdi+0x00001828]
    jmp qword ptr [InfiniteGambits::jmp_return]

  cheatcode:
    jmp qword ptr [InfiniteGambits::jmp_return]
  }
}

// clang-format on

void InfiniteGambits::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> InfiniteGambits::on_initialize() {
  init_check_box_info();

  auto base              = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled           = &InfiniteGambits::cheaton;
  m_on_page              = Page_Gilver;
  m_depends_on           = { "PlayerTracker" };
  m_full_name_string     = "Infinite Gambits";
  m_author_string        = "SSSiyan";
  m_description_string   = "Infinite number of aerial gambits.";

  set_up_hotkey();
  auto infinitegambit_addr = m_patterns_cache->find_addr(base, "FF 87 28 18 00 00 48");

  if (!infinitegambit_addr) {
    return "Unable to find infinitegambit pattern.";
  }
  if (!install_new_detour(infinitegambit_addr.value(), m_infinitegambit_detour, &newmem_detour, &jmp_return, 6)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize infinitegambit";
  }

  return Mod::on_initialize();
}

// during load
// void InfiniteGambits::on_config_load(const utility::Config &cfg) {}
// during save
// void InfiniteGambits::on_config_save(utility::Config &cfg) {}
// do something every frame
//void InfiniteGambits::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void InfiniteGambits::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void InfiniteGambits::on_draw_ui() {}