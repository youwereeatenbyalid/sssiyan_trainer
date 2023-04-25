#include "mods/PlayerTracker.hpp"
#include "mods/VergilDoppelBanish.hpp"

uintptr_t VergilDoppelBanish::jmp_ret{NULL};
uintptr_t VergilDoppelBanish::jmp_je{NULL};
uintptr_t VergilDoppelBanish::jmp_ret_yamato_banish{NULL};
uintptr_t VergilDoppelBanish::jmp_ret_beowulf_banish{NULL};
uintptr_t VergilDoppelBanish::jmp_ret_forceedge_banish{NULL};
uintptr_t VergilDoppelBanish::jmp_ret_sdt_yamato_banish{NULL};
uintptr_t VergilDoppelBanish::jmp_ret_sdt_beowulf_banish{NULL};
bool VergilDoppelBanish::cheaton{NULL};
uintptr_t VergilDoppelBanish::super_banish_call{NULL};

bool SDTDeactivationToggle{false};
bool SuperMovesToggle{false};

// clang-format off
// only in clang/icl mode on x64, sorry
static naked void detour() {
	__asm {
	    validation:
            cmp [PlayerTracker::playerid], 4
            jne code
            cmp byte ptr [VergilDoppelBanish::cheaton], 1 // Is the main cheat on?
            je cheatcode                                  // If yes, jump to cheatcode
            jmp code                                      // If not, ignore inner checkbox and jump to code

        cheatcode:
            cmp byte ptr [SDTDeactivationToggle], 1       // Is the inner cheat on?
            je doppeloff                                  // If yes, jump to doppel off
            jmp code                                      // If not, jump to code

        code:
            cmp byte ptr [rdi+0x000018A8],00
            je doppeloff
            jmp qword ptr [VergilDoppelBanish::jmp_ret]    

        doppeloff:
            jmp qword ptr [VergilDoppelBanish::jmp_je]
	}
}

static naked void DetourYamatoBanish() { //Yamato Super Banish
	__asm {
      validation:
        cmp [PlayerTracker::playerid], 4
        jne code
        cmp byte ptr [VergilDoppelBanish::cheaton], 1
        je cheatcode
        jmp code
      
      cheatcode:
        cmp byte ptr [SuperMovesToggle], 0  
        je code
        jmp qword ptr [VergilDoppelBanish::jmp_ret_yamato_banish]

      code:
        call qword ptr [VergilDoppelBanish::super_banish_call] 
        jmp qword ptr [VergilDoppelBanish::jmp_ret_yamato_banish]        
    }
} 

static naked void DetourBeowulfBanish() { //Beowulf Super Banish
	__asm {
      validation:
        cmp [PlayerTracker::playerid], 4
        jne code
        cmp byte ptr [VergilDoppelBanish::cheaton], 1
        je cheatcode
        jmp code
      
      cheatcode:
        cmp byte ptr [SuperMovesToggle], 0  
        je code
        jmp qword ptr [VergilDoppelBanish::jmp_ret_beowulf_banish]

      code:
        call qword ptr [VergilDoppelBanish::super_banish_call] 
        jmp qword ptr [VergilDoppelBanish::jmp_ret_beowulf_banish]        
    }
} 

static naked void DetourForceEdgeBanish() { //Force Edge Super Banish
	__asm {
      validation:
        cmp [PlayerTracker::playerid], 4
        jne code
        cmp byte ptr [VergilDoppelBanish::cheaton], 1
        je cheatcode
        jmp code
      
      cheatcode:
        cmp byte ptr [SuperMovesToggle], 0  
        je code 
        jmp qword ptr [VergilDoppelBanish::jmp_ret_forceedge_banish]

      code:
        call qword ptr [VergilDoppelBanish::super_banish_call] 
        jmp qword ptr [VergilDoppelBanish::jmp_ret_forceedge_banish]        
    }
} 

static naked void DetourSDTYamatoBanish() { //SDT Yamato Super Banish
	__asm {
      validation:
        cmp [PlayerTracker::playerid], 4
        jne code
        cmp byte ptr [VergilDoppelBanish::cheaton], 1
        je cheatcode
        jmp code
      
      cheatcode:
        cmp byte ptr [SuperMovesToggle], 0  
        je code  
        jmp qword ptr [VergilDoppelBanish::jmp_ret_sdt_yamato_banish]

      code:
        call qword ptr [VergilDoppelBanish::super_banish_call] 
        jmp qword ptr [VergilDoppelBanish::jmp_ret_sdt_yamato_banish]        
    }
} 

static naked void DetourSDTBeowulfBanish() { //SDT Beowulf Super Banish
	__asm {
      validation:
        cmp [PlayerTracker::playerid], 4
        jne code
        cmp byte ptr [VergilDoppelBanish::cheaton], 1
        je cheatcode
        jmp code
      
      cheatcode:
        cmp byte ptr [SuperMovesToggle], 0  
        je code 
        jmp qword ptr [VergilDoppelBanish::jmp_ret_sdt_beowulf_banish]

      code:
        call qword ptr [VergilDoppelBanish::super_banish_call] 
        jmp qword ptr [VergilDoppelBanish::jmp_ret_sdt_beowulf_banish]        
    }
} 
  // clang-format on

