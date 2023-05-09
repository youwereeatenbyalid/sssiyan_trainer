
#include "CaliburExceed.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t CaliburExceed::jmp_ret{NULL};
bool CaliburExceed::cheaton{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	    validation:
            cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
            jne code
            cmp byte ptr [CaliburExceed::cheaton], 1
            je cheatcode
            jmp code
        code:
            mov rax, [rbx+0x50]
            cmp [rax+0x18], rdi
            jmp qword ptr [CaliburExceed::jmp_ret]        
        cheatcode:
            mov rcx, 3
            mov rax, [rbx+0x50]
            cmp [rax+0x18], rdi
            jmp qword ptr [CaliburExceed::jmp_ret]
	}
}
// clang-format on

void CaliburExceed::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> CaliburExceed::on_initialize() {
  init_check_box_info();

  auto base              = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled           = &CaliburExceed::cheaton;
  m_on_page              = Page_Nero;
  m_depends_on           = { "PlayerTracker" };
  m_full_name_string     = "Force Level 3 Calibur";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Forces calibur to always perform the EX 3 variant.";

  set_up_hotkey();
  //DevilMayCry5.app_PlayerNero__setCalibar
  auto addr = m_patterns_cache->find_addr(base, "92 40 00 8B CF EB 03 8B 48 18 48 8B 43 50 48 39 78 18");
  if (!addr) {
    return "Unable to find CaliburExceed pattern.";
  }
  if (!install_new_detour(addr.value()+0xA, m_detour, &detour, &jmp_ret, 8)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize CaliburExceed";
  }
  return Mod::on_initialize();
}

// during load
// void CaliburExceed::on_config_load(const utility::Config &cfg) {}
// during save
// void CaliburExceed::on_config_save(utility::Config &cfg) {}
// do something every frame
// void CaliburExceed::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void CaliburExceed::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void CaliburExceed::on_draw_ui() {}