#include "NeroNoDTCooldown.hpp"
#include "PlayerTracker.hpp"
uintptr_t NeroNoDTCooldown::jmp_ret{NULL};
uintptr_t NeroNoDTCooldown::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        push rax
        mov rax,[NeroNoDTCooldown::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
        jmp qword ptr [NeroNoDTCooldown::jmp_ret]

    code:
        mov [rdi+0000111Ch], eax
    retjmp:
        jmp qword ptr [NeroNoDTCooldown::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> NeroNoDTCooldown::on_initialize() {
  ischecked            = false;
  onpage               = neropage;
  full_name_string     = "No DT Cooldown";
  author_string        = "SSSiyan";
  description_string   = "Removes the cooldown on exiting DT after entering.";
  NeroNoDTCooldown::cheaton = (uintptr_t)&ischecked;
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "89 87 1C 11 00 00 48 8B 43 50 48 83");
  if (!addr) {
    return "Unable to find NeroNoDTCooldown pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroNoDTCooldown";
  }
  return Mod::on_initialize();
}

void NeroNoDTCooldown::on_draw_ui() {}
