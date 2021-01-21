
#include "SprintInBattle.hpp"
#include "PlayerTracker.hpp"
uintptr_t SprintInBattle::jmp_ret{NULL};
uintptr_t SprintInBattle::jmp_jne{NULL};
uintptr_t SprintInBattle::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        // cmp byte ptr [PlayerTracker::playerid], 0 //change this to the char number obviously
        // jne code
        push rax
        mov rax, [SprintInBattle::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
        jmp retcode

    code:
        cmp byte ptr [rdx+00000ECAh], 00
        jne jnejmp
    retcode:
        jmp qword ptr [SprintInBattle::jmp_ret]

    jnejmp:
        jmp qword ptr [SprintInBattle::jmp_jne]
	}
}

// clang-format on

std::optional<std::string> SprintInBattle::on_initialize() {
  ischecked          = false;
  onpage             = gamepage;
  full_name_string   = "Sprint In Battle";
  author_string      = "SSSiyan";
  description_string = "Allows you to Sprint in battle.";
  SprintInBattle::cheaton = (uintptr_t)&ischecked;
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "80 BA CA 0E 00 00 00 0F 85 1E");
  if (!addr) {
    return "Unable to find SprintInBattle pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 13)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize SprintInBattle";
  }

  SprintInBattle::jmp_jne = addr.value() + 299;

  return Mod::on_initialize();
}

void SprintInBattle::on_draw_ui() {}
