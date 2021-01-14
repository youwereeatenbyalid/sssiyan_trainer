
#include "DisplayEnemyHPInOrbs.hpp"
#include "DamageMultiplier.hpp"

uintptr_t DisplayEnemyHPInOrbs::jmp_ret{NULL};
uintptr_t DisplayEnemyHPInOrbs::jmp_cont{NULL};
uintptr_t DisplayEnemyHPInOrbs::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        push rax
        mov rax, [DisplayEnemyHPInOrbs::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je code
        jmp cheatcode

    cheatcode:
        cmp dword ptr [DamageMultiplier::enemyhpvalue], 00000000
        jbe code
        CVTTSS2SI ebp, [DamageMultiplier::enemyhpvalue]
        jmp qword ptr [DisplayEnemyHPInOrbs::jmp_cont]

        code:
        mov ebp, [rdx+78h]
        jmp qword ptr [DisplayEnemyHPInOrbs::jmp_cont]
	}
}

// clang-format on

std::optional<std::string> DisplayEnemyHPInOrbs::on_initialize() {
  ischecked            = false;
  onpage               = commonpage;
  full_name_string     = "Display Enemy HP in Orbs";
  author_string        = "SSSiyan";
  description_string = "This should display the enemies HP in the orb counter on the top right,\n but it doesn't. I blame Siyan.";
  DisplayEnemyHPInOrbs::cheaton = (uintptr_t)&ischecked;
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "8B 6A 78 EB 02");
  DisplayEnemyHPInOrbs::jmp_cont = utility::scan(base, "44 8B 05 D5 FC 9E 05").value();
  // DisplayEnemyHPInOrbs::jmp_cont = (base + 0x02494A0C); // ?? 

  if (!addr) {
    return "Unable to find DisplayEnemyHPInOrbs pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DisplayEnemyHPInOrbs";
  }
  return Mod::on_initialize();
}

  void DisplayEnemyHPInOrbs::on_draw_ui() {
  }
