#include "WalkOnKeyboard.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t WalkOnKeyboard::jmp_ret{NULL};
bool WalkOnKeyboard::cheaton{NULL};
float keyboardWalkDiv{2.0f};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	    validation:
            cmp byte ptr [WalkOnKeyboard::cheaton], 1
            jne code

        // cheatcode
            push r10
            mov r10, [PlayerTracker::playerentity]
            test r10, r10
            je popcode
            mov r10, [r10+0xED0]
            cmp r10d, 1
            je popcode
            jmp divstickpush

        divstickpush:
            divss xmm0, [keyboardWalkDiv]
            jmp popcode

        popcode:
            pop r10
        code:
            movss [r14+30h], xmm0
            jmp qword ptr [WalkOnKeyboard::jmp_ret]        
	}
}

// clang-format on

void WalkOnKeyboard::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> WalkOnKeyboard::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &WalkOnKeyboard::cheaton;
  m_on_page               = qol;
  m_full_name_string     = "Walk Using Keyboard Controls";
  m_author_string        = "SSSiyan";
  m_description_string   = "Your character will walk rather than run.";

  set_up_hotkey();

  auto addr = patterns->find_addr(base, "F3 41 0F 11 46 30 48 8B 46");
  if (!addr) {
    return "Unable to find WalkOnKeyboard pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize WalkOnKeyboard";
  }
  return Mod::on_initialize();
}

// during load
// void WalkOnKeyboard::on_config_load(const utility::Config &cfg) {}
// during save
// void WalkOnKeyboard::on_config_save(utility::Config &cfg) {}
// do something every frame
//void WalkOnKeyboard::on_frame() {}
//will show up in debug window, dump ImGui widgets you want here
//void WalkOnKeyboard::on_draw_debug_ui() {}
//will show up in main window, dump ImGui widgets you want here
// void WalkOnKeyboard::on_draw_ui() {}
