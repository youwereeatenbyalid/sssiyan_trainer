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
  onpage               = gamepage;
  BufferedReversals::cheaton = (uintptr_t)&ischecked;
  full_name_string           = "Buffered Reversals";
  author_string              = "SSSiyan";
  description_string		 = "Allows you to use directional moves in any direction. "
							   "During the recovery of a move, buffer a directional attack, then "
							   "push the stick in a new direction and let go of lock on before the "
							   "buffered attack comes out to change the direction it points.";
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
