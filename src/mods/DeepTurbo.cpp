
#include "DeepTurbo.hpp"
#include "utility/Scan.hpp"

uintptr_t DeepTurbo::jmp_ret1{NULL};
uintptr_t DeepTurbo::jmp_ret2{NULL};

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
        xor byte ptr [state], 0
        jnz originalcode
        push rbx
        mov ebx, [turbospeed]
        mov [rax+00000388h], ebx
        movss xmm0, [rax+00000388h]
        pop rbx
        jmp qword ptr [DeepTurbo::jmp_ret2]

        originalcode:
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

// during load
// void MoveID::on_config_load(const utility::Config &cfg) {}
// during save
// void MoveID::on_config_save(utility::Config &cfg) {}
// do something every frame
// void MoveID::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
  void DeepTurbo::on_draw_debug_ui() {
  ImGui::Text("Deep Turbo: %.1f", turbospeed);
  }
// will show up in main window, dump ImGui widgets you want here
// void MoveID::on_draw_ui() {}
