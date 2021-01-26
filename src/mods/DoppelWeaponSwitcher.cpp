
#include "DoppelWeaponSwitcher.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t DoppelWeaponSwitcher::updateweapon_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::resetweapon_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::fixparameter_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::fixjdc_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::yamatotype{NULL};
uintptr_t DoppelWeaponSwitcher::beowulftype{NULL};
uintptr_t DoppelWeaponSwitcher::forceedgetype{NULL};
uintptr_t DoppelWeaponSwitcher::weaponresetparameter{NULL};
uint32_t DoppelWeaponSwitcher::doppelweaponid{0};
uintptr_t DoppelWeaponSwitcher::doppelweaponparameter{NULL};
bool      DoppelWeaponSwitcher::cheaton{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void updateweapon_detour() {
	__asm {
	    validation:
            cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
            jne code
            cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
            je cheatcode
            jmp code
        code:
            mov rcx, [rdi]
            mov r9d, r15d
            jmp qword ptr [DoppelWeaponSwitcher::updateweapon_jmp_ret]
        cheatcode:
            push r8
            push r10
            push r11
            push r12
            push r13
            push r14
            //exit if not doppelganger
            cmp byte ptr [rdi+0x17F0], 0
            je originalcode
            //normal vergil
            mov r11, [PlayerTracker::vergilentity]

            //don't swap on just judgement cuts
            //cmp byte ptr [justjdcflag], 1
            je originalcode
            //weaponid
            mov r12d, [r11+0x1978]
            //doppeljdccharge
            mov r13, [r11+0x1908]
            //weaponmodel
            mov r14, [r11+0x1980]

            mov r14, [r14+0x10]
            mov r14, [r14+0x300]

            //load doppel weapon
            mov dword ptr [DoppelWeaponSwitcher::doppelweaponid], r12d

            //backup common parameter
            mov [DoppelWeaponSwitcher::doppelweaponparameter], r14

            //enable JC's and reset the JC status so the game doesn't get confused
            mov byte ptr [r13+0x10], 1
            mov byte ptr [r13+0x30], 0

            //if we don't have yamato equiped, turn off doppelganger JC charging.
            cmp [DoppelWeaponSwitcher::doppelweaponid], 0
            je updateweapon
            mov byte ptr [r13+0x10], 0

        updateweapon:
        //doppelganger model
            mov r8, [rdi+0x1980]
            mov r8, [r8+0x10]

            cmp byte ptr [DoppelWeaponSwitcher::doppelweaponid], 2
            je beowulfassign

            cmp byte ptr [DoppelWeaponSwitcher::doppelweaponid], 1
            je forceedgeassign

        yamatoassign:
            mov r10, [DoppelWeaponSwitcher::yamatotype]
            mov r10, [r10]
            jmp assignmain

        beowulfassign:
            mov r10, [DoppelWeaponSwitcher::beowulftype]
            mov r10, [r10]
            jmp assignmain

        forceedgeassign:
            mov r10, [DoppelWeaponSwitcher::forceedgetype]
            mov r10, [r10]

        assignmain:
            mov [r8], r10
            mov r11, [DoppelWeaponSwitcher::doppelweaponparameter]
            test r11, r11
            je originalcode
            mov[r8+0x300], r11
        originalcode:
            mov rcx, [rdi]
            mov r9d, r15d
            pop r14
            pop r13
            pop r12
            pop r11
            pop r10
            pop r8
            jmp qword ptr [DoppelWeaponSwitcher::updateweapon_jmp_ret]
	}
}
static naked void resetweapon_detour() {
    __asm {
    memory:
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne originalcode
        mov rdx, [PlayerTracker::doppelweaponmodel]
        cmp rbx, [rdx+0x10]
        jne originalcode
        mov rax, [DoppelWeaponSwitcher::yamatotype]
        mov rax, [rax]
        mov [rbx], rax
    originalcode:
        mov rax, [rbx]
        mov rdx, [DoppelWeaponSwitcher::weaponresetparameter]
        mov rdx, [rdx]
        jmp qword ptr[DoppelWeaponSwitcher::resetweapon_jmp_ret]
    }
}
static naked void fixparameter_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        mov rcx, [PlayerTracker::yamatocommonparameter]
        test rcx, rcx
        je code
        mov rax, [PlayerTracker::yamatocommonparameter]
        mov r9, [PlayerTracker::yamatocommonparameter]
    code:
        mov rcx, [rbx+0x50]
        mov r14, rax
        jmp qword ptr [DoppelWeaponSwitcher::fixparameter_jmp_ret]
    }
}
static naked void fixjdc_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        mov rax, [rdx]
        lea r9, [rbp-0x60]
        jmp qword ptr [DoppelWeaponSwitcher::fixjdc_jmp_ret]
    cheatcode:
        //check if just judgement cut
        //cmp byte ptr [justjdcflag], 1
        //jne originalcode

        //check if doppelganger
        cmp byte ptr [r13+0x17F0], 1
        jne originalcode

        //move yamato
        mov rax, [DoppelWeaponSwitcher::yamatotype]
        mov rax, [rax]
        //mov byte ptr [justjdcflag], 0
        jmp originalcode2
    originalcode:
        mov rax, [rdx]
    originalcode2:
        lea r9, [rbp-0x60]
        jmp qword ptr [DoppelWeaponSwitcher::fixjdc_jmp_ret]
    }
}

