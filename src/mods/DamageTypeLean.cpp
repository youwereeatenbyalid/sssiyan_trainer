
#include "DamageTypeLean.hpp"
#include "PlayerTracker.hpp"
#include "NeroSwapWiresnatch.hpp"
uintptr_t DamageTypeLean::jmp_ret{NULL};


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 0
        je nerocode
        jmp code

    nerocode:
		cmp byte ptr [NeroSwapWiresnatch::cheaton], 1
        je nerocheatcode
        jmp code

    nerocheatcode:
        cmp dword ptr [rax+0xA8], 626 // Nero_SnakeSword_PowerSnatch_Lv2,
        je forcehit
        jmp code

    forcehit:
        mov ebp, 8
        jmp retcode

    code:
        mov ebp, [rax+0xC8]
    retcode:
		jmp qword ptr [DamageTypeLean::jmp_ret]
	}
}

// clang-format on

void DamageTypeLean::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DamageTypeLean::on_initialize() {
  // init_check_box_info();

  // m_is_enabled = &DamageTypeLean::cheaton;
  // m_on_page = mechanics;

  // m_full_name_string = "Knockback Edits";
  // m_author_string = "Siyan";
  // m_description_string = "Add or remove knockbacks/launches etc from moves.";

  // set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = m_patterns_cache->find_addr(base, "8B A8 C8 00 00 00 E8");
  if (!addr) {
    return "Unable to find DamageTypeLean pattern.";
  }

  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DamageTypeLean";
  }
  return Mod::on_initialize();
}
