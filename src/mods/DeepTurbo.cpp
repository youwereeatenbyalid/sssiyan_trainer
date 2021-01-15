#include "DeepTurbo.hpp"

uintptr_t DeepTurbo::jmp_ret1{NULL};
uintptr_t DeepTurbo::jmp_ret2{NULL};
uintptr_t DeepTurbo::cheaton{NULL};
float defscale   = 1.0;
float turbospeed = 1.2;
int state        = 0;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour1() {
	__asm {
        mov [rsi+68h], eax
        mov [state], rax
        mov [rsi+6Ch], r13d
		jmp qword ptr [DeepTurbo::jmp_ret1]
	}
}

static naked void detour2() {
	__asm {
        push rax
        mov rax, [DeepTurbo::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
        xor byte ptr [state], 0
        jnz code
        push rbx
        mov ebx, [turbospeed]
        mov [rax+00000388h], ebx
        movss xmm0, [rax+00000388h]
        pop rbx
        jmp qword ptr [DeepTurbo::jmp_ret2]

    code:
        push rbx
        mov ebx, [defscale]
        mov [rax+00000388h], ebx
        movss xmm0, [rax+00000388h]
        pop rbx
        jmp qword ptr [DeepTurbo::jmp_ret2]
	}
}

// clang-format on

std::optional<std::string> DeepTurbo::on_initialize() {
  ischecked          = false;
  onpage             = commonpage;
  full_name_string   = "Turbo";
  author_string      = "DeepDarkKapustka";
  description_string = "Change the game speed by adjusting the slider.\n"
                       "1.2 is the value used in DMC3 and DMC4.";
  DeepTurbo::cheaton = (uintptr_t)&ischecked;

  auto base  = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr1 = utility::scan(base, "89 46 68 44 89 6E 6C");
  if (!addr1) {
    return "Unable to find DeepTurbo1 pattern.";
  }
  auto addr2 = utility::scan(base, "F3 0F 10 80 88 03 00 00 48");
  if (!addr2) {
    return "Unable to find DeepTurbo2 pattern.";
  }

  if (!install_hook_absolute(addr1.value(), m_function_hook1, &detour1,
                             &jmp_ret1, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DeepTurbo1";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2,
                             &jmp_ret2, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DeepTurbo2";
  }
  return Mod::on_initialize();
}
void DeepTurbo::on_config_load(const utility::Config& cfg) {
  //ischecked = cfg.get<bool>("deep_turbo_custom").value_or(false);
   turbospeed = cfg.get<float>("deep_turbo_value").value_or(1.2f);
}
void DeepTurbo::on_config_save(utility::Config& cfg) {
  //cfg.set<bool>("deep_turbo_custom", ischecked);
  cfg.set<float>("deep_turbo_value", turbospeed);
}

void DeepTurbo::on_draw_ui() {
  // ModSlider turboslider = ModSlider("Mod Speed", 0.5f, 2.5f, turbospeed);
  // turboslider.draw("Turbo slider");
  ImGui::SliderFloat("Speed", &turbospeed, 0.5f, 2.5f, "%.2f");
}


void DeepTurbo::on_draw_debug_ui() {
  ImGui::Text("Deep Turbo: %.1f", turbospeed);
}
