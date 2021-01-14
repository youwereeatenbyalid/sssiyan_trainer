
#include "NoScreenShake.hpp"

uintptr_t NoScreenShake::jmp_ret{NULL};
uintptr_t NoScreenShake::cheaton{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        push rax
        mov rax, [NoScreenShake::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
        ret

    code:
        mov [rsp+18h], rbx
		jmp qword ptr [NoScreenShake::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> NoScreenShake::on_initialize() {
  ischecked            = false;
  onpage               = commonpage;
  full_name_string     = "No Screen Shake";
  author_string        = "SSSiyan";
  description_string   = "Disables the screen shake effect";
  NoScreenShake::cheaton = (uintptr_t)&ischecked;

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr      = utility::scan(base, "00 CC CC CC CC CC CC CC 48 89 5C 24 18 56 57");
  if (!addr) {
    return "Unable to find NoScreenShake pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NoScreenShake";
  }
  return Mod::on_initialize();
}

void NoScreenShake::on_draw_ui() {
}
