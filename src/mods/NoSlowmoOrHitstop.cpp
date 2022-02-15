#include "NoSlowmoOrHitstop.hpp"
#include "PlayerTracker.hpp"

uintptr_t NoSlowmoOrHitstop::jmp_ret{NULL};
bool NoSlowmoOrHitstop::cheaton{NULL};

float noHitstopSpeed = 1.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [NoSlowmoOrHitstop::cheaton], 1
        je cheatcode
    code:
        movss [rdx+0x64], xmm2
		jmp qword ptr [NoSlowmoOrHitstop::jmp_ret]

    cheatcode:
        movss xmm2, [noHitstopSpeed]
        movss [rdx+0x64], xmm2
		jmp qword ptr [NoSlowmoOrHitstop::jmp_ret]
	}
}

// clang-format on

void NoSlowmoOrHitstop::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NoSlowmoOrHitstop::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &NoSlowmoOrHitstop::cheaton;
  m_on_page             = animation;
  m_full_name_string   = "No Slowmo Or Hitstop";
  m_author_string      = "SSSiyan";
  m_description_string = "Disables slowdown and hitstop.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "F3 0F 11 52 64 48 8B 41 50");
  if (!addr) {
    return "Unable to find NoSlowmoOrHitstop pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NoSlowmoOrHitstop";
  }
  return Mod::on_initialize();
}
