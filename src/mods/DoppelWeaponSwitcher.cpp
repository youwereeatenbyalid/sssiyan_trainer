
#include "DoppelWeaponSwitcher.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t DoppelWeaponSwitcher::updateweapon_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::resetweapon_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::fixparameter_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::fixjdc_jmp_ret{NULL};

uintptr_t DoppelWeaponSwitcher::jjdcaltcheck_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::doppeljjdc_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::doppeljjdc_jmp_jne{NULL};
uintptr_t DoppelWeaponSwitcher::doppelonlyjjdc_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::doppelonlyjjdc_jmp_call{NULL};
uintptr_t DoppelWeaponSwitcher::doppelonlyjjdcteleport_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::doppelonlyjjdcteleport_jmp_call{NULL};

uintptr_t DoppelWeaponSwitcher::doppelidle1_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::doppelidle2_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::doppelidle3_jmp_ret{NULL};

uintptr_t DoppelWeaponSwitcher::doppelbeowulfcharge_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::soundchargestart_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::soundchargeend_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::soundchargelevel1_jmp_ret{NULL};
uintptr_t DoppelWeaponSwitcher::soundchargelevel2_jmp_ret{NULL};

uintptr_t DoppelWeaponSwitcher::yamatotype{NULL};
uintptr_t DoppelWeaponSwitcher::beowulftype{NULL};
uintptr_t DoppelWeaponSwitcher::forceedgetype{NULL};
uintptr_t DoppelWeaponSwitcher::weaponresetparameter{NULL};

uint32_t DoppelWeaponSwitcher::doppelweaponid{0};
uintptr_t DoppelWeaponSwitcher::doppelweaponparameter{NULL};
bool justjdcflag = false;
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
            cmp byte ptr [justjdcflag], 1
            //this confirms doppel is either being summoned or recalled, and prevents it changing on trick
            je originalcode
            cmp r14d, 0xFFFFFFFF
            je swapcondition
            cmp r14d, 0x0
            je swapcondition
            jmp originalcode
        swapcondition:
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
        cmp byte ptr [PlayerTracker::playerid], 4 //change this to the char number obviously
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
        cmp byte ptr [justjdcflag], 1
        jne originalcode

        //check if doppelganger
        cmp byte ptr [r13+0x17F0], 1
        jne originalcode

        //move yamato
        mov rax, [DoppelWeaponSwitcher::yamatotype]
        mov rax, [rax]
        mov byte ptr [justjdcflag], 0
        jmp originalcode2
    originalcode:
        mov rax, [rdx]
    originalcode2:
        lea r9, [rbp-0x60]
        jmp qword ptr [DoppelWeaponSwitcher::fixjdc_jmp_ret]
    }
}

static naked void jjdcaltcheck_detour() {
    __asm {
    validation:
        mov [justjdcflag], al
        movzx ecx, al
        mov rax, [rbx+0x50]
        jmp qword ptr [DoppelWeaponSwitcher::jjdcaltcheck_jmp_ret]
    }
}
static naked void doppeljjdc_detour() {
    __asm {
    validation:
        cmp byte ptr [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        cmp byte ptr [rdi+0x00001C3D], 00
        jne jne_ret
        jmp jmp_ret
        
    cheatcode:
        cmp byte ptr [rdi+0x00001C3D], 00
        je forcejump
        mov [justjdcflag], 1
        jmp originalcode

    forcejump:
        cmp byte ptr [DoppelWeaponSwitcher::doppelweaponid], 0
            //jmp DevilMayCry5.exe+566D21
    originalcode:
        jne jne_ret
        mov rdx, [rdi+0x000018B0]
        jmp jmp_ret
    jne_ret:
        jmp qword ptr [DoppelWeaponSwitcher::doppeljjdc_jmp_jne]
    jmp_ret:
        jmp qword ptr [DoppelWeaponSwitcher::doppeljjdc_jmp_ret]
    }
}
static naked void doppelonlyjjdc_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        jmp jmp_call
    cheatcode:
        cmp r8, 0
        je jmp_ret
        jmp jmp_call
    jmp_call:
        call qword ptr [DoppelWeaponSwitcher::doppelonlyjjdc_jmp_call]
    jmp_ret:
        jmp qword ptr [DoppelWeaponSwitcher::doppelonlyjjdc_jmp_ret]
    }
}
static naked void doppelonlyjjdcteleport_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        mov rdx, rdi
        jmp jmp_call
    cheatcode:
        mov rdx, rdi
        cmp byte ptr [rdi+0x00001C3D], 00
        je jmp_ret
        jmp jmp_call
    jmp_call:
        call qword ptr[DoppelWeaponSwitcher::doppelonlyjjdcteleport_jmp_call]
    jmp_ret:
        jmp qword ptr[DoppelWeaponSwitcher::doppelonlyjjdcteleport_jmp_ret]
    }
}

