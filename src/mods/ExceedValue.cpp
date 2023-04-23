#include "ExceedValue.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t ExceedValue::jmp_ret{NULL};
bool ExceedValue::cheaton{NULL};
uint32_t ExceedValue::exceed_value{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	    validation:
            cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
            jne code
            cmp byte ptr [ExceedValue::cheaton], 1
            je cheatcode
            jmp code
        code:
            mov rcx, [rax+0x00001948]
            jmp qword ptr [ExceedValue::jmp_ret]        
        cheatcode:
            mov rcx, [rax+0x00001948]
            push r8
            xor r8, r8
            mov r8, [ExceedValue::exceed_value]
            mov [rcx+0x18], r8
            pop r8
            jmp qword ptr [ExceedValue::jmp_ret]
	}
}

// clang-format on

void ExceedValue::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> ExceedValue::on_initialize() {
  init_check_box_info();

  auto base              = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled           = &ExceedValue::cheaton;
  m_on_page              = Page_Nero;
  m_depends_on           = { "PlayerTracker" };
  m_full_name_string     = "Set Exceed Level (+)";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Set/Lock Nero's Exceed.";

  set_up_hotkey();

  auto addr = m_patterns_cache->find_addr(base, "48 8B 88 48 19 00 00 33 F6 48 85");
  if (!addr) {
    return "Unable to find ExceedValue pattern.";
  }
  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 7)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize ExceedValue";
  }
  return Mod::on_initialize();
}

// during load
void ExceedValue::on_config_load(const utility::Config &cfg) {
    exceed_value = cfg.get<int>("exceed_value").value_or(1);
}
// during save
void ExceedValue::on_config_save(utility::Config &cfg) {
    cfg.set<int>("exceed_value", exceed_value);
}
// do something every frame
//void ExceedValue::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void ExceedValue::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void ExceedValue::on_draw_ui() {
    UI::SliderInt("Exceed Level",(int*)&ExceedValue::exceed_value, 0, 3);
}