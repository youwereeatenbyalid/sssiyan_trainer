#include "NeroNoDTCooldown.hpp"
#include "PlayerTracker.hpp"
uintptr_t NeroNoDTCooldown::jmp_ret{NULL};
bool NeroNoDTCooldown::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [NeroNoDTCooldown::cheaton], 1
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

void NeroNoDTCooldown::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NeroNoDTCooldown::on_initialize() {
  init_check_box_info();

  ischecked            = &NeroNoDTCooldown::cheaton;
  onpage               = nero;

  full_name_string     = "No DT Cooldown";
  author_string        = "SSSiyan";
  description_string   = "Removes the cooldown on exiting DT after entering.";

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