static naked void doppelidle1_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        mov edi, [rdx+0x00001978]
        jmp qword ptr [DoppelWeaponSwitcher::doppelidle1_jmp_ret]
    cheatcode:
        cmp byte ptr [rdx+0x17F0], 0
        je code
        mov edi, [DoppelWeaponSwitcher::doppelweaponid]
        test edi, edi
        je code
        jmp qword ptr [DoppelWeaponSwitcher::doppelidle1_jmp_ret]
    }
}
static naked void doppelidle2_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        cmp edi, eax
        setne al
        jmp qword ptr[DoppelWeaponSwitcher::doppelidle2_jmp_ret]
    cheatcode:
        cmp eax, eax
        setne al
        jmp qword ptr [DoppelWeaponSwitcher::doppelidle2_jmp_ret]
    }
}
static naked void doppelidle3_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        mov eax, [rax+0x00001978]
        jmp qword ptr [DoppelWeaponSwitcher::doppelidle3_jmp_ret]
    cheatcode:
        cmp byte ptr [rax+0x17F0], 0
        je code
        //I think this won't be used??
        mov rcx, [DoppelWeaponSwitcher::doppelweaponid]
        test rcx, rcx
        je code
        mov eax, [DoppelWeaponSwitcher::doppelweaponid]
        jmp qword ptr [DoppelWeaponSwitcher::doppelidle3_jmp_ret]
    }
}

static naked void doppelbeowulfcharge_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        cmp dword ptr [rdx+0x00001978], 02
        jmp qword ptr [DoppelWeaponSwitcher::doppelbeowulfcharge_jmp_ret]
    cheatcode:
        cmp byte ptr [rdx+0x17F0], 0
        je code
        cmp dword ptr [DoppelWeaponSwitcher::doppelweaponid], 02
        jmp qword ptr [DoppelWeaponSwitcher::doppelbeowulfcharge_jmp_ret]
    }
}
static naked void soundchargestart_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        mov rcx, [rax+0x00001980]
            
        jmp qword ptr[DoppelWeaponSwitcher::soundchargestart_jmp_ret]
    cheatcode:
        mov rcx, [PlayerTracker::beowulfmodel]
        jmp qword ptr[DoppelWeaponSwitcher::soundchargestart_jmp_ret]
    }
}
static naked void soundchargeend_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        mov rdx, [rax+0x10]
        mov rcx, rbx
        jmp qword ptr [DoppelWeaponSwitcher::soundchargeend_jmp_ret]
    cheatcode:
        mov rax, [PlayerTracker::beowulfmodel]
        mov rdx, [rax+0x10]
        mov rcx, rbx
        jmp qword ptr[DoppelWeaponSwitcher::soundchargeend_jmp_ret]
    }
}
static naked void soundchargelevel1_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        mov rdx, [rcx+0x10]
        mov rcx, rbx
        jmp qword ptr [DoppelWeaponSwitcher::soundchargelevel1_jmp_ret]
    cheatcode:
        mov rax, [PlayerTracker::beowulfmodel]
        mov rdx, [rcx+0x10]
        mov rcx, rbx
        jmp qword ptr [DoppelWeaponSwitcher::soundchargelevel1_jmp_ret]
    }
}
static naked void soundchargelevel2_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [DoppelWeaponSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        mov rdx, [rcx+0x10]
        mov rcx, rbx
        jmp qword ptr [DoppelWeaponSwitcher::soundchargelevel2_jmp_ret]
    cheatcode:
        mov rax, [PlayerTracker::beowulfmodel]
        mov rdx, [rcx+0x10]
        mov rcx, rbx
        jmp qword ptr [DoppelWeaponSwitcher::soundchargelevel2_jmp_ret]
    }
}
// clang-format on

