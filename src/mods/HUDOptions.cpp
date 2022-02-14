#include "HUDOptions.hpp"
#include "PlayerTracker.hpp"

uintptr_t HUDOptions::jmp_ret{NULL};
uintptr_t HUDOptions::jmp_ret2{ NULL };
uintptr_t HUDOptions::jmp_ret3{ NULL };
bool HUDOptions::cheaton{NULL};
bool hideeverything;
bool hidehp;
bool hideorbs;
bool hidestyle;

bool danteWeaponUnhide;
bool vergilWeaponUnhide;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        // cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        // jne code
        cmp byte ptr [HUDOptions::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [hideeverything], 1
        je retcode
        cmp byte ptr [rbx+08h], 17    // Nero HP
        je hpcheck
        cmp byte ptr [rbx+08h], 10    // Dante HP
        je hpcheck
        cmp byte ptr [rbx+08h], 11    // V HP
        je hpcheck
        cmp byte ptr [rbx+08h], 6     // Vergil HP
        je hpcheck
        cmp byte ptr [rbx+08h], 5     // Orbs
        je orbcheck
        cmp byte ptr [rbx+08h], 18    // Style Meter
        je stylecheck
        jmp code

    hpcheck:
        cmp byte ptr [hidehp], 1
        je retcode
        jmp code

    orbcheck:
        cmp byte ptr [hideorbs], 1
        je retcode
        jmp code

    stylecheck:
        cmp byte ptr [hidestyle], 1
        je retcode
        jmp code

    code:
        mov [rbx+000000C4h], eax
    retcode:
        jmp qword ptr [HUDOptions::jmp_ret]
	}
}

 // WEAPON WHEELS
static naked void newmem_detour() {
    __asm {
        cmp byte ptr [HUDOptions::cheaton], 1
        jne code
        cmp byte ptr [danteWeaponUnhide], 1
        jne code
        cmp [PlayerTracker::playerid], 1
        je cheatcode
    code:
        comiss xmm1, xmm0
        movss [rdi+0x40], xmm1
        jmp qword ptr [HUDOptions::jmp_ret2]

    cheatcode:
        comiss xmm1, xmm0
        jmp qword ptr [HUDOptions::jmp_ret2]
    }
}

static naked void newmem_detour_vergil() { // special snowflake
    __asm {
        cmp byte ptr [HUDOptions::cheaton], 1
        jne code
        cmp byte ptr [vergilWeaponUnhide], 1
        jne code
        cmp [PlayerTracker::playerid], 4 // juust in case
        je cheatcode
    code:
        comiss xmm1, xmm0
        movss [rbx+0x40], xmm1
        jmp qword ptr[HUDOptions::jmp_ret3]
    cheatcode:
        comiss xmm1, xmm0
        jmp qword ptr[HUDOptions::jmp_ret3]
    }
}

// clang-format on

void HUDOptions::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> HUDOptions::on_initialize() {
  init_check_box_info();

  m_is_enabled               = &HUDOptions::cheaton  ;
  m_on_page                  = camera;

  m_full_name_string        = "HUD Options (+)";
  m_author_string           = "SSSiyan";
  m_description_string      = "Disable / Enable elements of the Heads Up Display.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "89 83 C4 00 00 00 F3");
  if (!addr) {
    return "Unable to find HUDOptions pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize HUDOptions";
  }

  // WEAPON WHEELS
  auto addr2 = patterns->find_addr(base, "F3 0F 11 4F 40 77");
  if (!addr2) {
    return "Unable to find donthideweaponandgun pattern.";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &newmem_detour, &jmp_ret2, 5)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize donthideweaponandgun";
  }

  auto addr3 = patterns->find_addr(base, "0F 2F C8 F3 0F 11 4B 40 77 A3");
  if (!addr3) {
      return "Unable to find donthideweaponandgun pattern_vergil.";
  }
  if (!install_hook_absolute(addr3.value(), m_function_hook3, &newmem_detour_vergil, &jmp_ret3, 8)) {
      //return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize donthideweaponandgun_vergil";
  }
  return Mod::on_initialize();
}

void HUDOptions::on_config_load(const utility::Config& cfg) {
  hidehp         = cfg.get<bool>("hide_hp_hud").value_or(false);
  hideorbs       = cfg.get<bool>("hide_orbs_hud").value_or(false);
  hidestyle      = cfg.get<bool>("hide_style_hud").value_or(false);
  hideeverything = cfg.get<bool>("hide_all_hud").value_or(false);
  danteWeaponUnhide = cfg.get<bool>("never_hide_dante_weapon_hud").value_or(false);
  vergilWeaponUnhide = cfg.get<bool>("never_hide_vergil_weapon_hud").value_or(false);
}
void HUDOptions::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("hide_hp_hud", hidehp);
  cfg.set<bool>("hide_orbs_hud", hideorbs);
  cfg.set<bool>("hide_style_hud", hidestyle);
  cfg.set<bool>("hide_all_hud", hideeverything);
  cfg.set<bool>("never_hide_dante_weapon_hud", danteWeaponUnhide);
  cfg.set<bool>("never_hide_vergil_weapon_hud", vergilWeaponUnhide);
}

void HUDOptions::on_draw_ui() {
  ImGui::Checkbox("Hide HP", &hidehp);
  ImGui::Checkbox("Hide Orbs", &hideorbs);
  ImGui::Checkbox("Hide Style", &hidestyle);
  ImGui::Checkbox("Hide all optional HUD", &hideeverything);
  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Checkbox("Never Hide Dante's Weapon Wheels", &danteWeaponUnhide);
  ImGui::Checkbox("Never Hide Vergil's Weapon Wheel", &vergilWeaponUnhide);
}
