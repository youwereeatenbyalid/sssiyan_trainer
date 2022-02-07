
#include "MovingTargetSwitch.hpp"

uintptr_t MovingTargetSwitch::jmp_ret{NULL};
bool MovingTargetSwitch::cheaton{NULL};
uintptr_t MovingTargetSwitch::jmp_jae{NULL};

float comissvalue = 0.75f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [MovingTargetSwitch::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		jmp qword ptr [MovingTargetSwitch::jmp_ret]

    code:
        comiss xmm0, [comissvalue]
        jae jaejmp
        jmp qword ptr [MovingTargetSwitch::jmp_ret]

    jaejmp:
        jmp qword ptr [MovingTargetSwitch::jmp_jae]
	}
}

// clang-format on

void MovingTargetSwitch::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> MovingTargetSwitch::on_initialize() {
  init_check_box_info();

  ischecked            = &MovingTargetSwitch::cheaton;
  onpage               = mechanics;

  full_name_string     = "Moving Target Switch";
  author_string        = "SSSiyan";
  description_string   = "Allows you to switch targets while moving the left stick";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns.find_addr(base, "0F 2F 05 B1 DF F6 02");
  if (!addr) {
    return "Unable to find MovingTargetSwitch pattern.";
  }

  MovingTargetSwitch::jmp_jae = patterns.find_addr(base, "3E 00 00 48 85 C9 75 12").value()+3;

  if (!install_hook_absolute(addr.value()+6, m_function_hook, &detour, &jmp_ret, 13)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize MovingTargetSwitch";
  }
  return Mod::on_initialize();
}

void MovingTargetSwitch::on_draw_ui() {
}
