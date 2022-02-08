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
        push rbx
        setturbospeed:
        mov ebx, [DeepTurbo::turbospeed]
        deepturbo:
        mov [rax+00000388h], ebx
        movss xmm0, [rax+00000388h]
        pop rbx
        jmp qword ptr [DeepTurbo::jmp_ret2]

        menucheck:
        cmp byte ptr [DeepTurbo::isCutscene], 1
        je code
        push rbx
        //cmp dword ptr [EnemySwapper::nowFlow], 0x11 // start //but it doesnt work for some reason. Guess Deep's prev check also skips it
        //je menuturbo
        cmp dword ptr [EnemySwapper::gameMode], 1 //secretMission
        je setturbospeed
        cmp dword ptr [EnemySwapper::nowFlow], 0x16//Gameplay
        jne menuturbo
        mov rbx, [DeepTurbo::pauseBase]
        mov rbx, [rbx]
        mov rbx, [rbx + 0x100]
        mov rbx, [rbx + 0x288]
        mov rbx, [rbx + 0xC8]
        mov rbx, [rbx + 0x5C4]
        cmp bx, 1
        jne setturbospeed

        menuturbo:
        mov ebx, [DeepTurbo::menuSpeed]
        jmp deepturbo

    //menuspeedupcheck:
        //cmp byte ptr [shouldMenuSpeedup], 1
        //jne code
        //push rbx
        //mov ebx, [menuspeed]
        //mov [rax+00000388h], ebx
        //movss xmm0, [rax+00000388h]
        //pop rbx
        //jmp qword ptr [DeepTurbo::jmp_ret2]

    code:
        push rbx
        mov ebx, [defscale]
        mov [rax+00000388h], ebx
        movss xmm0, [rax+00000388h]
        pop rbx
        jmp qword ptr [DeepTurbo::jmp_ret2]
	}
}

static naked void is_cutscene_detour()
{
    __asm {
        mov byte ptr [DeepTurbo::isCutscene], cl
        mov [rsi + 0x00000094], cl
        jmp qword ptr [DeepTurbo::isCutsceneRet]
    }
}

// clang-format on

void DeepTurbo::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DeepTurbo::on_initialize() {
  init_check_box_info();

  ischecked          = &DeepTurbo::cheaton;
  onpage             = mechanics;

  full_name_string   = "Turbo (+)";
  author_string      = "DeepDarkKapustka";
  description_string = "Change the game speed by adjusting the slider.\n\n"
                       "1.2 is the value used in DMC3 and DMC4.";

  auto base  = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr1 = patterns->find_addr(base, "89 46 68 44 89 6E 6C");
  if (!addr1) {
    return "Unable to find DeepTurbo1 pattern.";
  }
  auto addr2 = patterns->find_addr(base, "F3 0F 10 80 88 03 00 00 48");
  if (!addr2) {
    return "Unable to find DeepTurbo2 pattern.";
  }

  auto isCutsceneAddr = patterns->find_addr(base, "88 8E 94 00 00 00");//DevilMayCry5.exe+FD9606
  if (!isCutsceneAddr)
  {
      return "Unable to find DeepTurbo.isCutsceneAddr.";
  }

  pauseBase = g_framework->get_module().as<uintptr_t>() + 0x7E55910;

  if (!install_hook_absolute(addr1.value(), m_function_hook1, &detour1, &jmp_ret1, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DeepTurbo1";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DeepTurbo2";
  }

  if (!install_hook_absolute(isCutsceneAddr.value(), m_cutscene_hook, &is_cutscene_detour, &isCutsceneRet, 6))
  {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize DeepTurbo.isCutscene";
  }

  // save bytes, remember false = detour enabled
  m_patch01 = Patch::create(addr2.value(), {0xF3, 0x0F, 0x10, 0x80, 0x88, 0x03, 0x00, 0x00}, false);

  return Mod::on_initialize();
}
void DeepTurbo::on_config_load(const utility::Config& cfg) {
  //ischecked = cfg.get<bool>("deep_turbo_custom").value_or(false);
  turbospeed = cfg.get<float>("deep_turbo_value").value_or(1.2f);
  menuSpeed = cfg.get<float>("DeepTurbo.menuSpeed").value_or(1.6f);
  //shouldMenuSpeedup = cfg.get<bool>("menu_speed_enable").value_or(false);
  //menuspeed         = cfg.get<float>("menu_speed_value").value_or(1.0f);
  disableTurbo      = cfg.get<bool>("disable_turbo").value_or(false);
  isSpeedUpMenu = cfg.get<bool>("DeepTurbo.isSpeedUpMenu").value_or(false);
  m_patch01->toggle(disableTurbo);

}
void DeepTurbo::on_config_save(utility::Config& cfg) {
  //cfg.set<bool>("deep_turbo_custom", ischecked);
  cfg.set<float>("deep_turbo_value", turbospeed);
  //(*cfg.set<bool>("menu_speed_enable", shouldMenuSpeedup);
  //cfg.set<float>("menu_speed_value", menuspeed);
  cfg.set<bool>("disable_turbo", disableTurbo);
  cfg.set<bool>("DeepTurbo.isSpeedUpMenu", isSpeedUpMenu);
  cfg.set<float>("DeepTurbo.menuSpeed", menuSpeed);
}

void DeepTurbo::on_draw_ui() {
  ImGui::Text("Game Speed");
  UI::SliderFloat("##Speed slider", &turbospeed, 0.0f, 2.5f, "%.1f");
  ImGui::Spacing();
  /*ImGui::Text("Menu And Cutscene Speed"); // this works but only actually sped up pause screen, cutscenes and load screens
  ImGui::Checkbox("Variable menu/cutscene speed", &shouldMenuSpeedup);
  UI::SliderFloat("##Menu Speed slider", &menuspeed, 0.0f, 2.5f, "%.1f");*/
  if (ImGui::Checkbox("Disable turbo writing to allow camera tool to freeze the game", &disableTurbo)) {
    m_patch01->toggle(disableTurbo);
  }
  ImGui::ShowHelpMarker("Enable this before using the camera tool if you want to use its built in freeze function.", 450.0f);
  ImGui::Spacing();
  ImGui::Checkbox("Separate speed for menus", &isSpeedUpMenu);
  if (isSpeedUpMenu)
  {
      ImGui::TextWrapped("Menu speed:");
      ImGui::SliderFloat("##menuSpeed", &menuSpeed, 1.0f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
  }
}

void DeepTurbo::on_draw_debug_ui() {
  ImGui::Text("Deep Turbo: %.1f", turbospeed);
}
