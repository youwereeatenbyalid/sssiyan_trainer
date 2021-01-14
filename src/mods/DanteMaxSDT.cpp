#include "DanteMaxSDT.hpp"
#include "PlayerTracker.hpp"
uintptr_t DanteMaxSDT::jmp_ret{NULL};
uintptr_t DanteMaxSDT::cheaton{NULL};

    // clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
    validation:
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code

        push rax
        mov rax, [DanteMaxSDT::cheaton]
        cmp byte ptr [rax], 1
        pop rax

        je cheatcode
        jmp code

    cheatcode:
		mov dword ptr [rdi+0x00001A14], 0x461c4000 // 10,000
        movss xmm1, [rdi+0x00001A14]
		jmp qword ptr [DanteMaxSDT::jmp_ret]

    code:
        movss xmm1,[rdi+0x00001A14]
        jmp qword ptr [DanteMaxSDT::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> DanteMaxSDT::on_initialize() {
  ischecked            = false;
  onpage               = dantepage;
  full_name_string     = "Max SDT";
  author_string        = "SSSiyan";
  description_string   = "Set SDT Bar to maximum.";
  DanteMaxSDT::cheaton = (uintptr_t)&ischecked;
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "F3 0F 10 8F 14 1A 00 00 BA");
  if (!addr) {
    return "Unable to find DanteMaxSDT pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteMaxSDT";
  }
  return Mod::on_initialize();
}

void DanteMaxSDT::on_draw_ui() {
  //ImGui::Checkbox("Dante Max SDT", &dantemaxsdtcheck);
}
