#include "DeepTurbo.hpp"

uintptr_t DeepTurbo::jmp_ret1{NULL};
uintptr_t DeepTurbo::jmp_ret2{NULL};

float defscale   = 1.0;
//float menuspeed  = 2.0f;
int state        = 0;
//bool shouldMenuSpeedup{NULL};
bool DeepTurbo::cheaton{NULL};
bool disableTurbo{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour1() {
	__asm {
        mov [rsi+68h], eax
        mov [state], eax
        mov [rsi+6Ch], r13d
		jmp qword ptr [DeepTurbo::jmp_ret1]
	}
}

static naked void detour2() {
	__asm {
		cmp byte ptr [DeepTurbo::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [DeepTurbo::isSpeedUpMenu], 1
        je menucheck
        xor byte ptr [state], 0
        jnz code
    setturbospeed:
        mulss xmm1, [DeepTurbo::turbospeed]
        jmp qword ptr [DeepTurbo::jmp_ret2]

    menucheck:
        cmp byte ptr [GameplayStateTracker::isCutscene], 1
        je code
        cmp dword ptr [GameplayStateTracker::gameMode], 1 //secretMission
        je setturbospeed
        cmp dword ptr [GameplayStateTracker::nowFlow], 0x16 //Gameplay
        jne menuturbo
        cmp byte ptr [GameplayStateTracker::isExecutePause], 1 //Paused
        jne setturbospeed

    menuturbo:
        mulss xmm1, [DeepTurbo::menuSpeed]
        jmp qword ptr [DeepTurbo::jmp_ret2]

    code:
        mulss xmm1, [rax+00000388h]
        jmp qword ptr [DeepTurbo::jmp_ret2]
	}
}

// clang-format on

void DeepTurbo::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DeepTurbo::on_initialize() {
  init_check_box_info();

  m_is_enabled         = &DeepTurbo::cheaton;
  m_on_page            = Page_Mechanics;
  m_depends_on         = { "GameplayStateTracker" };
  m_full_name_string   = "Turbo (+)";
  m_author_string      = "deepdarkkapustka";
  m_description_string = "Change the game speed by adjusting the slider.\n\n"
                       "1.2 is the value used in DMC3 and DMC4.";

  set_up_hotkey();

  auto base  = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr1 = m_patterns_cache->find_addr(base, "89 46 68 44 89 6E 6C");
  if (!addr1) {
    return "Unable to find DeepTurbo1 pattern.";
  }
  auto addr2 = m_patterns_cache->find_addr(base, "F3 0F 59 8B 88 03 00 00");
  if (!addr2) {
    return "Unable to find DeepTurbo2 pattern.";
  }

  pauseBase = g_framework->get_module().as<uintptr_t>() + 0x7E55910;

  if (!install_new_detour(addr1.value(), m_detour1, &detour1, &jmp_ret1, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DeepTurbo1";
  }
  if (!install_new_detour(addr2.value(), m_detour2, &detour2, &jmp_ret2, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DeepTurbo2";
  }

  return Mod::on_initialize();
}
void DeepTurbo::on_config_load(const utility::Config& cfg) {
  turbospeed = cfg.get<float>("deep_turbo_value").value_or(1.2f);
  menuSpeed = cfg.get<float>("DeepTurbo.menuSpeed").value_or(1.6f);
  disableTurbo      = cfg.get<bool>("disable_turbo").value_or(false);
  isSpeedUpMenu = cfg.get<bool>("DeepTurbo.isSpeedUpMenu").value_or(false);
  m_patch01->toggle(disableTurbo);

}
void DeepTurbo::on_config_save(utility::Config& cfg) {
  cfg.set<float>("deep_turbo_value", turbospeed);
  cfg.set<bool>("disable_turbo", disableTurbo);
  cfg.set<bool>("DeepTurbo.isSpeedUpMenu", isSpeedUpMenu);
  cfg.set<float>("DeepTurbo.menuSpeed", menuSpeed);
}

void DeepTurbo::on_draw_ui() {
  ImGui::Text("Game Speed");
  UI::SliderFloat("##Speed slider", &turbospeed, 0.0f, 2.5f, "%.1f");
  ImGui::Spacing();
  ImGui::ShowHelpMarker("Enable this before using the camera tool if you want to use its built in freeze function.", 450.0f);
  ImGui::Spacing();
  ImGui::Checkbox("Separate speed for menus", &isSpeedUpMenu);
  if (isSpeedUpMenu)
  {
      ImGui::TextWrapped("Menu speed:");
      UI::SliderFloat("##menuSpeed", &menuSpeed, 1.0f, 10.0f, "%.1f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
  }
}

void DeepTurbo::on_draw_debug_ui() {
  ImGui::Text("Deep Turbo: %.1f", turbospeed);
}
