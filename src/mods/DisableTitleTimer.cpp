
#include "DisableTitleTimer.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t DisableTitleTimer::jmp_ret{NULL};
bool DisableTitleTimer::cheaton{NULL};
float maxtime = 1800.0;
float timerdisable = 1.0;
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	    validation:
            cmp byte ptr [DisableTitleTimer::cheaton], 1
            je cheatcode
            jmp code
        code:
            movss xmm0, [maxtime]
            jmp qword ptr [DisableTitleTimer::jmp_ret]        
        cheatcode:
            movss xmm0, [timerdisable]
            movss [rbx+0xD8], xmm0
		    movss xmm0, [maxtime]
            jmp qword ptr [DisableTitleTimer::jmp_ret]
	}
}

// clang-format on

void DisableTitleTimer::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DisableTitleTimer::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &DisableTitleTimer::cheaton;
  m_on_page    = Page_QOL;
  m_full_name_string     = "Disable Titlescreen Timer";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Prevent the titlescreen from playing the mission 1 cutscene when left idling.";

  set_up_hotkey();

  auto addr = m_patterns_cache->find_addr(base, "F3 0F 10 05 CF FF 45 06");
  if (!addr) {
    return "Unable to find DisableTitleTimer pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 8)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DisableTitleTimer";
  }
  return Mod::on_initialize();
}

// during load
// void DisableTitleTimer::on_config_load(const utility::Config &cfg) {}
// during save
// void DisableTitleTimer::on_config_save(utility::Config &cfg) {}
// do something every frame
// void DisableTitleTimer::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void DisableTitleTimer::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void DisableTitleTimer::on_draw_ui() {}
