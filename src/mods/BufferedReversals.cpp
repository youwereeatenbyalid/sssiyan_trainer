
#include "BufferedReversals.hpp"

uintptr_t BufferedReversals::jmp_ret{NULL};
uintptr_t BufferedReversals::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
		push rax
		mov rax, [BufferedReversals::cheaton]
		cmp byte ptr [rax], 1
		pop rax

		je cheatcode
		jmp code

	cheatcode:
		add [rdi+0x1740], rax
		jmp qword ptr [BufferedReversals::jmp_ret]

	code:
		mov [rdi+0x1740], rax
		jmp qword ptr [BufferedReversals::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> BufferedReversals::on_initialize() {
  
  ischecked            = false;
  onpage               = commonpage;
  BufferedReversals::cheaton = (uintptr_t)&ischecked;

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "48 89 87 40 17 00 00");
  if (!addr) {
    return "Unable to find BufferedReversals pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize BufferedReversals";
  }
  return Mod::on_initialize();
}

void BufferedReversals::on_draw_ui() {
}
