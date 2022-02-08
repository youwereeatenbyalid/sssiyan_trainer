
#include "NoScreenShake.hpp"

uintptr_t NoScreenShake::jmp_ret{NULL};
bool NoScreenShake::cheaton{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry

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

  ischecked            = &NoScreenShake::cheaton;
  onpage               = camera;

  full_name_string     = "No Screen Shake";
  author_string        = "DeepDarkKapustka";
  description_string   = "Disables screen shake completely.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr      = patterns->find_addr(base, "00 CC CC CC CC CC CC CC 48 89 5C 24 18 56 57");
  if (!addr) {
    return "Unable to find NoScreenShake pattern.";
  }

  if (!install_hook_absolute(addr.value()+8, m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NoScreenShake";
  }
  return Mod::on_initialize();
}

void NoScreenShake::on_draw_ui() {
}
