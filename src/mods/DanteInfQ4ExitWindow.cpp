
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
		jmp qword ptr [DanteInfQ4ExitWindow::jmp_ret]

    code:
        cmp qword ptr [rax+18h], 00
        jne exitjne
        jmp qword ptr [DanteInfQ4ExitWindow::jmp_ret]

    exitjne:
        jmp qword ptr [DanteInfQ4ExitWindow::jmp_jne]
	}
}

// clang-format on

std::optional<std::string> DanteInfQ4ExitWindow::on_initialize() {
  ischecked            = &DanteInfQ4ExitWindow::cheaton;
  onpage               = dantesdt;

  full_name_string     = "Infinite Q4 SDT Exit Window";
  author_string        = "SSSiyan";
  description_string   = "Removes the time limit on the Quadruple S Exit window.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "75 2E F3 0F 5A C0 F2 0F 5C F8 66 0F 5A CF F3 0F 11 8B");
  if (!addr) {
    return "Unable to find DanteInfQ4ExitWindow pattern.";
  }
  DanteInfQ4ExitWindow::jmp_jne = utility::scan(base, "1A 00 00 00 00 00 00 0F 28 74 24 30").value();

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteInfQ4ExitWindow";
  }
  return Mod::on_initialize();
}

void DanteInfQ4ExitWindow::on_draw_ui() {
}
