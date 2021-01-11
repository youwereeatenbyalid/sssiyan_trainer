
#include "DanteInfQ4ExitWindow.hpp"
#include "utility/Scan.hpp"

uintptr_t DanteInfQ4ExitWindow::jmp_ret{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cvtss2sd xmm0,xmm0
		jmp qword ptr [DanteInfQ4ExitWindow::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> DanteInfQ4ExitWindow::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "75 2E F3 0F 5A C0 F2 0F 5C F8 66 0F 5A CF F3 0F 11 8B");
  if (!addr) {
    return "Unable to find DanteInfQ4ExitWindow pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteInfQ4ExitWindow";
  }
  return Mod::on_initialize();
}

// during load
// void MoveID::on_config_load(const utility::Config &cfg) {}
// during save
// void MoveID::on_config_save(utility::Config &cfg) {}
// do something every frame
// void MoveID::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void DeepTurbo::on_draw_debug_ui() {
// ImGui::Text("Deep Turbo : %.0f", turbospeed);
// }
// will show up in main window, dump ImGui widgets you want here
// void MoveID::on_draw_ui() {}
