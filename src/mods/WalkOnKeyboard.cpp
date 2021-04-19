#include "WalkOnKeyboard.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t WalkOnKeyboard::jmp_ret{NULL};
bool WalkOnKeyboard::cheaton{NULL};
float keyboardWalkDiv{0.5f};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	    validation:
            cmp byte ptr [WalkOnKeyboard::cheaton], 1
            jne code
            cmp [PlayerTracker::playerid], 0
            je nerocode
            cmp [PlayerTracker::playerid], 1
            je dantecode
            cmp [PlayerTracker::playerid], 2
            je vcode
            cmp [PlayerTracker::playerid], 4
            je vergilcode
            jmp code

        nerocode:
            // cmp byte ptr [PlayerTracker::neroentity+0xED0], 1
            je code
            jmp divstickpush
        dantecode:
            // cmp byte ptr [PlayerTracker::danteentity+0xED0], 1
            je code
            jmp divstickpush
        vcode:
            // cmp byte ptr [PlayerTracker::ventity+0xED0], 1
            je code
            jmp divstickpush
        vergilcode:
            // cmp byte ptr [PlayerTracker::vergilentity+0xED0], 1
            je code
            jmp divstickpush

        divstickpush:
            divss xmm0, [keyboardWalkDiv]
            jmp code

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
  ischecked = &WalkOnKeyboard::cheaton;
  onpage               = mechanics;
  full_name_string     = "Walk On Keyboard";
  author_string        = "SSSiyan";
  description_string   = "Your character will walk rather than run.";

  auto addr = utility::scan(base, "F3 41 0F 11 46 30 48 8B 46");
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
void WalkOnKeyboard::on_config_load(const utility::Config &cfg) {}
// during save
void WalkOnKeyboard::on_config_save(utility::Config &cfg) {}
// do something every frame
//void WalkOnKeyboard::on_frame() {}
//will show up in debug window, dump ImGui widgets you want here
//void WalkOnKeyboard::on_draw_debug_ui() {}
//will show up in main window, dump ImGui widgets you want here
void WalkOnKeyboard::on_draw_ui() {
}
