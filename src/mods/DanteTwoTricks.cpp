
#include "DanteTwoTricks.hpp"
#include "PlayerTracker.hpp"
uintptr_t DanteTwoTricks::jmp_ret{NULL};
uintptr_t DanteTwoTricks::jmp_jne{ NULL };
bool DanteTwoTricks::cheaton{NULL};


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code
		cmp byte ptr [DanteTwoTricks::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [PlayerTracker::isgrounded], 1
        je retcode
    code:
        test rax, rax
        jne jnecode
    retcode:
        jmp qword ptr [DanteTwoTricks::jmp_ret]

    jnecode:
        jmp qword ptr [DanteTwoTricks::jmp_jne]
	}
}

// clang-format on

void DanteTwoTricks::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DanteTwoTricks::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &DanteTwoTricks::cheaton;
  m_on_page             = Page_DanteCheat;

  m_full_name_string   = "DMC4 Trick Count";
  m_author_string      = "SSSiyan";
  m_description_string = "Using Trick while grounded won't use up an aerial trick.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = m_patterns_cache->find_addr(base, "60 17 00 00 48 85 C0 75 1E");
  if (!addr) {
    return "Unable to find DanteTwoTricks pattern.";
  }

  if (!install_new_detour(addr.value()+4, m_detour, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteTwoTricks";
  }
  DanteTwoTricks::jmp_jne = addr.value() + 0x4 + 0x23;
  return Mod::on_initialize();
}

// void DanteTwoTricks::on_draw_ui() {}
