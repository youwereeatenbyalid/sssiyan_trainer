
#include "DamageTypeLean.hpp"
#include "PlayerTracker.hpp"
#include "NeroSwapWiresnatch.hpp"
uintptr_t DamageTypeLean::jmp_ret{NULL};


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [PlayerTracker::playerid], 0
        je nerocode
        jmp code

    nerocode:
        push rax
        mov rax,[NeroSwapWiresnatch::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je nerocheatcode
        jmp code

    nerocheatcode:
        cmp dword ptr [rax+0xA8], 626 // Nero_SnakeSword_PowerSnatch_Lv2,
        je forcehit
        jmp code

    forcehit:
        mov ebp, 8
        jmp retcode

    code:
        mov ebp, [rax+0xC8]
    retcode:
		jmp qword ptr [DamageTypeLean::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> DamageTypeLean::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "8B A8 C8 00 00 00 E8");
  if (!addr) {
    return "Unable to find DamageTypeLean pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DamageTypeLean";
  }
  return Mod::on_initialize();
}
