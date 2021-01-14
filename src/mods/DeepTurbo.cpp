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
  full_name_string   = "Deep's Turbo";
  author_string      = "DeepDarkKapusta";
  description_string = "Change the game speed by adjusting the slider.";
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

void DeepTurbo::on_draw_debug_ui() {
  ImGui::Text("Deep Turbo: %.1f", turbospeed);
}
void DeepTurbo::on_draw_ui() {
ImGui::SliderFloat("Speed", &turbospeed, 0.5f, 2.5f, "%.2f");
}

#if 0
#include "DeepTurbo.hpp"
#include "utility/Scan.hpp"

uintptr_t DeepTurbo::jmp_ret1{NULL};
uintptr_t DeepTurbo::jmp_ret2{NULL};
bool deepturbocheck;

float defscale   = 1.0;
float turbospeed = 1.2;
int   state        = 0;

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
        cmp byte ptr [deepturbocheck], 1
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
  auto base  = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr1   = utility::scan(base, "89 46 68 44 89 6E 6C");
  if (!addr1) {
  return "Unable to find DeepTurbo1 pattern.";
  }
  auto addr2   = utility::scan(base, "F3 0F 10 80 88 03 00 00 48");
  if (!addr2) {
    return "Unable to find DeepTurbo2 pattern.";
  }

  if (!install_hook_absolute(addr1.value(), m_function_hook1, &detour1, &jmp_ret1, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DeepTurbo1";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DeepTurbo2";
  }
  return Mod::on_initialize();
}

  void DeepTurbo::on_draw_debug_ui() {
  ImGui::Text("Deep Turbo: %.1f", turbospeed);
  } 
  void DeepTurbo::on_draw_ui() {
    ImGui::Checkbox("Deep Turbo", &deepturbocheck);
  }
  
  #endif