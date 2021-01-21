#include "NeroSkipCS2.hpp"
#include "PlayerTracker.hpp"
uintptr_t NeroSkipCS2::jmp_ret{NULL};
uintptr_t NeroSkipCS2::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        push rax
        mov rax,[NeroSkipCS2::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [rdi+58h], 3
        jne code
        mov byte ptr [rdi+58h], 5
        mov rax, [rbx+50h]
		jmp retjmp

    code:
        inc byte ptr [rdi+58h]
    retjmp:
        mov rax, [rbx+50h]
        jmp qword ptr [NeroSkipCS2::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> NeroSkipCS2::on_initialize() {
  ischecked            = false;
  onpage               = neropage;
  full_name_string     = "Skip Charge Shot 2";
  author_string        = "SSSiyan";
  description_string   = "Bypasses Charge Shot 2 when charging Blue Rose.";
  NeroSkipCS2::cheaton = (uintptr_t)&ischecked;
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "FF 47 58 48 8B 43 50 48 39");
  if (!addr) {
    return "Unable to find NeroSkipCS2 pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroSkipCS2";
  }
  return Mod::on_initialize();
}

void NeroSkipCS2::on_draw_ui() {}
