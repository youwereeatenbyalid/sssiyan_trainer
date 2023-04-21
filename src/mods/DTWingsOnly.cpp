#include "DTWingsOnly.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t DTWingsOnly::jmp_ret{NULL};
uintptr_t DTWingsOnly::jmp_ret2{NULL};
bool DTWingsOnly::cheaton{NULL};
static bool wingsOnlyDT = false;
static bool alwaysWings = false;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	    validation:
            cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
            jne code
            cmp byte ptr [DTWingsOnly::cheaton], 1
            jne code
            cmp byte ptr [wingsOnlyDT], 1
            je cheatcode
            jmp code

        code:
            cmp qword ptr [rax+0x18], 00
            jmp qword ptr [DTWingsOnly::jmp_ret]        
        cheatcode:
            cmp byte ptr [DTWingsOnly::cheaton], 0 //force sete AL to fail
            jmp qword ptr [DTWingsOnly::jmp_ret]
	}
}

static naked void detour2() {
	__asm {
	    validation:
            cmp [PlayerTracker::playerid], 0
            jne code
            cmp byte ptr [DTWingsOnly::cheaton], 1
            jne code
            cmp byte ptr [alwaysWings], 1
            je cheatcode
            jmp code

        code:
            cmp [rdx+0x000009B0], r12d
            jmp qword ptr [DTWingsOnly::jmp_ret2]
        cheatcode:
            cmp byte ptr [DTWingsOnly::cheaton], 1 //force je to succeed
            jmp qword ptr [DTWingsOnly::jmp_ret2]
	}
}

// clang-format on

void DTWingsOnly::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DTWingsOnly::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &DTWingsOnly::cheaton;
  m_on_page               = Page_Nero;
  m_full_name_string     = "Nero Wings (+)";
  m_author_string        = "Siyan";
  m_description_string   = "Set when wings appear.";

  set_up_hotkey();

  auto addr = m_patterns_cache->find_addr(base, "48 83 78 18 00 0F 94 C0 48 8B 6C 24 50");
  if (!addr) {
    return "Unable to find DTWingsOnly pattern.";
  }
  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DTWingsOnly";
  }

  auto addr2 = m_patterns_cache->find_addr(base, "44 39 A2 B0 09 00 00");
  if (!addr2) {
    return "Unable to find DTWingsOnly pattern2.";
  }
  if (!install_new_detour(addr2.value(), m_detour2, &detour2, &jmp_ret2, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DTWingsOnly2";
  }

  return Mod::on_initialize();
}

// during load
void DTWingsOnly::on_config_load(const utility::Config &cfg) {
    wingsOnlyDT = cfg.get<bool>("nero_wings_only_DT").value_or(false);
    alwaysWings = cfg.get<bool>("nero_always_wings").value_or(false);
}
// during save
void DTWingsOnly::on_config_save(utility::Config &cfg) {
    cfg.set<bool>("nero_wings_only_DT", wingsOnlyDT);
    cfg.set<bool>("nero_always_wings", alwaysWings);
}
// do something every frame
// void DTWingsOnly::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void DTWingsOnly::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void DTWingsOnly::on_draw_ui() {
    ImGui::Checkbox("DT form only adds wings", &wingsOnlyDT);
    ImGui::Checkbox("Force Wings", &alwaysWings);
}
