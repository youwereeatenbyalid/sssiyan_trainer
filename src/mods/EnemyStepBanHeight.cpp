
#include "EnemyStepBanHeight.hpp"
#include "PlayerTracker.hpp"
uintptr_t EnemyStepBanHeight::jmp_ret{NULL};
bool EnemyStepBanHeight::cheaton{NULL};
float newenemystepheight = 0.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code
		cmp byte ptr [EnemyStepBanHeight::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        movss xmm0, [newenemystepheight]
        jmp qword ptr [EnemyStepBanHeight::jmp_ret]

    code:
        movss xmm0, [rax+14h]
        jmp qword ptr [EnemyStepBanHeight::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> EnemyStepBanHeight::on_initialize() {
  ischecked            = &EnemyStepBanHeight::cheaton;
  onpage               = gamemode;
  full_name_string     = "Remove Enemy Step Height Restriction";
  author_string        = "SSSiyan";
  description_string   = "Allows you to enemy step as close to the floor as possible.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "F3 0F 10 40 14 0F 5A C0 66 0F 2F F0 76");
  if (!addr) {
    return "Unable to find EnemyStepBanHeight pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemyStepBanHeight";
  }
  return Mod::on_initialize();
}

void EnemyStepBanHeight::on_draw_ui() {}
