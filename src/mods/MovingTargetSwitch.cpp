
#include "MovingTargetSwitch.hpp"

uintptr_t MovingTargetSwitch::jmp_ret{NULL};
uintptr_t MovingTargetSwitch::cheaton{NULL};
uintptr_t MovingTargetSwitch::jmp_jae{NULL};
bool movingtargetswitchcheck;

float comissvalue = 0.75f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        push rax
        mov rax, [MovingTargetSwitch::cheaton]
        cmp byte ptr [rax], 1
        pop rax
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

std::optional<std::string> MovingTargetSwitch::on_initialize() {
  ischecked            = false;
  onpage               = commonpage;
  full_name_string     = "Moving Target Switch";
  author_string        = "SSSiyan";
  description_string   = "Allows you to switch targets while moving the left stick";
  MovingTargetSwitch::cheaton = (uintptr_t)&ischecked;
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "0F 2F 05 81 DF F6 02");
  if (!addr) {
    return "Unable to find MovingTargetSwitch pattern.";
  }

  MovingTargetSwitch::jmp_jae = utility::scan(base, "3E 00 00 48 85 C9 75 12").value()+3;

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 13)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize MovingTargetSwitch";
  }
  return Mod::on_initialize();
}

void MovingTargetSwitch::on_draw_ui() {
  ImGui::Checkbox("Moving Target Switch", &movingtargetswitchcheck);
}
