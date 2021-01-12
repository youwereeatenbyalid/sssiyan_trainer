
#include "DanteAlwaysQ4SDT.hpp"

uintptr_t DanteAlwaysQ4SDT::jmp_ret{NULL};
uintptr_t DanteAlwaysQ4SDT::jmp_jne{NULL};
bool dantealwaysq4sdtcheck;


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [dantealwaysq4sdtcheck], 1
        je cheatcode
        jmp code

    cheatcode:
		jmp qword ptr [DanteAlwaysQ4SDT::jmp_ret]

    code:
        cmp dword ptr [rax+0xB0], 7
        jne jnecode
        jmp qword ptr [DanteAlwaysQ4SDT::jmp_ret]

    jnecode:
        jmp qword ptr [DanteAlwaysQ4SDT::jmp_jne]
	}
}

// clang-format on

std::optional<std::string> DanteAlwaysQ4SDT::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "83 B8 B0 00 00 00 07");
  if (!addr) {
    return "Unable to find DanteAlwaysQ4SDT pattern.";
  }

  DanteAlwaysQ4SDT::jmp_jne = utility::scan(base, "32 C0 48 8B 5C 24 30 48 83 C4 20 5F C3 E8 C6").value();

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 13)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteAlwaysQ4SDT";
  }
  return Mod::on_initialize();
}

void DanteAlwaysQ4SDT::on_draw_ui() {
  ImGui::Checkbox("Dante Always Q4SDT", &dantealwaysq4sdtcheck);
}