void VergilDoppelBanish::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> VergilDoppelBanish::on_initialize() {
  init_check_box_info();

  auto base          = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled          = &VergilDoppelBanish::cheaton;
  m_on_page             = Page_VergilDoppel;
  m_full_name_string   = "Disable Doppel Banish (+)";
  m_author_string      = "Dr.penguin";
  m_description_string = "Stops doppelganger from despawning while doing certain actions.";

  set_up_hotkey();

  auto addr = m_patterns_cache->find_addr(base, "80 BF A8 18 00 00 00 74 1D"); // No Dismiss on leaving SDT // THIS MIGHT BE WRONG PLEASE DOUBLECHECK
  if (!addr) {
    return "Unable to find VergilDoppelBanish pattern.";
  }

  auto addrYamatoBanish = m_patterns_cache->find_addr(base, "48 8B 97 10 03 00 00 45 33 C0 48 8B CB 48 85 D2 74 7E E8 ? ? ? FE"); // No Banish on Yamato Super // THIS MIGHT BE WRONG PLEASE DOUBLECHECK
  //DevilMayCry5.exe+1C12395 (-0x9)
  if (!addrYamatoBanish) {
    return "Unable to find YamatoBanish pattern.";
  }

  auto addrBeowulfBanish = m_patterns_cache->find_addr(base, "45 33 C0 BA 38 00 00 00 EB ? 0F 57 DB 41 B8 1A 00 00 00"); // No Banish on Beowulf Super // NEEDS AOB //DevilMayCry5.exe+11EC052 (+0x9D)
  if (!addrBeowulfBanish) {
    return "Unable to find BeowulfBanish pattern.";
  }

  auto addrForceEdgeBanish = m_patterns_cache->find_addr(base, "0F 57 DB 41 B8 25 00 00 00"); // No Banish on Force Edge Super // NEEDS AOB //DevilMayCry5.exe+C9A08A (+0x29)
  if (!addrForceEdgeBanish) {
    return "Unable to find ForceEdgeBanish pattern.";
  }

  auto addrSDTYamatoBanish = m_patterns_cache->find_addr(base, "45 33 C0  BA 38 00 00 00 EB ? 0F 57 DB 41 B8 10 00 00 00"); // No Banish on SDT Yamato Super // NEEDS AOB //DevilMayCry5.exe+1C123F1 (+0x1F)
  if (!addrSDTYamatoBanish) {
    return "Unable to find SDTYamatoBanish pattern.";
  }

  auto addrSDTBeowulfBanish = m_patterns_cache->find_addr(base, "0F 57 DB 41 B8 1A 00 00 00 E8"); // No Banish on SDT Beowulf Super // NEEDS AOB //DevilMayCry5.exe+11EC033 (+0x29)
  if (!addrSDTBeowulfBanish) {
    return "Unable to find SDTBeowulfBanish pattern.";
  }

  VergilDoppelBanish::super_banish_call = m_patterns_cache->find_addr(base, "40 53 57 41 57 48 83 EC 40 48 8B 41 50 45").value(); // The call every banish uses
  if (!VergilDoppelBanish::super_banish_call) {
    return "Unable to find VergilDoppelBanishCall pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 9)) {
    // return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilDoppelBanish";
  }
  VergilDoppelBanish::jmp_je = addr.value() + 38;

  if (!install_hook_absolute(addrYamatoBanish.value() + 0x9, m_function_hookYamatoBanish, &DetourYamatoBanish, &jmp_ret_yamato_banish, 5)) {
    // return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilDoppelBanish_yamatobanish";
  }

  if (!install_hook_absolute(addrBeowulfBanish.value() - 0x9D, m_function_hookBeowulfBanish, &DetourBeowulfBanish, &jmp_ret_beowulf_banish, 5)) {
    // return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilDoppelBanish_beowulfbanish";
  }

  if (!install_hook_absolute(addrForceEdgeBanish.value() - 0x29, m_function_hookForceEdgeBanish, &DetourForceEdgeBanish, &jmp_ret_forceedge_banish, 5)) {
    // return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilDoppelBanish_forceedgebanish";
  }

  if (!install_hook_absolute(addrSDTYamatoBanish.value() - 0x1F, m_function_hookSDTYamatoBanish, &DetourSDTYamatoBanish, &jmp_ret_sdt_yamato_banish, 5)) {
    // return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilDoppelBanish_sdtyamatobanish";
  }

  if (!install_hook_absolute(addrSDTBeowulfBanish.value() - 0x29, m_function_hookSDTBeowulfBanish, &DetourSDTBeowulfBanish, &jmp_ret_sdt_beowulf_banish, 5)) {
    // return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilDoppelBanish_sdtbeowulfbanish";
  }

  return Mod::on_initialize();
}

void VergilDoppelBanish::on_config_load(const utility::Config& cfg) {
  SDTDeactivationToggle = cfg.get<bool>("SDT_Deactivation").value_or(true);
  SuperMovesToggle = cfg.get<bool>("Super_Moves_Deactivation").value_or(true);
}

void VergilDoppelBanish::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("SDT_Deactivation", SDTDeactivationToggle);
  cfg.set<bool>("Super_Moves_Deactivation", SuperMovesToggle);
}

void VergilDoppelBanish::on_draw_ui() {
  ImGui::Checkbox("Disable doppel banish when leaving SDT", &SDTDeactivationToggle);
  ImGui::Checkbox("Disable doppel banish when doing supers", &SuperMovesToggle);
}