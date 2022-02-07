#include "NothingCancelsBubble.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t NothingCancelsBubble::jmp_ret{NULL};
bool NothingCancelsBubble::cheaton{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	    validation:
            cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
            jne code
            cmp byte ptr [NothingCancelsBubble::cheaton], 1
            je cheatcode
            jmp code
        code:
            mov rax, [rcx+0x50]
            mov r15,r8
            jmp qword ptr [NothingCancelsBubble::jmp_ret]        
        cheatcode:
            jmp qword ptr [NothingCancelsBubble::jmp_ret]
	}
}

// clang-format on

void NothingCancelsBubble::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NothingCancelsBubble::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &NothingCancelsBubble::cheaton;
  m_on_page    = nero;
  m_full_name_string     = "Nothing cancels ragtime bubble";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Prevents ragtime bubble from being destroyed prematurely.";

  set_up_hotkey();

  auto addr = utility::scan(base, "48 8B 41 50 4D 8B F8 48 8B DA 4C");
  if (!addr) {
    return "Unable to find NothingCancelsBubble pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 7)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NothingCancelsBubble";
  }
  return Mod::on_initialize();
}

// during load
void NothingCancelsBubble::on_config_load(const utility::Config &cfg) {}
// during save
void NothingCancelsBubble::on_config_save(utility::Config &cfg) {}
// do something every frame
void NothingCancelsBubble::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void NothingCancelsBubble::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void NothingCancelsBubble::on_draw_ui() {}