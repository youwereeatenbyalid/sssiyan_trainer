#include "DTWingsOnly.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t DTWingsOnly::jmp_ret{NULL};
bool DTWingsOnly::cheaton{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	    validation:
            cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
            jne code
            cmp byte ptr [DTWingsOnly::cheaton], 1
            je cheatcode
            jmp code
        code:
            cmp qword ptr [rax+0x18], 00
            jmp qword ptr [DTWingsOnly::jmp_ret]        
        cheatcode:
            jmp qword ptr [DTWingsOnly::jmp_ret]
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
  m_on_page               = commoncheat;
  m_full_name_string     = "Nero Wings only DT";
  m_author_string        = "The Hitchhiker";
  m_description_string   = "Nero DT only wings.";

  set_up_hotkey();

  auto addr = patterns->find_addr(base, "48 83 78 18 00 0F 94 C0 48 8B 6C 24 50");
  if (!addr) {
    return "Unable to find DTWingsOnly pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DTWingsOnly";
  }
  return Mod::on_initialize();
}

// during load
void DTWingsOnly::on_config_load(const utility::Config &cfg) {}
// during save
void DTWingsOnly::on_config_save(utility::Config &cfg) {}
// do something every frame
// void DTWingsOnly::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void DTWingsOnly::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void DTWingsOnly::on_draw_ui() {}
