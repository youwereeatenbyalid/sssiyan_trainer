#include "modNeroAlwaysInitialDT.hpp"
#include "PlayerTracker.hpp"
bool NeroAlwaysInitialDT::cheaton{NULL};
uintptr_t NeroAlwaysInitialDT::jmp_ret{NULL};
uintptr_t NeroAlwaysInitialDT::jmp_jne{NULL};
uintptr_t NeroAlwaysInitialDT::jmp_away{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
		cmp byte ptr [NeroAlwaysInitialDT::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		jmp qword ptr [NeroAlwaysInitialDT::jmp_away] // skips the next 2 opcodes too

    code:
        cmp byte ptr [rdi+00001AB0h], 00
        jne jnecode
        jmp qword ptr [NeroAlwaysInitialDT::jmp_ret]

    jnecode:
        jmp qword ptr [NeroAlwaysInitialDT::jmp_jne]
	}
}

// clang-format on

void NeroAlwaysInitialDT::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NeroAlwaysInitialDT::on_initialize() {
  init_check_box_info();

  m_is_enabled = &NeroAlwaysInitialDT::cheaton;
  m_on_page    = Page_Nero;

  m_full_name_string   = "Forced Initial DT Animation";
  m_author_string      = "SSSiyan";
  m_description_string = "Note: the voice line will only play the first time you use this each run of M20.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = m_patterns_cache->find_addr(base, "80 BF B0 1A 00 00 00"); // DevilMayCry5.exe+2104DE2 Vergil exe
  if (!addr) {
    return "Unable to find NeroAlwaysInitialDT pattern.";
  }

  NeroAlwaysInitialDT::jmp_jne = addr.value() + 321;
  NeroAlwaysInitialDT::jmp_away = addr.value() + 26;

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 13)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroAlwaysInitialDT";
  }

  // m_patch01 = Patch::create_nop(addr.value(), 6, false);
  // m_patch02 = Patch::create_nop(addr.value()+13, 6, false);
  // or m_patch02 = Patch::create(addr.value()+13, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90}, false);
  // m_patch02->enable();
  return Mod::on_initialize();
}
/*
void NeroAlwaysInitialDT::on_config_load(const utility::Config& cfg) { // this works
  m_patch01->toggle(&NeroAlwaysInitialDT::cheaton);
  m_patch02->toggle(&NeroAlwaysInitialDT::cheaton);
}
void NeroAlwaysInitialDT::on_config_save(utility::Config& cfg) {
}

void NeroAlwaysInitialDT::on_draw_ui() { // this doesn't work
  m_patch01->toggle(&NeroAlwaysInitialDT::cheaton);
  m_patch02->toggle(&NeroAlwaysInitialDT::cheaton);
}
*/
