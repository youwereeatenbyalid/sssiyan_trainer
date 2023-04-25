#include "VergilNoTrickRestriction.hpp"
#include "PlayerTracker.hpp"
uintptr_t VergilNoTrickRestriction::jmp_ret{NULL};
bool VergilNoTrickRestriction::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [VergilNoTrickRestriction::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        mov byte ptr [rcx+000000A8h], 00
        cmp byte ptr [rcx+000000A8h], 00
		jmp qword ptr [VergilNoTrickRestriction::jmp_ret]

    code:
        cmp byte ptr [rcx+000000A8h], 00
        jmp qword ptr [VergilNoTrickRestriction::jmp_ret]
	}
}

// clang-format on

void VergilNoTrickRestriction::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> VergilNoTrickRestriction::on_initialize() {
  init_check_box_info();

  m_is_enabled         = &VergilNoTrickRestriction::cheaton;
  m_on_page            = Page_VergilTrick;
  m_depends_on         = { "PlayerTracker" };
  m_full_name_string   = "No Trick Restriction";
  m_author_string      = "SSSiyan";
  m_description_string = "Allows you to trick during things like World of V startup.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr      = m_patterns_cache->find_addr(base, "80 B9 A8 00 00 00 00 0F");
  if (!addr) {
    return "Unable to find VergilNoTrickRestriction pattern.";
  }

  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilNoTrickRestriction";
  }
  return Mod::on_initialize();
}

//void VergilNoTrickRestriction::on_draw_ui() {}
