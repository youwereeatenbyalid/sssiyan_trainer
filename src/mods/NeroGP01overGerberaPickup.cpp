#include "NeroGP01overGerberaPickup.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t NeroGP01overGerberaPickup::jmp_ret{NULL};
bool NeroGP01overGerberaPickup::cheaton{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry
static naked void detour() {
	__asm {
	    validation:
            cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
            jne code
            cmp byte ptr [NeroGP01overGerberaPickup::cheaton], 1
            je cheatcode
            jmp code

        code:
            mov [rcx+0x20], r11d
            mov r9d, 00000001
            jmp qword ptr [NeroGP01overGerberaPickup::jmp_ret]        

        cheatcode:
            cmp r11d, 3
            je GP01overGerbera
            jmp code

         GP01overGerbera:
            mov dword ptr [rcx+0x20], 9
            mov r9d, 00000001
            jmp qword ptr [NeroGP01overGerberaPickup::jmp_ret]
	}
}
// clang-format on

void NeroGP01overGerberaPickup::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NeroGP01overGerberaPickup::on_initialize() {
  init_check_box_info();

   auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
   m_is_enabled = &NeroGP01overGerberaPickup::cheaton;
   m_on_page    = Page_BloodyPalace;
   m_full_name_string     = "GP01 Over Gerbera Pickup";
   m_author_string        = "Dr. Penguin, Siyan";
   m_description_string   = "Replace all Gerbera picked up in BP with GP01.";

  set_up_hotkey();

   auto addr = m_patterns_cache->find_addr(base, "44 89 59 20 41 B9 01 00 00 00");
   if (!addr) {
    return "Unable to find NeroGP01overGerberaPickup pattern.";
  }
   if (!install_new_detour(addr.value(), m_detour, &detour,&jmp_ret, 10)) {
    // return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroGP01overGerberaPickup";
  }
  return Mod::on_initialize();
}

// during load
// void NeroGP01overGerberaPickup::on_config_load(const utility::Config &cfg) {}
// during save
// void NeroGP01overGerberaPickup::on_config_save(utility::Config &cfg) {}
// do something every frame
// void NeroGP01overGerberaPickup::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void NeroGP01overGerberaPickup::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void NeroGP01overGerberaPickup::on_draw_ui() {}
