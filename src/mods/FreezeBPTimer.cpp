
#include "FreezeBPTimer.hpp"
#include "utility/Scan.hpp"

uintptr_t FreezeBPTimer::jmp_ret{NULL};
bool freezebptimercheck;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [freezebptimercheck], 1
        je cheatcode
        jmp code

    cheatcode:
        cvtpd2ps xmm1, xmm1
		jmp qword ptr [FreezeBPTimer::jmp_ret]

    code:
        subsd xmm1,xmm0
        cvtpd2ps xmm1,xmm1
        jmp qword ptr [FreezeBPTimer::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> FreezeBPTimer::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr      = utility::scan(base, "F2 0F 5C C8 66 0F 5A C9 F3 0F 11 4B 10");
  if (!addr) {
    return "Unable to find FreezeBPTimer pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize FreezeBPTimer";
  }
  return Mod::on_initialize();
}

void FreezeBPTimer::on_draw_ui() {
  ImGui::Checkbox("Freeze BP Timer", &freezebptimercheck);
}
