
#include "DanteQuickSDT.hpp"
#include "PlayerTracker.hpp"

uintptr_t DanteQuickSDT::jmp_ret{NULL};
uintptr_t DanteQuickSDT::cheaton{NULL};
float sdtspeedup = 3.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code

        push rax
        mov rax,[DanteQuickSDT::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
        movss xmm0, [rdi+00000128h]
        mulss xmm0, [sdtspeedup]
		jmp qword ptr [DanteQuickSDT::jmp_ret]

    code:
        movss xmm0,[rdi+00000128h]
        jmp qword ptr [DanteQuickSDT::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> DanteQuickSDT::on_initialize() {
  ischecked            = false;
  onpage               = dantepage;
  full_name_string     = "Quick SDT";
  author_string        = "SSSiyan";
  description_string   = "SDT Bar fills quickly.";
  DanteQuickSDT::cheaton = (uintptr_t)&ischecked;
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "FF F3 0F 10 8F 24 11 00 00 F3 0F 10 87 28 01 00 00");
  if (!addr) {
    return "Unable to find DanteQuickSDT pattern.";
  }

  if (!install_hook_absolute(addr.value()+9, m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteQuickSDT";
  }
  return Mod::on_initialize();
}

void DanteQuickSDT::on_draw_ui() {
}
