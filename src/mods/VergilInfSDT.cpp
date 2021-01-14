
#include "VergilInfSDT.hpp"

uintptr_t VergilInfSDT::jmp_ret1{NULL};
uintptr_t VergilInfSDT::jmp_ret2{NULL};
uintptr_t VergilInfSDT::cheaton{NULL};

float desiredsdtvalue = 10000.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour1() {
	__asm {
        push rax
        mov rax, [VergilInfSDT::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
        movss xmm1, [desiredsdtvalue]
        movss [rbx+00001B20h], xmm1
		jmp qword ptr [VergilInfSDT::jmp_ret1]

    code:
        movss xmm1,[rbx+00001B20h]
		jmp qword ptr [VergilInfSDT::jmp_ret1]
	}
}

static naked void detour2() {
	__asm {
        push rax
        mov rax, [VergilInfSDT::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
		jmp qword ptr [VergilInfSDT::jmp_ret2]

    code:
        movss [rdi+00001B20h], xmm0
        jmp qword ptr [VergilInfSDT::jmp_ret2]
	}
}

// clang-format on

std::optional<std::string> VergilInfSDT::on_initialize() {
  ischecked            = false;
  onpage               = vergilpage;
  full_name_string     = "Infinite SDT";
  author_string        = "SSSiyan";
  description_string   = "Grant infinite SDT.";
  VergilInfSDT::cheaton = (uintptr_t)&ischecked;
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr1 = utility::scan(base, "F3 0F 10 8B 20 1B 00 00 8B");
  if (!addr1) {
    return "Unable to find VergilInfSDT pattern.";
  }
  auto addr2 = utility::scan(base, "F3 0F 11 87 20 1B 00 00 48 8B 43 50 48");
  if (!addr2) {
    return "Unable to find VergilInfSDT pattern.";
  }

  if (!install_hook_absolute(addr1.value(), m_function_hook1, &detour1, &jmp_ret1, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilInfSDT1";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilInfSDT2";
  }
  return Mod::on_initialize();
}

void VergilInfSDT::on_draw_ui() {
}
