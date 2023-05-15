
#include "NoScreenShake.hpp"
uintptr_t NoScreenShake::jmp_ret{NULL};
bool NoScreenShake::cheaton{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry
//Could be replaced with API hook to skip function call?
static naked void detour() {
	__asm {
        cmp byte ptr [NoScreenShake::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        ret

    code:
        mov [rsp+18h], rbx
		jmp qword ptr [NoScreenShake::jmp_ret]
	}
}

// clang-format on
void NoScreenShake::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NoScreenShake::on_initialize() {
  init_check_box_info();

  m_is_enabled            = &NoScreenShake::cheaton;
  m_on_page               = Page_Camera;

  m_full_name_string     = "No Screen Shake";
  m_author_string        = "deepdarkkapustka";
  m_description_string   = "Disables screen shake completely.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  //.text:0000000140CBD3C0 app_PlayerCameraController__RequestCameraShake147315 proc near
  //tu6 AOB 00 CC CC CC CC CC CC CC 48 89 5C 24 18 56 57 +0x8
  //tu7 call to E8 ? ? ? ? 48 8B AC 24 ? ? ? ? 0F 28 74 24 ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B 18 49 8B 73 20 49 8B 7B 28 
  auto addr = m_patterns_cache->find_addr(base, "E8 ? ? ? ? 48 8B AC 24 ? ? ? ? 0F 28 74 24 ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B 18 49 8B 73 20 49 8B 7B 28 ");
  if (!addr) {
    return "Unable to find NoScreenShake pattern.";
  }
  uintptr_t actual_address = getRelCallDestination(addr.value());
  if (!install_new_detour(actual_address, m_detour, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NoScreenShake";
  }
  return Mod::on_initialize();
}

// void NoScreenShake::on_draw_ui() {}
