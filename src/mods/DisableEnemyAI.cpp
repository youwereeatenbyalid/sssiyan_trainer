
#include "DisableEnemyAI.hpp"
#include "PlayerTracker.hpp"

uintptr_t DisableEnemyAI::jmp_ret{NULL};
bool DisableEnemyAI::cheaton{NULL};
uintptr_t DisableEnemyAI::jmp_je{NULL};

    // clang-format off
    // only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        //cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        //jne code
        cmp byte ptr [DisableEnemyAI::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		jmp qword ptr [DisableEnemyAI::jmp_ret]

    code:
        cmp byte ptr [rax+0000009Ch], 00
        je  jejmp
        jmp qword ptr [DisableEnemyAI::jmp_ret]

    jejmp:
        jmp qword ptr [DisableEnemyAI::jmp_je]
	}
}

// clang-format on

void DisableEnemyAI::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DisableEnemyAI::on_initialize() {
  init_check_box_info();

  m_is_enabled               = &DisableEnemyAI::cheaton;
  m_on_page                  = commoncheat;

  m_full_name_string        = "Disable Enemy AI";
  m_author_string           = "SSSiyan";
  m_description_string      = "Forces enemies to act like they do when disabling Void's 'Enemy Action'.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "80 B8 9C 00 00 00 00 74 11");
  if (!addr) {
    return "Unable to find DisableEnemyAI pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 9)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DisableEnemyAI";
  }

  DisableEnemyAI::jmp_je = addr.value() + 26;

  return Mod::on_initialize();
}

// void DisableEnemyAI::on_draw_ui() {}
