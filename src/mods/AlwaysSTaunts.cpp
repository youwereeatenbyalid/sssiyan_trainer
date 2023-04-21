#include "AlwaysSTaunts.hpp"
#include "PlayerTracker.hpp"
uintptr_t AlwaysSTaunts::jmp_ret{NULL};
uintptr_t AlwaysSTaunts::jmp_ret2{NULL};
uintptr_t AlwaysSTaunts::jmp_ret3{NULL};
bool AlwaysSTaunts::cheaton{NULL};

int lowstylerank  = 0;
int highstylerank = 7;
bool sminus;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [AlwaysSTaunts::cheaton], 1
        je cheatcode
        jmp code

        cheatcode:
        mov ecx, 7
		jmp qword ptr [AlwaysSTaunts::jmp_ret]

        code:
        mov ecx, [rax+000000B0h]
        jmp qword ptr [AlwaysSTaunts::jmp_ret]
	}
}

static naked void detour2() {
	__asm {
        cmp byte ptr [AlwaysSTaunts::cheaton], 1
        je cheatcode
        jmp code

        cheatcode:
        cmp byte ptr [sminus], 1
        je lowstyle
        jmp highstyle

    highstyle:
        mov eax, [highstylerank]
		jmp qword ptr [AlwaysSTaunts::jmp_ret2]

    lowstyle:
        mov eax, [lowstylerank]
		jmp qword ptr [AlwaysSTaunts::jmp_ret2]

        code:
        mov eax, [rax+000000B0h]
        jmp qword ptr [AlwaysSTaunts::jmp_ret2]
	}
}

static naked void detour3() {
	__asm {
		cmp byte ptr [AlwaysSTaunts::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [sminus], 1
        je lowstyle
        jmp highstyle

    highstyle:
        mov eax, [highstylerank]
		jmp qword ptr [AlwaysSTaunts::jmp_ret3]

    lowstyle:
        mov eax, [lowstylerank]
		jmp qword ptr [AlwaysSTaunts::jmp_ret3]

        code:
        mov eax, [rax+000000B0h]
        jmp qword ptr [AlwaysSTaunts::jmp_ret3]
	}
}

// clang-format on

void AlwaysSTaunts::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> AlwaysSTaunts::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &AlwaysSTaunts::cheaton;
  m_on_page             = Page_Taunt;

  m_full_name_string   = "Always S+ Taunts (+)";
  m_author_string      = "SSSiyan";
  m_description_string = "Restricts your taunts to those that play when at S rank or above.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr      = m_patterns_cache->find_addr(base, "8B 88 B0 00 00 00 48 8B 15");
  if (!addr) {
    return "Unable to find AlwaysSTaunts pattern.";
  }
  auto addr2 = m_patterns_cache->find_addr(
      base, "8B 80 B0 00 00 00 83 E8 05 83 F8 02 0F 96 C1 EB D0 48");
  if (!addr2) {
    return "Unable to find AlwaysSTaunts2 pattern.";
  }
  auto addr3 = m_patterns_cache->find_addr(
      base, "8B 80 B0 00 00 00 83 E8 05 83 F8 02 0F 96 C1 EB D0 41");
  if (!addr3) {
    return "Unable to find AlwaysSTaunts3 pattern.";
  }

  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize AlwaysSTaunts";
  }
  if (!install_new_detour(addr2.value(), m_detour2, &detour2, &jmp_ret2, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilAirTauntSelect2";
  }
  if (!install_new_detour(addr3.value(), m_detour3, &detour3, &jmp_ret3, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilAirTauntSelect3";
  }
  return Mod::on_initialize();
}

void AlwaysSTaunts::on_config_load(const utility::Config& cfg) {
  sminus = cfg.get<bool>("vergil_air_taunt_sminus").value_or(false);
}
void AlwaysSTaunts::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("vergil_air_taunt_sminus", sminus);
}

void AlwaysSTaunts::on_draw_ui() {
  ImGui::Checkbox("Force Vergil's Launcher Air Taunt instead", &sminus);
}
