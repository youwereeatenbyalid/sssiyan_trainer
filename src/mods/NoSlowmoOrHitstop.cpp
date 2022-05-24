#include "NoSlowmoOrHitstop.hpp"
#include "PlayerTracker.hpp"

uintptr_t NoSlowmoOrHitstop::jmp_ret{NULL};
uintptr_t NoSlowmoOrHitstop::jmp_ret2{NULL};
bool NoSlowmoOrHitstop::cheaton{NULL};

float noHitstopSpeed = 1.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

// hitstop
static naked void detour() { // AttackUserData->? accessed from DevilMayCry5.app_WorkRate__setHitStop74027 (DevilMayCry5.exe+CCD9B2 copyright)
	__asm {
        cmp byte ptr [NoSlowmoOrHitstop::cheaton], 1
        je cheatcode
    code:
        movss [rdx+0x64], xmm2
		jmp qword ptr [NoSlowmoOrHitstop::jmp_ret]

    cheatcode:
        movss xmm2, [noHitstopSpeed]
        movss [rdx+0x64], xmm2
		jmp qword ptr [NoSlowmoOrHitstop::jmp_ret]
	}
}
/*
static naked void detour2() { // DamageUserData->SlowTime (+B0) accessed from DevilMayCry5.app_HitController__checkJustEscape158532 (DevilMayCry5.exe+24CE8C3 copyright)
	__asm {
        cmp byte ptr [NoSlowmoOrHitstop::cheaton], 1
        je cheatcode
    code:
        comiss xmm0, [rax+0x000000B0]
		jmp qword ptr [NoSlowmoOrHitstop::jmp_ret2]

    cheatcode:
        comiss xmm0, xmm0
		jmp qword ptr [NoSlowmoOrHitstop::jmp_ret2]
	}
}
*/

// other
static naked void detour2() { // DevilMayCry5.app_WorkRateSystem__setWorkRate96344 (DevilMayCry5.exe+77EFE0 copyright)
	__asm {
        cmp byte ptr [NoSlowmoOrHitstop::cheaton], 1
        je cheatcode
    code:
        mov [rsp+0x20],rbp
		jmp qword ptr [NoSlowmoOrHitstop::jmp_ret2]

    cheatcode:
        ret
	}
}

// clang-format on

void NoSlowmoOrHitstop::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NoSlowmoOrHitstop::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &NoSlowmoOrHitstop::cheaton;
  m_on_page             = Page_Animation;
  m_full_name_string   = "No Slowmo Or Hitstop";
  m_author_string      = "SSSiyan";
  m_description_string = "Disables slowdown and hitstop.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = m_patterns_cache->find_addr(base, "F3 0F 11 52 64 48 8B 41 50");
  if (!addr) {
    return "Unable to find NoSlowmoOrHitstop pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NoSlowmoOrHitstop";
  }
  /*
  auto addr2 = m_patterns_cache->find_addr(base, "0F 2F 80 B0 00 00 00 73");
  if (!addr) {
      return "Unable to find NoSlowmoOrHitstop2 pattern.";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 7)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize NoSlowmoOrHitstop2";
  }
  */
    auto addr2 = m_patterns_cache->find_addr(base, "48 89 6C 24 20 57 41 54 41 55 41 56 41 57 48 81 EC 80");
  if (!addr2) {
      return "Unable to find NoSlowmoOrHitstop2 pattern.";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 5)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize NoSlowmoOrHitstop2";
  }
  return Mod::on_initialize();
}
