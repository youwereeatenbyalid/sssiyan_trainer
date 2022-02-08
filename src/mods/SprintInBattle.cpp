
#include "SprintInBattle.hpp"
#include "PlayerTracker.hpp"
uintptr_t SprintInBattle::jmp_ret{NULL};
uintptr_t SprintInBattle::jmp_jne{NULL};
bool SprintInBattle::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        // cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        // jne code
        cmp byte ptr [SprintInBattle::cheaton], 1
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

void SprintInBattle::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> SprintInBattle::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &SprintInBattle::cheaton;
  m_on_page             = mechanics;

  m_full_name_string   = "Sprint In Battle";
  m_author_string      = "SSSiyan";
  m_description_string = "Allows you to Sprint in battle.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "80 BA CA 0E 00 00 00 0F 85 1E");
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
