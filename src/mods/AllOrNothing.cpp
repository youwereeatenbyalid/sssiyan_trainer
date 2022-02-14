#include "AllOrNothing.hpp"
#include "mods/PlayerTracker.hpp"
#include "mods/HeavyDay.hpp"
#include "mods/StyleRank.hpp"
#include "mods/OneHitKill.hpp"
#include "mods/NoOneTakesDamage.hpp"
uintptr_t AllOrNothing::jmp_ret{NULL};
bool AllOrNothing::cheaton{NULL};
uint32_t AllOrNothing::stylebar{NULL};
bool noonetakesdamage;
bool onehitkill;
float zerovalue = 0.0;
float playermult = 10.0;
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
    newmem:
        push r8
        mov r8, [r14+0x98]
        test r8, r8
        je originalcode

	validation:
        cmp byte ptr [NoOneTakesDamage::cheaton], 1
        je nodamagecheck

    cheatcode:
        //if enemy, jump to allornothing
        cmp byte ptr [r8+0xF4], 1
        je allornothing
        //we pvp bois?
        cmp byte ptr [HeavyDay::cheaton], 1
        jne originalcode
        //if networked enemy don't do damage
        cmp byte ptr [r8+0xF4], 3
        je nodamage

        //if it's the master aka player character, multiply incoming damage
        //because players have stupid amounts of health.
        cmp byte ptr [r8+0xF4], 2
        je playerdamage

        //idk why you'd be here lol
        jmp originalcode
    allornothing:
    //Raw IK?
    cmp byte ptr [OneHitKill::cheaton], 1
    je alldamage
    //if we're not in combat, jump to the original code
	cmp byte ptr [AllOrNothing::cheaton], 0
    je originalcode
    cmp byte ptr [PlayerTracker::incombat], 0
    je originalcode
    //don't need the enemy entity anymore, so move the style bar in
    //style bar is the style rank we must equal or be above to do damage
    mov r8, [AllOrNothing::stylebar]
    cmp byte ptr [StyleRank::rank],r8b
    //if below, do no damage to the enemy.
    jb nodamage
    jmp originalcode

    nodamagecheck: // only check for cheaton if sent to nodamage via damage cheats. Skip this if PVP.
        cmp byte ptr [NoOneTakesDamage::cheaton], 1
        je nodamage
        jmp originalcode

    nodamage:
        //makes the damage done 0
        mulss xmm6, [zerovalue]
        jmp originalcode
    playerdamage:
        //player multiplier for PVP damage.
        mulss xmm6, [playermult]
        jmp originalcode
    alldamage:
        //this makes the damage done = the enemies current health.
        cmp byte ptr [OneHitKill::cheaton], 0 // Alldamage isn't used in PVP, so can safely limit this to AllOrNothing cheat
        je originalcode
        movss xmm6, [rdi+0x10]
        jmp originalcode
    originalcode:
        pop r8
        movss xmm1,[rdi+0x10]
        jmp qword ptr [AllOrNothing::jmp_ret]
	}
}

// clang-format on

void AllOrNothing::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> AllOrNothing::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_on_page    = gamemode;
  m_is_enabled = &AllOrNothing::cheaton;

  m_full_name_string     = "Must Style (+)";
  m_author_string        = "The Hitchhiker";
  m_description_string   = "Disable damage altogether or when below a certain Style Rank.";

  set_up_hotkey();

  auto addr = patterns->find_addr(base, "F3 0F 10 4F 10 0F 57 C0 0F 5A");
  if (!addr) {
    return "Unable to find AllOrNothing pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize AllOrNothing";
  }
  return Mod::on_initialize();
}

void AllOrNothing::on_config_load(const utility::Config& cfg) {
  //noonetakesdamage = cfg.get<bool>("no_one_takes_damage").value_or(false);
  //onehitkill       = cfg.get<bool>("one_hit_kill").value_or(false);
  AllOrNothing::stylebar = cfg.get<int>("style_damage_requirement").value_or(5);
}
void AllOrNothing::on_config_save(utility::Config& cfg) {
  //cfg.set<bool>("no_one_takes_damage", noonetakesdamage);
  //cfg.set<bool>("one_hit_kill", onehitkill);
  cfg.set<int>("style_damage_requirement", AllOrNothing::stylebar);
}

