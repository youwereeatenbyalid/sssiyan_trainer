#include "NeroInfPunchline.hpp"
#include "PlayerTracker.hpp"
uintptr_t NeroInfPunchline::jmp_ret{NULL};
uintptr_t NeroInfPunchline::jmp_jnl{NULL};
bool NeroInfPunchline::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [NeroInfPunchline::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		jmp jnlcode

    code:
        cmp dword ptr [rax+000000B0h],05
        jnl jnlcode
        jmp qword ptr [NeroInfPunchline::jmp_ret]

    jnlcode:
        jmp qword ptr [NeroInfPunchline::jmp_jnl]
	}
}

// clang-format on

void NeroInfPunchline::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NeroInfPunchline::on_initialize() {
  init_check_box_info();

  m_is_enabled            = &NeroInfPunchline::cheaton;
  m_on_page               = nero;

  m_full_name_string     = "Infinite Punchline Duration";
  m_author_string        = "SSSiyan";
  m_description_string   = "Removes the ride timer on Punchline.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "83 B8 B0 00 00 00 05");
  if (!addr) {
    return "Unable to find NeroInfPunchline pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 9)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroInfPunchline";
  }
  NeroInfPunchline::jmp_jnl = addr.value() + 67;

  return Mod::on_initialize();
}

void NeroInfPunchline::on_draw_ui() {}
