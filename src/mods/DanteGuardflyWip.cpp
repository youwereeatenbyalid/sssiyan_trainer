
#include "DanteGuardflyWip.hpp"
#include "PlayerTracker.hpp"
uintptr_t DanteGuardflyWip::jmp_ret{NULL};
uintptr_t DanteGuardflyWip::jmp_jae{NULL};
bool DanteGuardflyWip::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code
		cmp byte ptr [DanteGuardflyWip::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		comiss xmm0, xmm2
        jne exitjae
        jmp qword ptr [DanteGuardflyWip::jmp_ret]

    code:
        comiss xmm0, xmm2
        jae exitjae
        jmp qword ptr [DanteGuardflyWip::jmp_ret]

    exitjae:
        jmp qword ptr [DanteGuardflyWip::jmp_jae]
	}
}

// clang-format on

std::optional<std::string> DanteGuardflyWip::on_initialize() {
  ischecked = &DanteGuardflyWip::cheaton;
  onpage               = mechanics;

  full_name_string     = "Guardfly WIP";
  author_string        = "socks";
  description_string   = "Removes the inertia clamp on Guard.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "0F 2F C2 0F 83 81 00 00 00");
  if (!addr) {
    return "Unable to find DanteGuardflyWip pattern.";
  }
  DanteGuardflyWip::jmp_jae = addr.value() + 138;

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 9)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteGuardflyWip";
  }
  return Mod::on_initialize();
}

void DanteGuardflyWip::on_draw_ui() {}
