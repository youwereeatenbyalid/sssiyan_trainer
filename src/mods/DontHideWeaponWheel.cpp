#include "DontHideWeaponWheel.hpp"
#include "mods/PlayerTracker.hpp"
//jumps
uintptr_t DontHideWeaponWheel::jmp_return{NULL};
uintptr_t DontHideWeaponWheel::jmp_return_vergil{ NULL };
bool DontHideWeaponWheel::cheaton{NULL};
bool DanteHide;
bool VergilHide;
//variables
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void newmem_detour() {
    __asm {
    validation:
        cmp byte ptr [DontHideWeaponWheel::cheaton], 1
        jne code
        cmp byte ptr [DanteHide], 1
        jne code
        cmp [PlayerTracker::playerid], 1
        je cheatcode
    code:
        comiss xmm1, xmm0
        movss [rdi+0x40], xmm1
        jmp qword ptr [DontHideWeaponWheel::jmp_return]

    cheatcode:
        comiss xmm1, xmm0
        jmp qword ptr [DontHideWeaponWheel::jmp_return]
    }
}

static naked void newmem_detour_vergil() { // special snowflake
    __asm {
    validation:
        cmp byte ptr [DontHideWeaponWheel::cheaton], 1
        jne code
        cmp byte ptr [VergilHide], 1
        jne code
        cmp [PlayerTracker::playerid], 4 // juust in case
        je cheatcode
    code:
        comiss xmm1, xmm0
        movss [rbx+0x40], xmm1
        jmp qword ptr[DontHideWeaponWheel::jmp_return_vergil]
    cheatcode:
        comiss xmm1, xmm0
        jmp qword ptr[DontHideWeaponWheel::jmp_return_vergil]
    }
}

// clang-format on

void DontHideWeaponWheel::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DontHideWeaponWheel::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  
  m_is_enabled = &DontHideWeaponWheel::cheaton;
  m_on_page    = camera;
  m_full_name_string     = "Never Hide Guns/Weapons HUD (+)";
  m_author_string        = "SSSiyan";
  m_description_string   = "The weapon wheel always stays out.";

  set_up_hotkey();
  auto donthideweaponandgun_addr = patterns->find_addr(base, "F3 0F 11 4F 40 77");
  auto donthideweaponandgun_addr_vergil = patterns->find_addr(base, "0F 2F C8 F3 0F 11 4B 40 77 A3");

  if (!donthideweaponandgun_addr) {
    return "Unable to find donthideweaponandgun pattern.";
  }
  if (!donthideweaponandgun_addr_vergil) {
      return "Unable to find donthideweaponandgun pattern_vergil.";
  }
  if (!install_hook_absolute(donthideweaponandgun_addr.value(), m_donthideweaponandgun_hook, &newmem_detour, &jmp_return, 5)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize donthideweaponandgun";
  }
  if (!install_hook_absolute(donthideweaponandgun_addr_vergil.value(), m_donthideweaponandgun_hook_vergil, &newmem_detour_vergil, &jmp_return_vergil, 8)) {
      //return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize donthideweaponandgun_vergil";
  }
  return Mod::on_initialize();
}

// during load
void DontHideWeaponWheel::on_config_load(const utility::Config &cfg) {
    DanteHide = cfg.get<bool>("never_hide_dante_weapon_hud").value_or(true);
    VergilHide = cfg.get<bool>("never_hide_vergil_weapon_hud").value_or(true);
}
// during save
void DontHideWeaponWheel::on_config_save(utility::Config &cfg) {
    cfg.set<bool>("never_hide_dante_weapon_hud", DanteHide);
    cfg.set<bool>("never_hide_vergil_weapon_hud", VergilHide);
}

void DontHideWeaponWheel::on_draw_ui() {
    ImGui::Checkbox("Dante", &DanteHide);
    ImGui::Checkbox("Vergil", &VergilHide);
}

// do something every frame
// void DontHideWeaponWheel::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void DontHideWeaponWheel::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
