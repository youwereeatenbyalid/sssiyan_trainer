#include "InfDT.hpp"

uintptr_t InfDT::jmp_ret{NULL};
bool InfDT::cheaton{NULL};

float desireddt = 10000.0;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [InfDT::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        minss xmm0, [desireddt]
		jmp qword ptr [InfDT::jmp_ret]

    code:
        minss xmm0, [rdi+00001110h]
        jmp qword ptr [InfDT::jmp_ret]
	}
}

// clang-format on

void InfDT::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> InfDT::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &InfDT::cheaton;
  m_on_page             = Page_CommonCheat;
  // m_depends_on         = { "PlayerTracker" };
  m_full_name_string   = "Infinite DT";
  m_author_string      = "SSSiyan";
  m_description_string = "Freeze Player DT to max.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr      = m_patterns_cache->find_addr(base, "F3 0F 5D 87 10 11 00 00");
  if (!addr) {
    return "Unable to find InfDT pattern.";
  }

  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize InfDT";
  }
  return Mod::on_initialize();
}

// void InfDT::on_draw_ui() {}
