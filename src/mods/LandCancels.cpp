#include "LandCancels.hpp"
#include "PlayerTracker.hpp"

uintptr_t LandCancels::jmp_ret{NULL};
bool LandCancels::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [LandCancels::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [PlayerTracker::isgrounded], 1
        je code
        jmp retcode

    code:
        mov dword ptr [rdx+34h], 0
    retcode:
        jmp qword ptr [LandCancels::jmp_ret]
	}
}

// clang-format on

void LandCancels::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> LandCancels::on_initialize() {
  init_check_box_info();

  ischecked = &LandCancels::cheaton;
  onpage    = mechanics;

  full_name_string = "Land Cancels";
  author_string    = "SSSiyan";
  description_string = "Touching the floor will cancel your current aerial attack.";

    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "C7 42 34 00 00 00 00 C3 CC CC 48");
  if (!addr) {
    return "Unable to find LandCancels pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize LandCancels";
  }
  return Mod::on_initialize();
}

void LandCancels::on_draw_ui() {
}