void DoppelWeaponSwitcher::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DoppelWeaponSwitcher::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  uintptr_t staticbase = g_framework->get_module().as<uintptr_t>();
  ischecked = &DoppelWeaponSwitcher::cheaton;
  onpage    = vergildoppel;
  full_name_string     = "Doppelganger Weapon Switcher";
  author_string        = "The HitchHiker";
  description_string   = "This is the description of DoppelWeaponSwitcher.";
  
  

  DoppelWeaponSwitcher::yamatotype = staticbase+0x59DF358;

  DoppelWeaponSwitcher::beowulftype = staticbase+0x56A7F78;
 
  DoppelWeaponSwitcher::forceedgetype = staticbase+0x5B2C058;

  DoppelWeaponSwitcher::weaponresetparameter = staticbase+0x7DEEDF8;
  /// <summary>
  /// Main function
  /// </summary>
  /// <returns></returns>
  auto updateweapon_addr = utility::scan(base, "48 8B 0F 45 8B CF");
  if (!updateweapon_addr) {
    return "Unable to find updateweapon pattern.";
  }
  if (!install_hook_absolute(updateweapon_addr.value(), m_updateweapon_hook, &updateweapon_detour, &updateweapon_jmp_ret, 6)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize updateweapon";
  }
  /// <summary>
  /// Crash Prevention
  /// </summary>
  /// <returns></returns>
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
  /// <summary>
  /// Ensure doppelganger uses correct prefab during JDC
  /// </summary>
  /// <returns></returns>
  auto fixjdc_addr = utility::scan(base, "48 8B 02 4C 8D 4D A0 40");
  if (!fixjdc_addr) {
      return "Unable to find fixjdc pattern.";
  }
  if (!install_hook_absolute(fixjdc_addr.value(), m_fixjdc_hook, &fixjdc_detour, &fixjdc_jmp_ret, 7)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize fixjdc";
  }
  /// <summary>
  /// Ensures the doppelganger only just judgement cuts when using a non-yamato weapon
  /// </summary>
  /// <returns></returns>
  auto jjdcaltcheck_addr = utility::scan(base, "78 B9 01 0F B6 C8 48 8B 43 50");
  if (!jjdcaltcheck_addr) {
      return "Unable to find jjdcaltcheck pattern.";
  }
  if (!install_hook_absolute(jjdcaltcheck_addr.value()+3, m_jjdcaltcheck_hook, &jjdcaltcheck_detour, &jjdcaltcheck_jmp_ret, 7)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize jjdcaltcheck";
  }

  auto doppeljjdc_addr = utility::scan(base, "75 38 48 8B 97 B0 18 00 00");
  if (!doppeljjdc_addr) {
      return "Unable to find doppeljjdc pattern.";
  }
  if (!install_hook_absolute(doppeljjdc_addr.value(), m_doppeljjdc_hook, &doppeljjdc_detour, &doppeljjdc_jmp_ret, 9)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize doppeljjdc";
  }

  auto doppelonlyjjdc_addr = utility::scan(base, "E8 90 74 FE FF");
  if (!doppelonlyjjdc_addr) {
      return "Unable to find doppelonlyjjdc pattern.";
  }
  if (!install_hook_absolute(doppelonlyjjdc_addr.value(), m_doppelonlyjjdc_hook, &doppelonlyjjdc_detour, &doppelonlyjjdc_jmp_ret, 5)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize doppelonlyjjdc";
  }
  
  auto doppelonlyjjdcteleport_addr = utility::scan(base, "48 8B D7 E8 07 71 FE FF");
  if (!doppelonlyjjdcteleport_addr) {
      return "Unable to find doppelonlyjjdcteleport pattern.";
  }
  if (!install_hook_absolute(doppelonlyjjdcteleport_addr.value(), m_doppelonlyjjdcteleport_hook, &doppelonlyjjdcteleport_detour, &doppelonlyjjdcteleport_jmp_ret, 8)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize doppelonlyjjdcteleport";
  }

  DoppelWeaponSwitcher::doppeljjdc_jmp_jne = doppelonlyjjdcteleport_addr.value();
  DoppelWeaponSwitcher::doppelonlyjjdc_jmp_call = doppelonlyjjdc_addr.value()-0x18B6B;
  DoppelWeaponSwitcher::doppelonlyjjdcteleport_jmp_call = doppelonlyjjdcteleport_addr.value()-0x18EF1;
  /// <summary>
  /// Make the doppelganger use proper idle animations
  /// </summary>
  /// <returns></returns>
  auto doppelidle1_addr = utility::scan(base, "8B BA 78 19 00 00 48");
  if (!doppelidle1_addr) {
      return "Unable to find doppelidle1 pattern.";
  }
  if (!install_hook_absolute(doppelidle1_addr.value(), m_doppelidle1_hook, &doppelidle1_detour, &doppelidle1_jmp_ret, 6)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize doppelidle1";
  }

  if (!install_hook_absolute(doppelidle1_addr.value()+0x1F, m_doppelidle2_hook, &doppelidle2_detour, &doppelidle2_jmp_ret, 5)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize doppelidle2";
  }

  auto doppelidle3_addr = utility::scan(base, "8B 80 78 19 00 00");
  if (!doppelidle3_addr) {
      return "Unable to find doppelidle3 pattern.";
  }
  if (!install_hook_absolute(doppelidle3_addr.value(), m_doppelidle3_hook, &doppelidle3_detour, &doppelidle3_jmp_ret, 6)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize doppelidle3";
  }
  /// <summary>
  /// Beowulf Charge for Doppel
  /// </summary>
  /// <returns></returns>
  auto doppelbeowulfcharge_addr = utility::scan(base, "83 BA 78 19 00 00 02 0F 94");
  if (!doppelbeowulfcharge_addr) {
      return "Unable to find doppelbeowulfcharge pattern.";
  }
  if (!install_hook_absolute(doppelbeowulfcharge_addr.value(), m_doppelbeowulfcharge_hook, &doppelbeowulfcharge_detour, &doppelbeowulfcharge_jmp_ret, 7)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize doppelbeowulfcharge";
  }

  auto soundchargestart_addr = utility::scan(base, "48 8B 88 80 19 00 00 48 85 C9 74 DB	");
  if (!soundchargestart_addr) {
      return "Unable to find soundchargestart pattern.";
  }
  if (!install_hook_absolute(soundchargestart_addr.value(), m_soundchargestart_hook, &soundchargestart_detour, &soundchargestart_jmp_ret, 7)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize soundchargestart";
  }

  auto soundchargeend_addr = utility::scan(base, "25 48 8B 50 10 48 8B CB");
  if (!soundchargeend_addr) {
      return "Unable to find soundchargeend pattern.";
  }
  if (!install_hook_absolute(soundchargeend_addr.value()+1, m_soundchargeend_hook, &soundchargeend_detour, &soundchargeend_jmp_ret, 7)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize soundchargeend";
  }

  auto soundchargelevel1_addr = utility::scan(base, "48 8B 51 10 48 8B CB 48 85 D2 0F 84 07");
  if (!soundchargelevel1_addr) {
      return "Unable to find soundchargelevel1 pattern.";
  }
  if (!install_hook_absolute(soundchargelevel1_addr.value(), m_soundchargelevel1_hook, &soundchargelevel1_detour, &soundchargelevel1_jmp_ret, 7)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize soundchargelevel1";
  }

  auto soundchargelevel2_addr = utility::scan(base, "48 8B 51 10 48 8B CB 48 85 D2 0F 84 56");
  if (!soundchargelevel2_addr) {
      return "Unable to find soundchargelevel2 pattern.";
  }
  if (!install_hook_absolute(soundchargelevel2_addr.value(), m_soundchargelevel2_hook, &soundchargelevel2_detour, &soundchargelevel2_jmp_ret, 7)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize soundchargelevel2";
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
