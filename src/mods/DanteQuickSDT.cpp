
#include "DanteQuickSDT.hpp"


uintptr_t DanteQuickSDT::jmp_ret{NULL};
bool dantequicksdtcheck;

float sdtspeedup = 3.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [dantequicksdtcheck], 1
        je cheatcode
        jmp code

    cheatcode:
        movss xmm0, [rdi+00000128h]
        mulss xmm0, [sdtspeedup]
		jmp qword ptr [DanteQuickSDT::jmp_ret]

    code:
        movss xmm0,[rdi+00000128h]
        jmp qword ptr [DanteQuickSDT::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> DanteQuickSDT::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "FF F3 0F 10 8F 24 11 00 00 F3 0F 10 87 28 01 00 00");
  if (!addr) {
    return "Unable to find DanteQuickSDT pattern.";
  }

  if (!install_hook_absolute(addr.value()+9, m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteQuickSDT";
  }
  return Mod::on_initialize();
}

void DanteQuickSDT::on_draw_ui() {
  ImGui::Checkbox("Dante Quick SDT", &dantequicksdtcheck);
}
