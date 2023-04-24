
#include "MovingTargetSwitch.hpp"

uintptr_t MovingTargetSwitch::jmp_ret{NULL};
bool MovingTargetSwitch::cheaton{NULL};
uintptr_t MovingTargetSwitch::jmp_jae{NULL};

float comissvalue = 0.75f;
float fixvalue = 2.0f;
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [MovingTargetSwitch::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		comiss xmm0, [fixvalue]
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

  m_is_enabled            = &MovingTargetSwitch::cheaton;
  m_on_page               = Page_Mechanics;

  m_full_name_string     = "Moving Target Switch";
  m_author_string        = "SSSiyan";
  m_description_string   = "Allows you to switch targets while moving the left stick.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = m_patterns_cache->find_addr(base, "48 85 C9 74 CF F3 0F 10 41 30"); //DevilMayCry5.exe+168D0B8 (-0xA)
  if (!addr) {
    return "Unable to find MovingTargetSwitch pattern.";
  }

  MovingTargetSwitch::jmp_jae = m_patterns_cache->find_addr(base, "41 B1 01 45 33 C0 48 8B D7 48 8B CB 48 8B 5C 24 70").value()+0x20;

  if (!install_hook_absolute(addr.value() + 0xA, m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize MovingTargetSwitch";
  }
  return Mod::on_initialize();
}

// void MovingTargetSwitch::on_draw_ui(){}
