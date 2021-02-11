
#include "ChargeChecker.hpp"
#include "PlayerTracker.hpp"
uintptr_t ChargeChecker::jmp_ret{NULL};
bool ChargeChecker::cheaton{NULL};
float neroSwordMult{NULL};
float neroGunMult{NULL};
float neroBreakerMult{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
		cmp byte ptr [ChargeChecker::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp dword ptr [rdi+48h], 1
        je swordcharge
        cmp dword ptr [rdi+48h], 2
        je guncharge
        cmp dword ptr [rdi+48h], 4096
        je breakercharge
        // cmp [rdi+48h], 4097 // all in input
		jmp code

    swordcharge:
        mulss xmm6, [neroSwordMult]
        jmp code
    guncharge:
        mulss xmm6, [neroGunMult]
        jmp code
    breakercharge:
        mulss xmm6, [neroBreakerMult]
        jmp code

    code:
        movss xmm1, [rdi+5Ch]
        jmp qword ptr [ChargeChecker::jmp_ret]
	}
}

// clang-format on

void ChargeChecker::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> ChargeChecker::on_initialize() {
  init_check_box_info();

  ischecked          = &ChargeChecker::cheaton;
  onpage             = nero;

  full_name_string   = "Faster Charges";
  author_string      = "SSSiyan";
  description_string = "Speed up or slow down charges bound to hold inputs.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "F3 0F 10 4F 5C 0F 57 C0 0F 5A C9 F3"); // "DevilMayCry5.exe"+20BE39E
  if (!addr) {
    return "Unable to find ChargeChecker pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize ChargeChecker";
  }
  return Mod::on_initialize();
}

void ChargeChecker::on_config_load(const utility::Config& cfg) {
  neroSwordMult = cfg.get<float>("nero_sword_charge").value_or(1.0f);
  neroGunMult = cfg.get<float>("nero_gun_charge").value_or(1.0f);
  neroBreakerMult = cfg.get<float>("nero_breaker_charge").value_or(1.0f);
}
void ChargeChecker::on_config_save(utility::Config& cfg) {
  cfg.set<float>("nero_sword_charge", neroSwordMult);
  cfg.set<float>("nero_gun_charge", neroGunMult);
  cfg.set<float>("nero_breaker_charge", neroBreakerMult);
}

void ChargeChecker::on_draw_ui() {
  ImGui::Text("Sword Charge Speed");
  ImGui::SliderFloat("##swordmultslider", &neroSwordMult, 0.5f, 3.0f, "%.1f");
  ImGui::Text("Gun Charge Speed");
  ImGui::SliderFloat("##gunmultslider", &neroGunMult, 0.5f, 3.0f, "%.1f");
  ImGui::Text("Breaker Charge Speed");
  ImGui::SliderFloat("##breakermultslider", &neroBreakerMult, 0.5f, 3.0f, "%.1f");
}
