
#include "BufferedReversals.hpp"
#include "utility/Scan.hpp"

uintptr_t BufferedReversals::jmp_ret{NULL};
bool bufferedreversalscheck;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
		cmp byte ptr [bufferedreversalscheck], 1
		je bufferedreversals
		jmp code

	bufferedreversals:
		add [rdi+1740h], rax
		jmp qword ptr [BufferedReversals::jmp_ret]

	code:
		mov [rdi+1740h], rax
		jmp qword ptr [BufferedReversals::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> BufferedReversals::on_initialize() {
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
  ImGui::Checkbox("Buffered Reversals", &bufferedreversalscheck);
}
