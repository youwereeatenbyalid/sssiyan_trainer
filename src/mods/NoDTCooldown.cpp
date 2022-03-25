#include "NoDTCooldown.hpp"
#include "PlayerTracker.hpp"
uintptr_t NoDTCooldown::jmp_ret{NULL};
uintptr_t NoDTCooldown::jmp_ret2{ NULL };
uintptr_t NoDTCooldown::jmp_ja2{ NULL };
uintptr_t NoDTCooldown::jmp_ret3{ NULL };

bool NoDTCooldown::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() { // Nero
	__asm {
        cmp [PlayerTracker::playerid], 0
        jne code
        cmp byte ptr [NoDTCooldown::cheaton], 1
        je retjmp
    code:
        mov [rdi+0000111Ch], eax
    retjmp:
        jmp qword ptr [NoDTCooldown::jmp_ret]
	}
}

  static naked void detour2() { // V
    __asm {
        cmp [PlayerTracker::playerid], 2
        jne code
        cmp byte ptr [NoDTCooldown::cheaton], 1
        je retcode
    code:
        comiss xmm0, xmm6
        ja ja_return
    retcode:
        jmp qword ptr [NoDTCooldown::jmp_ret2]

    ja_return:
        jmp qword ptr [NoDTCooldown::jmp_ja2]
  }
}

    static naked void detour3() { // Dante
    __asm {
        cmp [PlayerTracker::playerid], 1
        jne code
        cmp byte ptr [NoDTCooldown::cheaton], 1
        je retcode
    code:
        mov [rdi+0x0000111C], eax
    retcode:
        jmp qword ptr [NoDTCooldown::jmp_ret3]
  }
}

// clang-format on

void NoDTCooldown::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NoDTCooldown::on_initialize() {
  init_check_box_info();

  m_is_enabled            = &NoDTCooldown::cheaton;
  m_on_page               = Page_Mechanics;

  m_full_name_string     = "No DT Cooldown";
  m_author_string        = "SSSiyan";
  m_description_string   = "Removes the cooldown on exiting DT after entering.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = m_patterns_cache->find_addr(base, "89 87 1C 11 00 00 48 8B 43 50 48 83");
  if (!addr) {
    return "Unable to find NoDTCooldownNero pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NoDTCooldownNero";
  }

  auto addr2 = m_patterns_cache->find_addr(base, "0F 87 A6 00 00 00 48 8B 15");
  if (!addr2) {
      return "Unable to find NoDTCooldownV pattern.";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 6)) {
      //return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize NoDTCooldownV";
  }
  NoDTCooldown::jmp_ja2 = addr2.value() + 0xAC;

  auto addr3 = m_patterns_cache->find_addr(base, "89 87 1C 11 00 00 48 8B 43 50 48");
  if (!addr3) {
      return "Unable to find NoDTCooldownV pattern.";
  }
  if (!install_hook_absolute(addr3.value(), m_function_hook3, &detour3, &jmp_ret3, 6)) {
      //return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize NoDTCooldownV";
  }

  return Mod::on_initialize();
}

// void NeroNoDTCooldown::on_draw_ui() {}
