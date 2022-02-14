#include "DanteAlwaysQ4SDT.hpp"
#include "PlayerTracker.hpp"
#include "DanteSDTRework.hpp"
uintptr_t DanteAlwaysQ4SDT::jmp_ret{NULL};
uintptr_t DanteAlwaysQ4SDT::jmp_jne{NULL};
bool DanteAlwaysQ4SDT::cheaton{NULL};


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code
		cmp byte ptr [DanteAlwaysQ4SDT::cheaton], 1
        je cheatcode
        cmp byte ptr [DanteSDTRework::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		jmp qword ptr [DanteAlwaysQ4SDT::jmp_ret]

    code:
        cmp dword ptr [rax+0xB0], 7
        jne jnecode
        jmp qword ptr [DanteAlwaysQ4SDT::jmp_ret]

    jnecode:
        jmp qword ptr [DanteAlwaysQ4SDT::jmp_jne]
	}
}

// clang-format on

void DanteAlwaysQ4SDT::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DanteAlwaysQ4SDT::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &DanteAlwaysQ4SDT::cheaton;
  m_on_page             = dantesdt;

  m_full_name_string   = "Always Quadruple S";
  m_author_string      = "SSSiyan";
  m_description_string = "Removes the style requirement for Quadruple S.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "83 B8 B0 00 00 00 07");
  if (!addr) {
    return "Unable to find DanteAlwaysQ4SDT pattern.";
  }

  DanteAlwaysQ4SDT::jmp_jne = patterns->find_addr(base, "32 C0 48 8B 5C 24 30 48 83 C4 20 5F C3 E8 C6").value();

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 13)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteAlwaysQ4SDT";
  }
  return Mod::on_initialize();
}

// void DanteAlwaysQ4SDT::on_draw_ui() {}
