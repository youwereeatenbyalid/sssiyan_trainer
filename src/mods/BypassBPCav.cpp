#include "BypassBPCav.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t BypassBPCav::jmp_cavrfix1_return{NULL};
uintptr_t BypassBPCav::jmp_cavrfix2_return{NULL};
uintptr_t BypassBPCav::jmp_cavrfix3_return{ NULL };
bool BypassBPCav::cheaton{NULL};
//variables
// clang-format off
// only in clang/icl mode on x64, sorry

  static naked void newmem_detour1() {
__asm {
  validation:
    cmp byte ptr [BypassBPCav::cheaton], 1
    je cheatcode
    jmp code
  code:
    mov rax,[rbx+0x50]
    mov rcx,[rax+0x18]
    jmp qword ptr [BypassBPCav::jmp_cavrfix1_return]

  cheatcode:
    mov edx, 0x0
    mov rax,[rbx+0x50]
    mov rcx,[rax+0x18]
    jmp qword ptr [BypassBPCav::jmp_cavrfix1_return]
  }
}
  static naked void newmem_detour2() {
__asm {
  validation:
    cmp byte ptr [BypassBPCav::cheaton], 1
    je cheatcode
    jmp code
  code:
    mov rax,[rbx+0x50]
    mov rcx,[rax+0x18]
    jmp qword ptr [BypassBPCav::jmp_cavrfix2_return]

  cheatcode:
    mov edx, 0x0
    mov rax,[rbx+0x50]
    mov rcx,[rax+0x18]
    jmp qword ptr [BypassBPCav::jmp_cavrfix2_return]
  }
}

  static naked void newmem_detour3() {
__asm {
  validation:
    cmp byte ptr [BypassBPCav::cheaton], 1
    je cheatcode
    jmp code
  code:
    mov rax, [rbx + 0x50]
    mov rcx, [rax + 0x18]
    jmp qword ptr [BypassBPCav::jmp_cavrfix3_return]

  cheatcode:
    mov edx, 0x0
    mov rax, [rbx + 0x50]
    mov rcx, [rax + 0x18]
    jmp qword ptr [BypassBPCav::jmp_cavrfix3_return]
    }
  }


// clang-format on
void BypassBPCav::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> BypassBPCav::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &BypassBPCav::cheaton;
  m_on_page    = Page_BloodyPalace;
  m_full_name_string = "Bypass BP Cavaliere Restriction";
  m_author_string = "SSSiyan";
  m_description_string = "Allows you to take Cavaliere R into Bloody Palace.";

  set_up_hotkey();

  auto cavrfix1_addr = m_patterns_cache->find_addr(base, "1F 61 00 0F B6 D0 48 8B 43 50 48 8B 48 18");

  if (!cavrfix1_addr) {
    return "Unable to find cavrfix1 pattern.";
  }
  if (!install_hook_absolute(cavrfix1_addr.value()+0x06, m_cavrfix1_hook, &newmem_detour1, &jmp_cavrfix1_return, 8)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize cavrfix1";
  }
  
  
  auto cavrfix2_addr = m_patterns_cache->find_addr(base, "96 60 00 0F B6 D0 48 8B 43 50 48 8B 48 18");

  if (!cavrfix2_addr) {
    return "Unable to find cavrfix2 pattern.";
  }
  if (!install_hook_absolute(cavrfix2_addr.value()+0x06, m_cavrfix2_hook, &newmem_detour2, &jmp_cavrfix2_return, 8)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize cavrfix2";
  }
  auto cavrfix3_addr = m_patterns_cache->find_addr(base, "97 60 00 0F B6 D0 48 8B 43 50 48 8B 48 18");

  if (!cavrfix3_addr) {
      return "Unable to find cavrfix3 pattern.";
  }

  if (!install_hook_absolute(cavrfix3_addr.value() + 0x06, m_cavrfix3_hook, &newmem_detour3, &jmp_cavrfix3_return, 8)) {
      //return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize cavrfix3";
  }
  

  return Mod::on_initialize();
}

// during load
//void BypassBPCav::on_config_load(const utility::Config &cfg) {}
// during save
//void BypassBPCav::on_config_save(utility::Config &cfg) {}
// do something every frame
//void BypassBPCav::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void BypassBPCav::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
//void BypassBPCav::on_draw_ui() {}