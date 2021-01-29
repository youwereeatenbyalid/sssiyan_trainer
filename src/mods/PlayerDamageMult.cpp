
#include "PlayerDamageMult.hpp"
#include "PlayerTracker.hpp"
uintptr_t PlayerDamageMult::jmp_ret{NULL};
uintptr_t PlayerDamageMult::jmp_ret2{NULL};
bool PlayerDamageMult::cheaton{NULL};

float desireddamageoutput = 1.0f;
float desireddamageinput  = 1.0f;
// int gamedifficultyreference;


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        // cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        // jne code
        cmp byte ptr [PlayerDamageMult::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        // push r11
        // mov r11, [addr3] // DevilMayCry5.exe+7E5BC40
        // mov r11, [r11+88h]
        // mov [gamedifficultyreference],r11d
        // pop r11
        movss xmm3,[rcx+rax*4+20h]
        mulss xmm3,[desireddamageoutput]
		jmp qword ptr [PlayerDamageMult::jmp_ret]

    code:
        movss xmm3,[rcx+rax*4+20h]
        jmp qword ptr [PlayerDamageMult::jmp_ret]
	}
}

static naked void detour2() {
	__asm {
        // cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        // jne code
        cmp byte ptr [PlayerDamageMult::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        movss xmm3,[rcx+rdx*4+20h]
        mulss xmm3,[desireddamageinput]
		jmp qword ptr [PlayerDamageMult::jmp_ret]

    code:
        movss xmm3,[rcx+rdx*4+20h]
        jmp qword ptr [PlayerDamageMult::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> PlayerDamageMult::on_initialize() {
  ischecked          = &PlayerDamageMult::cheaton;
  onpage             = gamemode;
  full_name_string   = "Damage Multiplier (+)";
  author_string      = "Dante";
  description_string = "Adjust the damage output of players and enemies.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "F3 0F 10 5C 81 20");
  if (!addr) {
    return "Unable to find PlayerDamageMult pattern.";
  }
  auto addr2 = utility::scan(base, "F3 0F 10 5C 91 20 48");
  if (!addr2) {
    return "Unable to find PlayerDamageMult2 pattern.";
  }
  // auto addr3 = utility::scan(base, "00 E3 01 FC BC 02 00 00 70"); // Game Difficulty Base
  // if (!addr3) {
  //   return "Unable to find PlayerDamageMult3 pattern.";
  // }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize PlayerDamageMult";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize PlayerDamageMult";
  }
  return Mod::on_initialize();
}

void PlayerDamageMult::on_config_load(const utility::Config& cfg) {
  desireddamageoutput = cfg.get<float>("player_damage_output").value_or(1.0f);
  desireddamageinput  = cfg.get<float>("player_damage_input").value_or(1.0f);
}

void PlayerDamageMult::on_config_save(utility::Config& cfg) {
  cfg.set<float>("player_damage_output", desireddamageoutput);
  cfg.set<float>("player_damage_input", desireddamageinput);
}

void PlayerDamageMult::on_draw_ui() {
  ImGui::PushItemWidth(120);
  ImGui::InputFloat("Player Damage Output", &desireddamageoutput, 0.1f);
  ImGui::InputFloat("Enemy Damage Output", &desireddamageinput, 0.1f);
  ImGui::PopItemWidth();
}
