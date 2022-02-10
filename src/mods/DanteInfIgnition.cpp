#include "DanteInfIgnition.hpp"
#include "PlayerTracker.hpp"
uintptr_t DanteInfIgnition::jmp_ret{NULL};
uintptr_t DanteInfIgnition::jmp_ret2{NULL};
bool DanteInfIgnition::cheaton{NULL};

    // clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
    validation:
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code

        cmp byte ptr [DanteInfIgnition::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cvtpd2ps xmm2, xmm1
		jmp qword ptr [DanteInfIgnition::jmp_ret]

    code:
        subsd xmm1, xmm0
        cvtpd2ps xmm2, xmm1
        jmp qword ptr [DanteInfIgnition::jmp_ret]
	}
}
static naked void detour2() {
	__asm {
    validation:
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code

        cmp byte ptr [DanteInfIgnition::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        mov eax, [rbx+00000378h]
        mov [rbx+00000374h], eax
		jmp qword ptr [DanteInfIgnition::jmp_ret2]

    code:
        mov eax, [rbx+00000378h]
        jmp qword ptr [DanteInfIgnition::jmp_ret2]
	}
}

// clang-format on

void DanteInfIgnition::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DanteInfIgnition::on_initialize() {
  init_check_box_info();

  m_is_enabled            = &DanteInfIgnition::cheaton;
  m_on_page               = dantecheat;

  m_full_name_string     = "Infinite Ignition";
  m_author_string        = "SSSiyan";
  m_description_string   = "Sets ignition to full when you switch modes.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "F2 0F 5C C8 66 0F 5A D1 F3 0F 11 97 68 03 00 00 48 8B 43 50 48 39 68 18 0F 85 17");
  if (!addr) {
    return "Unable to find DanteInfIgnition pattern.";
  }
  auto addr2 = patterns->find_addr(base, "65 8B 83 74 03 00 00 0F 57 C0 0F 57 D2 F2 48 0F 2A C0 8B 83 78 03 00 00");
  if (!addr2) {
    return "Unable to find DanteInfIgnition2 pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteInfIgnition";
  }
  if (!install_hook_absolute(addr2.value()+18, m_function_hook2, &detour2, &jmp_ret2, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteInfIgnition2";
  }

  return Mod::on_initialize();
}

void DanteInfIgnition::on_draw_ui() {}
