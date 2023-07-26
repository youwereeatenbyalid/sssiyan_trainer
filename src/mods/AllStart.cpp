#include "AllStart.hpp"
#include "PlayerTracker.hpp"
#include "GameInput.hpp"

uintptr_t AllStart::jmp_ret{NULL};
bool AllStart::cheaton{NULL};
uintptr_t AllStart::jmp_initial{NULL};
int cancelId = 0;
bool stingerjumpcancels;
bool guardCancelsEverything;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() { // "DevilMayCry5.exe"+964B06
	__asm {
        cmp byte ptr [AllStart::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        /*
        push r10                                    // The following was an attempt at getting MoveID faster
        push r11                                    // But didn't end up helping
        push r12                                    // At least it's an example on how PlayerTracker could work 

        mov r10, [PlayerTracker::playerentity]      // r10 will always be entity base
        test r10, r10
        je popcode
                                                    // r11 will always be base of class needed
        mov r11, [r10+0x2E0]                        // (Foot controller)

            mov r11, [r11+0xD8]                     // (idk ty for labels hitch)

                mov r12, [r11+0x2C]                 // Move bank
                mov [cancelId], r12w
                mov r12, [r11+0x34]                 // Move ID
                mov [cancelId+2], r12w

        pop r12
        pop r11
        pop r10
        */
        cmp [PlayerTracker::playerid], 0
        je nerocancels
        cmp [PlayerTracker::playerid], 1
        je dantecancels
        cmp [PlayerTracker::playerid], 2
        je vancels
        cmp [PlayerTracker::playerid], 4
        je vergilancels
        jmp code

    nerocancels:
        cmp dword ptr [PlayerTracker::playermoveid], 0x5280000 // Nero Stinger Jump
        je stingercheck
        jmp code

    dantecancels:
        cmp dword ptr [PlayerTracker::playermoveid], 0x5280000 // Dante Stinger Jump
        je stingercheck
        cmp byte ptr [guardCancelsEverything], 1 // should come last
        je guardallcheck
        jmp code

    vancels:
        jmp code

    vergilancels:
        cmp dword ptr [PlayerTracker::playermoveid], 0x5280000 // Dante Stinger Jump
        je stingercheck
        jmp code

//_____________________________________________________________________________________


    // jccheck:
        // cmp byte ptr [enemystepcancels], 1
        // je cancellable
        // jmp code

    // dantejccheck:
        // cmp byte ptr [enemystepcancels], 1
        // je dantejcweaponcheck
        // jmp code

    // dantejcweaponcheck:
        // cmp byte ptr [PlayerTracker::danteweapon], 5 // Cavaliere
        // je code
        // cmp byte ptr [PlayerTracker::danteweapon], 6 // Cavailiere R
        // je code
        // jmp cancellable

    guardallcheck:
        /* // these don't work because moveid updates too slow, using holdframes+4*? feels bad
        cmp dword ptr[PlayerTracker::playermoveid], 0x1B6201FE // release ground
        je code
        cmp dword ptr [PlayerTracker::playermoveid], 0x1B6C01FE // royal release ground
        je code
        cmp dword ptr [PlayerTracker::playermoveid], 0x1D5601FE // release air
        je code
        cmp dword ptr [PlayerTracker::playermoveid], 0x1D6001FE // royal release air
        je code
        cmp dword ptr [PlayerTracker::playermoveid], 0x138801FE // guard start ground
        je code
        cmp dword ptr [PlayerTracker::playermoveid], 0x139201FE // guard loop ground
        je code
        cmp dword ptr [PlayerTracker::playermoveid], 0x139C01FE // guard end ground
        je code
        cmp dword ptr [PlayerTracker::playermoveid], 0x157C01FE // guard start air
        je code
        cmp dword ptr [PlayerTracker::playermoveid], 0x158101FE // guard loop air
        je code
        cmp dword ptr [PlayerTracker::playermoveid], 0x158601FE // guard end air
        je code*/
        push r11
        mov r11, [PlayerTracker::playerentity]
        test r11,r11
        je codepops
        add r11, 0x1888
        cmp byte ptr [r11], 3 // royal guard
        jne codepops

        mov r11, [PlayerTracker::playerentity]
        add r11, 0xEF0
        mov r11, [r11]
        add r11, 0x48
        mov r11, [r11]
        shr r11, 0xC //this bit shifts 12 times, so 0x1000->0x1
        test r11, 0x1
        pop r11
        jne cancellable
        jmp code

    stingercheck:
        cmp byte ptr [stingerjumpcancels], 1
        je cancellable
        jmp code

    cancellable:
        mov word ptr [rdi+5Eh], 0100h
		jmp qword ptr [AllStart::jmp_ret]

    codepops:
        pop r11
    code:
        mov word ptr [rdi+5Eh], 0000h
        jmp qword ptr [AllStart::jmp_ret]
	}
}

// clang-format on

void AllStart::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> AllStart::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &AllStart::cheaton;
  m_on_page             = Page_Animation;
  m_depends_on         = { "PlayerTracker" };
  m_full_name_string   = "Selective Omnicancels (+)";
  m_author_string      = "SSSiyan, Dr.penguin";
  m_description_string = "Allows you to cancel out of a selection of moves with any action.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr      = m_patterns_cache->find_addr(base, "66 C7 47 5E 00 00");
  if (!addr) {
    return "Unable to find AllStart pattern.";
  }
  AllStart::jmp_initial = addr.value();

  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize AllStart";
  }
  return Mod::on_initialize();
}

void AllStart::on_config_load(const utility::Config& cfg) {
  stingerjumpcancels = cfg.get<bool>("stinger_jump_cancels").value_or(true);
  guardCancelsEverything = cfg.get<bool>("guard_cancels_everything").value_or(false);
}
void AllStart::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("stinger_jump_cancels", stingerjumpcancels);
  cfg.set<bool>("guard_cancels_everything", guardCancelsEverything);
}

void AllStart::on_draw_ui() {
  ImGui::Text("All Characters");
  ImGui::Checkbox("Stinger Jump", &stingerjumpcancels);
  ImGui::Separator();
  ImGui::Text("Dante");
  ImGui::Checkbox("Guard Cancels Everything", &guardCancelsEverything);
}
