#include "NeroSwapSidesteps.hpp"
#include "PlayerTracker.hpp"
uintptr_t NeroSwapSidesteps::jmp_ret{NULL};
uintptr_t NeroSwapSidesteps::jmp_ret2{NULL};
bool NeroSwapSidesteps::cheaton{NULL};


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code

        cmp byte ptr [NeroSwapSidesteps::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        mov dword ptr [rax+10h], 00000814h
		jmp qword ptr [NeroSwapSidesteps::jmp_ret]

    code:
        mov dword ptr [rax+10h], 0000081Fh
        jmp qword ptr [NeroSwapSidesteps::jmp_ret]
	}
}

static naked void detour2() {
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code

        cmp byte ptr [NeroSwapSidesteps::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        mov dword ptr [rax+10h], 00000813h
		jmp qword ptr [NeroSwapSidesteps::jmp_ret]

    code:
        mov dword ptr [rax+10h], 00000820h
        jmp qword ptr [NeroSwapSidesteps::jmp_ret]
	}
}

// clang-format on

void NeroSwapSidesteps::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NeroSwapSidesteps::on_initialize() {
  init_check_box_info();

  m_is_enabled            = &NeroSwapSidesteps::cheaton;
  m_on_page               = nero;

  m_full_name_string     = "Swap Sidesteps with Table Hopper";
  m_author_string        = "SSSiyan";
  m_description_string   = "Swaps Nero's Sidesteps with Table Hopper dodges.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "C7 40 10 1F 08 00 00");
  if (!addr) {
    return "Unable to find NeroSwapSidesteps pattern.";
  }
  auto addr2 = patterns->find_addr(base, "C7 40 10 20 08 00 00");
  if (!addr2) {
    return "Unable to find NeroSwapSidesteps2 pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroSwapSidesteps";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroSwapSidesteps2";
  }

  return Mod::on_initialize();
}

void NeroSwapSidesteps::on_draw_ui() {}
