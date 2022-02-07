#include "VergilInfJdCs.hpp"
#include "PlayerTracker.hpp"
uintptr_t VergilInfJdCs::jmp_ret{NULL};
bool VergilInfJdCs::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [VergilInfJdCs::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		jmp qword ptr [VergilInfJdCs::jmp_ret]

    code:
        inc byte ptr [rdi+000018E0h]
        jmp qword ptr [VergilInfJdCs::jmp_ret]
	}
}

// clang-format on

void VergilInfJdCs::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> VergilInfJdCs::on_initialize() {
  init_check_box_info();

  m_is_enabled            = &VergilInfJdCs::cheaton;
  m_on_page               = vergilcheat;

  m_full_name_string     = "Infinite Just Judgement Cuts";
  m_author_string        = "SSSiyan";
  m_description_string   = "Remove the consecutive Just Judgement Cut limit.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "FF 87 E0 18 00 00");
  if (!addr) {
    return "Unable to find VergilInfJdCs pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilInfJdCs";
  }
  return Mod::on_initialize();
}

void VergilInfJdCs::on_draw_ui() {
}
