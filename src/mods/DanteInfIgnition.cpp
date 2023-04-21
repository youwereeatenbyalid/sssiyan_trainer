#include "DanteInfIgnition.hpp"
#include "PlayerTracker.hpp"
uintptr_t DanteInfIgnition::jmp_ret{NULL};
uintptr_t DanteInfIgnition::jmp_ret2{NULL};
bool DanteInfIgnition::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() { // forces ignition 1 and timer 600
    __asm {
        cmp [PlayerTracker::playerid], 1
        jne code
        cmp byte ptr [DanteInfIgnition::cheaton], 1
        je cheatcode
        jmp code

     cheatcode :
        mov byte ptr [rdi+0x360], 1
        mov dword ptr [rdi+0x368], 0x44160000 // 600.0f

    code :
        mov eax, [rdi+0x00000360]
        jmp qword ptr [DanteInfIgnition::jmp_ret]
    }
}

static naked void detour2() { // disables anything turning ignition off by forcing al in DevilMayCry5.app_WeaponBalrog__setBurningMode221563 (DevilMayCry5.exe+C1EC34 copyright)
    __asm {
        cmp [PlayerTracker::playerid], 1
        jne code
        cmp byte ptr [DanteInfIgnition::cheaton], 1
        je cheatcode
        jmp code

     cheatcode :
        mov al,1
        jmp qword ptr [DanteInfIgnition::jmp_ret2]

    code:
        test rax,rax
        sete al
        jmp qword ptr [DanteInfIgnition::jmp_ret2]
    }
}

// clang-format on

void DanteInfIgnition::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DanteInfIgnition::on_initialize() {
  init_check_box_info();

  m_is_enabled            = &DanteInfIgnition::cheaton;
  m_on_page               = Page_DanteCheat;

  m_full_name_string     = "Infinite Ignition";
  m_author_string        = "SSSiyan";
  m_description_string   = "Forces Balrog's Ignition.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  
  auto addr = m_patterns_cache->find_addr(base, "8B 87 60 03 00 00 83");
  if (!addr) {
    return "Unable to find DanteInfIgnition pattern.";
  }
  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteInfIgnition";
  }

  auto addr2 = m_patterns_cache->find_addr(base, "48 85 C0 0F 94 C0 84 C0 0F 85 60");
  if (!addr2) {
    return "Unable to find DanteInfIgnition2 pattern.";
  }
  if (!install_new_detour(addr2.value(), m_detour2, &detour2, &jmp_ret2, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteInfIgnition2";
  }

  return Mod::on_initialize();
}

// void DanteInfIgnition::on_draw_ui() {}
