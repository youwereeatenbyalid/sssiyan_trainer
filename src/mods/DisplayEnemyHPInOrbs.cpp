/*
#include "DisplayEnemyHPInOrbs.hpp"
#include "DamageMultiplier.hpp"
#include "utility/Scan.hpp"

uintptr_t DisplayEnemyHPInOrbs::jmp_ret{NULL};
uintptr_t DisplayEnemyHPInOrbs::jmp_cont{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp dword ptr [DamageMultiplier::enemyhpvalue], 00000000
        jbe write0

        CVTTSS2SI ebp, [DamageMultiplier::enemyhpvalue]
        jmp qword ptr [DisplayEnemyHPInOrbs::jmp_cont]

        write0:
        mov ebp, [rdx+78h]
        jmp qword ptr [DisplayEnemyHPInOrbs::jmp_cont]
	}
}

// clang-format on

std::optional<std::string> DisplayEnemyHPInOrbs::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "8B 6A 78 EB 02");
  DisplayEnemyHPInOrbs::jmp_cont = base + 0x02494A0C; // @HELP

  if (!addr) {
    return "Unable to find DisplayEnemyHPInOrbs pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DisplayEnemyHPInOrbs";
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
*/