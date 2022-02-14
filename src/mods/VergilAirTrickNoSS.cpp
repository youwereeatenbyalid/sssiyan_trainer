
#include "VergilAirTrickNoSS.hpp"
#include "PlayerTracker.hpp"
uintptr_t VergilAirTrickNoSS::jmp_ret{NULL};
uintptr_t VergilAirTrickNoSS::jmp_je{NULL};
bool VergilAirTrickNoSS::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [VergilAirTrickNoSS::cheaton], 1
        je cheatcode
        jmp code

     cheatcode:
		jmp qword ptr [VergilAirTrickNoSS::jmp_ret]

    code:
        test ecx, ecx
        je jecode
        jmp qword ptr [VergilAirTrickNoSS::jmp_ret]

    jecode:
        jmp qword ptr [VergilAirTrickNoSS::jmp_je]
	}
}

// clang-format on

void VergilAirTrickNoSS::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> VergilAirTrickNoSS::on_initialize() {
  init_check_box_info();

  m_is_enabled = &VergilAirTrickNoSS::cheaton;
  m_on_page = vergiltrick;

  m_full_name_string = "Disable Embedded Swords";
  m_author_string        = "VPZadov";
  m_description_string   = "Allows you to trick to enemies without using an embedded sword.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "85 C9 0F 84 BE 01 00 00 F3 0F 10 87");
  if (!addr) {
    return "Unable to find VergilAirTrickNoSS pattern.";
  }

  VergilAirTrickNoSS::jmp_je = addr.value() + 0x1C6; //patterns->find_addr(base, "F3 0F 10 15 93 D6 09 04").value();

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilAirTrickNoSS";
  }
  return Mod::on_initialize();
}

// void VergilAirTrickNoSS::on_draw_ui() {}
