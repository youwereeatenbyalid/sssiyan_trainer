
#include "DisableGauntletStages.hpp"
#include "PlayerTracker.hpp"
uintptr_t DisableGauntletStages::jmp_ret{NULL};
uintptr_t DisableGauntletStages::jmp_jne{NULL};
bool DisableGauntletStages::cheaton{NULL};


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        jne jnecode // after cmp so coop trainer still works
		cmp byte ptr [DisableGauntletStages::cheaton], 1
        je jnecode
        jmp [DisableGauntletStages::jmp_ret]

    jnecode:
        jmp qword ptr [DisableGauntletStages::jmp_jne]
	}
}

// clang-format on

void DisableGauntletStages::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DisableGauntletStages::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &DisableGauntletStages::cheaton;
  m_on_page             = bloodypalace;

  m_full_name_string   = "Skip Nero's BP Gauntlet Stages";
  m_author_string      = "Dante";
  m_description_string = "Recommended for use with Breaker Switcher or Infinite Breakers.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "0F 85 5D 02 00 00 38");
  if (!addr) {
    return "Unable to find DisableGauntletStages pattern.";
  }

  DisableGauntletStages::jmp_jne = addr.value() + 611;

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DisableGauntletStages";
  }
  return Mod::on_initialize();
}

// void DisableGauntletStages::on_draw_ui() {}
