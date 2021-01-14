#include "BypassBPCav.hpp"
#include "PlayerTracker.hpp"
uintptr_t BypassBPCav::jmp_ret{NULL};
uintptr_t BypassBPCav::jmp_jb{NULL};
uintptr_t BypassBPCav::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code
        push rax
        mov rax, [BypassBPCav::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

        cheatcode:
		jmp qword ptr [BypassBPCav::jmp_ret]

        code:
        cmp esi, [rax+1Ch]
        jb jbcode
        jmp qword ptr [BypassBPCav::jmp_ret]

        jbcode:
        jmp qword ptr [BypassBPCav::jmp_jb]
	}
}

// clang-format on

std::optional<std::string> BypassBPCav::on_initialize() {
  ischecked          = false;
  onpage             = dantepage;
  full_name_string   = "Bypass BP Cavaliere Restriction";
  author_string      = "SSSiyan";
  description_string = "Allows you to take cavaliere R into bloody palace.";
  BypassBPCav::cheaton = (uintptr_t)&ischecked;
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr      = utility::scan(base, "C8 00 EB 44 3B 70 1C 72 11");
  if (!addr) {
    return "Unable to find BypassBPCav pattern.";
  }
    
   BypassBPCav::jmp_jb = utility::scan(base, "42 89 0C 30 EB 28").value();

  if (!install_hook_absolute(addr.value()+4, m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize BypassBPCav";
  }
  return Mod::on_initialize();
}

void BypassBPCav::on_draw_ui() {
}
