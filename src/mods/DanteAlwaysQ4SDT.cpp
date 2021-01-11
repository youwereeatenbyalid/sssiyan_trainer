
#include "DanteAlwaysQ4SDT.hpp"
#include "utility/Scan.hpp"

uintptr_t DanteAlwaysQ4SDT::jmp_ret{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
		jmp qword ptr [DanteAlwaysQ4SDT::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> DanteAlwaysQ4SDT::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "0F 85 50 FF FF FF 48 8B 87 08");
  if (!addr) {
    return "Unable to find DanteAlwaysQ4SDT pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteAlwaysQ4SDT";
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