// clang-format on

std::optional<std::string> DoppelWeaponSwitcher::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  uintptr_t staticbase = g_framework->get_module().as<uintptr_t>();
  ischecked = &DoppelWeaponSwitcher::cheaton;
  onpage    = vergilpage;
  full_name_string     = "Doppelganger Weapon Switcher";
  author_string        = "The HitchHiker";
  description_string   = "This is the description of DoppelWeaponSwitcher.";
  
  

  DoppelWeaponSwitcher::yamatotype = staticbase+0x59DF358;

  DoppelWeaponSwitcher::beowulftype = staticbase+0x56A7F78;
 
  DoppelWeaponSwitcher::forceedgetype = staticbase+0x5B2C058;

  DoppelWeaponSwitcher::weaponresetparameter = staticbase+0x7DEEDF8;

  auto updateweapon_addr = utility::scan(base, "48 8B 0F 45 8B CF");
  if (!updateweapon_addr) {
    return "Unable to find updateweapon pattern.";
  }
  if (!install_hook_absolute(updateweapon_addr.value(), m_updateweapon_hook, &updateweapon_detour, &updateweapon_jmp_ret, 6)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize updateweapon";
  }

  auto resetweapon_addr = utility::scan(base, "48 8B 03 48 8B 15 69 CF A5 07");
  if (!resetweapon_addr) {
      return "Unable to find resetweapon pattern.";
  }
  if (!install_hook_absolute(resetweapon_addr.value(), m_resetweapon_hook, &resetweapon_detour, &resetweapon_jmp_ret, 10)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize resetweapon";
  }

  auto fixparameter_addr = utility::scan(base, "48 8B 4B 50 4C 8B F0 48 83 79 18 00 0F 85 5E 1B	");
  if (!fixparameter_addr) {
      return "Unable to find fixparameter pattern.";
  }
  if (!install_hook_absolute(fixparameter_addr.value(), m_fixparameter_hook, &fixparameter_detour, &fixparameter_jmp_ret, 7)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize fixparameter";
  }

  auto fixjdc_addr = utility::scan(base, "48 8B 02 4C 8D 4D A0 40");
  if (!fixjdc_addr) {
      return "Unable to find fixjdc pattern.";
  }
  if (!install_hook_absolute(fixjdc_addr.value(), m_fixjdc_hook, &fixjdc_detour, &fixjdc_jmp_ret, 7)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize fixjdc";
  }
  return Mod::on_initialize();
}

// during load
void DoppelWeaponSwitcher::on_config_load(const utility::Config &cfg) {}
// during save
void DoppelWeaponSwitcher::on_config_save(utility::Config &cfg) {}
// do something every frame
void DoppelWeaponSwitcher::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void DoppelWeaponSwitcher::on_draw_debug_ui() {
ImGui::Text("Weapon id: %X",DoppelWeaponSwitcher::doppelweaponid);
ImGui::Text("Weapon id: %X", DoppelWeaponSwitcher::doppelweaponparameter);
ImGui::Text("Weapon id: %X", DoppelWeaponSwitcher::weaponresetparameter);
ImGui::Text("Weapon id: %X", DoppelWeaponSwitcher::yamatotype);
ImGui::Text("Weapon id: %X", DoppelWeaponSwitcher::beowulftype);
ImGui::Text("Weapon id: %X", DoppelWeaponSwitcher::forceedgetype);
}
// will show up in main window, dump ImGui widgets you want here
void DoppelWeaponSwitcher::on_draw_ui() {}
