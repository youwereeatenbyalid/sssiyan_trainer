#include "DeepTurbo.hpp"

uintptr_t DeepTurbo::jmp_ret1{NULL};

float defscale   = 1.0;
bool DeepTurbo::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour1() {
	__asm {
		cmp byte ptr [DeepTurbo::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [DeepTurbo::isSpeedUpMenu], 1
        je menucheck
    setturbospeed:
        mulss xmm1, [DeepTurbo::turbospeed]
        jmp qword ptr [DeepTurbo::jmp_ret1]

    menucheck:
        cmp byte ptr [GameplayStateTracker::isCutscene], 1
        je code
        cmp dword ptr [GameplayStateTracker::gameMode], 1 //secretMission
        je setturbospeed
        cmp dword ptr [GameplayStateTracker::nowFlow], 0x16 //Gameplay
        jne menuturbo
        cmp byte ptr [GameplayStateTracker::isExecutePause], 1 //Paused
        jne setturbospeed
        jmp code

    menuturbo:
        mulss xmm1, [DeepTurbo::menuSpeed]
        jmp qword ptr [DeepTurbo::jmp_ret1]

    code:
        mulss xmm1, [rbx+00000388h]
        jmp qword ptr [DeepTurbo::jmp_ret1]
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
  auto addr1 = m_patterns_cache->find_addr(base, "F3 0F 59 8B 88 03 00 00");
  if (!addr1) {
    return "Unable to find DeepTurbo1 pattern.";
  }

  pauseBase = g_framework->get_module().as<uintptr_t>() + 0x7E55910;

  if (!install_new_detour(addr1.value(), m_detour1, &detour1, &jmp_ret1, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DeepTurbo1";
  }

  return Mod::on_initialize();
}
void DeepTurbo::on_config_load(const utility::Config& cfg) {
  turbospeed = cfg.get<float>("deep_turbo_value").value_or(1.2f);
  menuSpeed = cfg.get<float>("DeepTurbo.menuSpeed").value_or(1.6f);
  isSpeedUpMenu = cfg.get<bool>("DeepTurbo.isSpeedUpMenu").value_or(false);

}
void DeepTurbo::on_config_save(utility::Config& cfg) {
  cfg.set<float>("deep_turbo_value", turbospeed);
  cfg.set<bool>("DeepTurbo.isSpeedUpMenu", isSpeedUpMenu);
  cfg.set<float>("DeepTurbo.menuSpeed", menuSpeed);
}

void DeepTurbo::on_draw_ui() {
  ImGui::Text("Game Speed");
  UI::SliderFloat("##Speed slider", &turbospeed, 0.0f, 2.5f, "%.1f");
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
