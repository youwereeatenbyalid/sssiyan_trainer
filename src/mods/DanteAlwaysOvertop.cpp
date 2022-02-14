
#include "DanteAlwaysOvertop.hpp"
#include "PlayerTracker.hpp"
uintptr_t DanteAlwaysOvertop::jmp_ret{NULL};
uintptr_t DanteAlwaysOvertop::jmp_ret2{NULL};
uintptr_t DanteAlwaysOvertop::jmp_ret3{NULL};
bool DanteAlwaysOvertop::cheaton{NULL};


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code
		cmp byte ptr [DanteAlwaysOvertop::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        mov dword ptr [rdi+00000340h], 2 
		jmp qword ptr [DanteAlwaysOvertop::jmp_ret]

    code:
        mov [rdi+00000340h], esi
        jmp qword ptr [DanteAlwaysOvertop::jmp_ret]
	}
}
static naked void detour2() {
	__asm {
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code
		cmp byte ptr [DanteAlwaysOvertop::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        mov dword ptr [rsi+000000BCh], 15h
		jmp qword ptr [DanteAlwaysOvertop::jmp_ret2]

    code:
        mov [rsi+000000BCh], eax
        jmp qword ptr [DanteAlwaysOvertop::jmp_ret2]
	}
}
static naked void detour3() {
	__asm {
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code
		cmp byte ptr [DanteAlwaysOvertop::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        mov dword ptr [rdi+00000340h], 2
		jmp qword ptr [DanteAlwaysOvertop::jmp_ret3]

    code:
        mov [rdi+00000340h], ebp
        jmp qword ptr [DanteAlwaysOvertop::jmp_ret3]
	}
}

// clang-format on

void DanteAlwaysOvertop::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DanteAlwaysOvertop::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &DanteAlwaysOvertop::cheaton;
  m_on_page             = dantecheat;

  m_full_name_string   = "Cavaliere Always Overtop";
  m_author_string      = "SSSiyan";
  m_description_string = "Forces Cavaliere to always use its highest gear.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "89 B7 40 03 00 00 48 8B");
  if (!addr) {
    return "Unable to find DanteAlwaysOvertop pattern1.";
  }
  auto addr2 = patterns->find_addr(base, "89 86 BC 00 00 00 4C");
  if (!addr2) {
    return "Unable to find DanteAlwaysOvertop pattern2.";
  }
  auto addr3 = patterns->find_addr(base, "09 00 00 48 8B 43 50 48 83 78 18 00 75 26 48 85 FF 74 A1 89 AF 40 03 00 00");
  if (!addr3) {
    return "Unable to find DanteAlwaysOvertop pattern3.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteAlwaysOvertop1";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteAlwaysOvertop2";
  }
  if (!install_hook_absolute(addr3.value()+19, m_function_hook3, &detour3, &jmp_ret3, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteAlwaysOvertop3";
  }
  return Mod::on_initialize();
}

// void DanteAlwaysOvertop::on_draw_ui() {}
