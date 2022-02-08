#include "BypassBPCav.hpp"
#include "PlayerTracker.hpp"
uintptr_t BypassBPCav::jmp_ret{NULL};
uintptr_t BypassBPCav::jmp_jb{NULL};
bool BypassBPCav::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        // cmp [PlayerTracker::playerid], 1 // "dante" player isn't active when this is called
        // jne code
        cmp byte ptr [BypassBPCav::cheaton], 1
        je cheatcode
        jmp code

        cheatcode:
		jmp qword ptr [BypassBPCav::jmp_ret]

        code:
        cmp esi, [rax+1Ch]
        jb jbcode
        jmp qword ptr [BypassBPCav::jmp_ret]

        jbcode:
        jmp qword ptr [BypassBPCav::jmp_jb]
	}
}

// clang-format on

void BypassBPCav::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> BypassBPCav::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &BypassBPCav::cheaton;
  m_on_page             = bloodypalace;

  m_full_name_string   = "Bypass BP Cavaliere Restriction";
  m_author_string      = "SSSiyan";
  m_description_string = "Allows you to take Cavaliere R into Bloody Palace.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr      = patterns->find_addr(base, "C8 00 EB 44 3B 70 1C 72 11");
  if (!addr) {
    return "Unable to find BypassBPCav pattern.";
  }
    
   BypassBPCav::jmp_jb = patterns->find_addr(base, "42 89 0C 30 EB 28").value();

  if (!install_hook_absolute(addr.value()+4, m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize BypassBPCav";
  }
  return Mod::on_initialize();
}

void BypassBPCav::on_draw_ui() {
}
