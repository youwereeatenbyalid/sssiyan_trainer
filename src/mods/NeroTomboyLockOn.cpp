
#include "NeroTomboyLockOn.hpp"
#include "PlayerTracker.hpp"
uintptr_t NeroTomboyLockOn::jmp_ret{NULL};
uintptr_t NeroTomboyLockOn::jmp_jne{NULL};
bool NeroTomboyLockOn::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [NeroTomboyLockOn::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        jmp retcode

    code:
        test ecx, ecx
        jne jnejmp
    retcode:
        mov rdx, rbx
        jmp qword ptr [NeroTomboyLockOn::jmp_ret]

    jnejmp:
        jmp qword ptr [NeroTomboyLockOn::jmp_jne]
	}
}

// clang-format on

void NeroTomboyLockOn::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NeroTomboyLockOn::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &NeroTomboyLockOn::cheaton;
  m_on_page             = Page_Nero;

  m_full_name_string   = "Lock On With Tomboy";
  m_author_string      = "SSSiyan";
  m_description_string        = "Allows you to lock on while using Tomboy.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = m_patterns_cache->find_addr(base, "20 85 C9 75 15 48 8B D3");
  if (!addr) {
    return "Unable to find NeroTomboyLockOn pattern.";
  }

  if (!install_new_detour(addr.value()+1, m_detour, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroTomboyLockOn";
  }

  NeroTomboyLockOn::jmp_jne = addr.value() + 26;

  return Mod::on_initialize();
}

// void NeroTomboyLockOn::on_draw_ui() {}
