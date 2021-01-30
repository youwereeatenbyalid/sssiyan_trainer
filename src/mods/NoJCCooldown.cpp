
#include "NoJCCooldown.hpp"
#include "PlayerTracker.hpp"
uintptr_t NoJCCooldown::jmp_ret{NULL};
bool NoJCCooldown::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        // cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        // jne code
        cmp byte ptr [NoJCCooldown::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		jmp qword ptr [NoJCCooldown::jmp_ret]

    code:
        movss [rdi+00001334h], xmm0
        jmp qword ptr [NoJCCooldown::jmp_ret]
	}
}

// clang-format on

void NoJCCooldown::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NoJCCooldown::on_initialize() {
  init_check_box_info();

  ischecked          = &NoJCCooldown::cheaton;
  onpage             = enemystep;

  full_name_string   = "No Jump Cancel Cooldown";
  author_string      = "SSSiyan";
  description_string = "Removes the cooldown that starts when you enemy step, "
                       "allowing you to perform jump cancels in quicker succession.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "F3 0F 11 87 34 13 00 00 48 8B 43 50 48 83 78 18 00 0F 85 F8");
  if (!addr) {
    return "Unable to find NoJCCooldown pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NoJCCooldown";
  }
  return Mod::on_initialize();
}

void NoJCCooldown::on_draw_ui() {}
