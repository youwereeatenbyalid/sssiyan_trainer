
#include "DisableEnemyAI.hpp"
#include "PlayerTracker.hpp"

uintptr_t DisableEnemyAI::jmp_ret{NULL};
uintptr_t DisableEnemyAI::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        //cmp byte ptr [PlayerTracker::playerid], 1 //change this to the char number obviously
        //jne code
        push rax
        mov rax,[DisableEnemyAI::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
		jmp qword ptr [DisableEnemyAI::jmp_ret]

    code:
        cmp byte ptr [rax+0000009Ch], 00
        jmp qword ptr [DisableEnemyAI::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> DisableEnemyAI::on_initialize() {
  ischecked               = false;
  onpage                  = commonpage;
  full_name_string        = "Disable Enemy AI";
  author_string           = "SSSiyan";
  description_string      = "Forces enemies to act like they do when disabling Void's 'Enemy Action'.";
  DisableEnemyAI::cheaton = (uintptr_t)&ischecked;
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr =
      utility::scan(base, "80 B8 9C 00 00 00 00 74 11");
  if (!addr) {
    return "Unable to find DisableEnemyAI pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DisableEnemyAI";
  }
  return Mod::on_initialize();
}

void DisableEnemyAI::on_draw_ui() {}
