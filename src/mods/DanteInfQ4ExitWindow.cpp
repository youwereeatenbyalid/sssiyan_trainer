
#include "DanteInfQ4ExitWindow.hpp"
#include "PlayerTracker.hpp"
uintptr_t DanteInfQ4ExitWindow::jmp_ret{NULL};
uintptr_t DanteInfQ4ExitWindow::jmp_jne{NULL};
bool DanteInfQ4ExitWindow::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code
		cmp byte ptr [DanteInfQ4ExitWindow::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		jmp qword ptr [DanteInfQ4ExitWindow::jmp_jne]

    code:
        cmp qword ptr [rax+18h], 00
        jne exitjne
        jmp qword ptr [DanteInfQ4ExitWindow::jmp_ret]

    exitjne:
        jmp qword ptr [DanteInfQ4ExitWindow::jmp_jne]
	}
}

// clang-format on

void DanteInfQ4ExitWindow::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DanteInfQ4ExitWindow::on_initialize() {
  init_check_box_info();

  m_is_enabled            = &DanteInfQ4ExitWindow::cheaton;
  m_on_page               = dantesdt;

  m_full_name_string     = "Infinite Sin Devil Trigger Exit Window";
  m_author_string        = "SSSiyan";
  m_description_string   = "Removes the time limit on the Quadruple S Exit window.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "47 50 48 83 78 18 00 75 2E F3 0F 5A");
  if (!addr) {
    return "Unable to find DanteInfQ4ExitWindow pattern.";
  }
  DanteInfQ4ExitWindow::jmp_jne = addr.value() + 55;

  if (!install_hook_absolute(addr.value()+2, m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteInfQ4ExitWindow";
  }
  return Mod::on_initialize();
}

// void DanteInfQ4ExitWindow::on_draw_ui() {}
