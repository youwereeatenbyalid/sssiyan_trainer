
#include "VergilInfSDT.hpp"
#include "utility/Scan.hpp"

uintptr_t VergilInfSDT::jmp_ret1{NULL};
uintptr_t VergilInfSDT::jmp_ret2{NULL};

float desiredsdtvalue = 10000.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour1() {
	__asm {
        movss xmm1, [desiredsdtvalue]
        movss [rbx+00001B20h], xmm1
		jmp qword ptr [VergilInfSDT::jmp_ret1]
	}
}

static naked void detour2() {
	__asm {
		jmp qword ptr [VergilInfSDT::jmp_ret2]
	}
}

// clang-format on

std::optional<std::string> VergilInfSDT::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr1 = utility::scan(base, "0F 84 BE 01 00 00 F3 0F 10 87");
  if (!addr1) {
    return "Unable to find VergilInfSDT pattern.";
  }
  auto addr2 = utility::scan(base, "0F 84 BE 01 00 00 F3 0F 10 87");
  if (!addr2) {
    return "Unable to find VergilInfSDT pattern.";
  }

  if (!install_hook_absolute(addr1.value(), m_function_hook1, &detour1, &jmp_ret1, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilInfSDT1";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilInfSDT2";
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
