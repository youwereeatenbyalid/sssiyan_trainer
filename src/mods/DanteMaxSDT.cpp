
#include "DanteMaxSDT.hpp"
#include "utility/Scan.hpp"

uintptr_t DanteMaxSDT::jmp_ret{NULL};
bool dantemaxsdtcheck;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [dantemaxsdtcheck], 1
        je cheatcode
        jmp code

    cheatcode:
		mov dword ptr [rdi+00001A14h], 461c4000h // 10,000
        movss xmm1, [rdi+00001A14h]
		jmp qword ptr [DanteMaxSDT::jmp_ret]

    code:
        movss xmm1,[rdi+00001A14h]
        jmp qword ptr [DanteMaxSDT::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> DanteMaxSDT::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr      = utility::scan(base, "F3 0F 10 8F 14 1A 00 00 BA");
  if (!addr) {
    return "Unable to find DanteMaxSDT pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteMaxSDT";
  }
  return Mod::on_initialize();
}

void DanteMaxSDT::on_draw_ui() {
  ImGui::Checkbox("Dante Max SDT", &dantemaxsdtcheck);
}
