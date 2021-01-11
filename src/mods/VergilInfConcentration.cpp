
#include "VergilInfConcentration.hpp"
#include "utility/Scan.hpp"

uintptr_t VergilInfConcentration::jmp_ret{NULL};

float maxconcentration = 300.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        movss xmm2, [maxconcentration]
        movss [rbx+00001B50h], xmm2
		jmp qword ptr [VergilInfConcentration::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> VergilInfConcentration::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "F3 0F 10 93 50 1B 00 00");
  if (!addr) {
    return "Unable to find VergilInfConcentration pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilInfConcentration";
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