void AllOrNothing::on_draw_ui() {
  //ImGui::Checkbox("No one takes damage", &noonetakesdamage);
  //ImGui::Checkbox("One hit kill", &onehitkill);
  ImGui::Text("Style Rank to beat (1:D, 7:SSS)");
  UI::SliderInt("##StlyeRankRequirement", (int*)&AllOrNothing::stylebar, 1, 7);
  
}

#if 0 // version before siyan attacked; cheats work even without AllOrNothing cheaton bool
#include "AllOrNothing.hpp"
#include "mods/PlayerTracker.hpp"
#include "mods/HeavyDay.hpp"
#include "mods/StyleRank.hpp"
uintptr_t AllOrNothing::jmp_ret{NULL};
bool AllOrNothing::cheaton{NULL};
uint32_t AllOrNothing::stylebar{NULL};
bool noonetakesdamage;
bool onehitkill;
float zerovalue = 0.0;
float playermult = 10.0;
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
    newmem:
        push r8
        mov r8, [r14+0x98]
        test r8, r8
        je originalcode

	validation:
        cmp byte ptr [noonetakesdamage], 1
        je nodamage

    cheatcode:
        //if enemy, jump to allornothing
        cmp byte ptr [r8+0xF4], 1
        je allornothing
        //we pvp bois?
        cmp byte ptr [HeavyDay::cheaton], 1
        jne originalcode
        //if networked enemy don't do damage
        cmp byte ptr [r8+0xF4], 3
        je nodamage

        //if it's the master aka player character, multiply incoming damage
        //because players have stupid amounts of health.
        cmp byte ptr [r8+0xF4], 2
        je playerdamage

        //idk why you'd be here lol
        jmp originalcode
    allornothing:
    //Raw IK?
    cmp byte ptr [onehitkill], 1
    je alldamage
    //if we're not in combat, jump to the original code
	cmp byte ptr [AllOrNothing::cheaton], 0
    je originalcode
    cmp byte ptr [PlayerTracker::incombat], 0
    je originalcode
    //don't need the enemy entity anymore, so move the style bar in
    //style bar is the style rank we must equal or be above to do damage
    mov r8, [AllOrNothing::stylebar]
    cmp byte ptr [StyleRank::rank],r8b
    //if below, do no damage to the enemy.
    jb nodamage
    jmp originalcode
    nodamage:
        //makes the damage done 0
        mulss xmm6, [zerovalue]
        jmp originalcode
    playerdamage:
        //player multiplier for PVP damage.
        mulss xmm6, [playermult]
        jmp originalcode
    alldamage:
        //this makes the damage done = the enemies current health.
        movss xmm6, [rdi+0x10]
        jmp originalcode
    originalcode:
        pop r8
        movss xmm1,[rdi+0x10]
        jmp qword ptr [AllOrNothing::jmp_ret]
	}
}

std::optional<std::string> AllOrNothing::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  onpage    = gamemode;
  ischecked = &AllOrNothing::cheaton;

  full_name_string     = "Must Style / Damage Toggles (+)";
  author_string        = "The Hitchhiker";
  description_string   = "Disable damage altogether or when below a certain Style Rank.";

  auto addr = patterns->find_addr(base, "F3 0F 10 4F 10 0F 57 C0 0F 5A");
  if (!addr) {
    return "Unable to find AllOrNothing pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize AllOrNothing";
  }
  return Mod::on_initialize();
}

void AllOrNothing::on_config_load(const utility::Config& cfg) {
  noonetakesdamage = cfg.get<bool>("no_one_takes_damage").value_or(false);
  onehitkill       = cfg.get<bool>("one_hit_kill").value_or(false);
  AllOrNothing::stylebar = cfg.get<int>("style_damage_requirement").value_or(5);
}
void AllOrNothing::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("no_one_takes_damage", noonetakesdamage);
  cfg.set<bool>("one_hit_kill", onehitkill);
  cfg.set<int>("style_damage_requirement", AllOrNothing::stylebar);
}

void AllOrNothing::on_draw_ui() {
  ImGui::Checkbox("No one takes damage", &noonetakesdamage);
  ImGui::Checkbox("One hit kill", &onehitkill);
  ImGui::Spacing();
  ImGui::Text("Style Rank to beat (1:D, 7:SSS)");
  UI::SliderInt("##StlyeRankRequirement", (int*)&AllOrNothing::stylebar, 1, 7);
}
#endif