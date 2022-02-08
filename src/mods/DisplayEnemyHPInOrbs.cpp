
#include "DisplayEnemyHPInOrbs.hpp"
#include "DamageMultiplier.hpp"

uintptr_t DisplayEnemyHPInOrbs::jmp_ret{NULL};
uintptr_t DisplayEnemyHPInOrbs::jmp_cont{NULL};
bool DisplayEnemyHPInOrbs::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [DisplayEnemyHPInOrbs::cheaton], 1
        je healthcheck
        jmp code

    healthcheck:
        cmp dword ptr [DamageMultiplier::enemyhpvalue], 80000000h // write default orbs if enemy hp is less than 0 (-0)
        ja code
        cmp dword ptr [DamageMultiplier::enemyhpvalue], 00000000h // write default orbs if enemy hp is equal to 0 (0)
        je code
        jmp cheatcode

    cheatcode:
        CVTTSS2SI ebp, [DamageMultiplier::enemyhpvalue]
        jmp qword ptr [DisplayEnemyHPInOrbs::jmp_cont]

        code:
        mov ebp, [rdx+78h]
        jmp qword ptr [DisplayEnemyHPInOrbs::jmp_cont]
	}
}

// clang-format on

void DisplayEnemyHPInOrbs::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DisplayEnemyHPInOrbs::on_initialize() {
  init_check_box_info();

  m_is_enabled            = &DisplayEnemyHPInOrbs::cheaton;
  m_on_page               = camera;

  m_full_name_string     = "Display Enemy HP in Orbs";
  m_author_string        = "SSSiyan";
  m_description_string   = "Displays the last hit enemy's HP in the orb counter on the top right.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "8B 6A 78 EB 02");
  DisplayEnemyHPInOrbs::jmp_cont = patterns->find_addr(base, "44 8B 05 D5 FC 9E 05").value();
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
