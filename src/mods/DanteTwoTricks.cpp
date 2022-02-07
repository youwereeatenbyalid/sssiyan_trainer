
#include "DanteTwoTricks.hpp"
#include "PlayerTracker.hpp"
uintptr_t DanteTwoTricks::jmp_ret{NULL};
bool DanteTwoTricks::cheaton{NULL};


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code
		cmp byte ptr [DanteTwoTricks::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		jmp qword ptr [DanteTwoTricks::jmp_ret]

    code:
        cmp byte ptr [rsi+00000098h], 00
        setne al
        jmp qword ptr [DanteTwoTricks::jmp_ret]
	}
}

// clang-format on

void DanteTwoTricks::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DanteTwoTricks::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &DanteTwoTricks::cheaton;
  m_on_page             = dantecheat;

  m_full_name_string   = "Always Get 2 Tricks";
  m_author_string      = "SSSiyan";
  m_description_string = "Allows non DT Dante to trick twice.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "80 BE 98 00 00 00 00 0F 95 C0 48 85 C9 75 9D 8B 4F 44");
  if (!addr) {
    return "Unable to find DanteTwoTricks pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 10)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteTwoTricks";
  }
  return Mod::on_initialize();
}

void DanteTwoTricks::on_draw_ui() {}
