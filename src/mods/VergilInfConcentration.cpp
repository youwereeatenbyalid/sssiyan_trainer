
#include "VergilInfConcentration.hpp"
#include "PlayerTracker.hpp"
uintptr_t VergilInfConcentration::jmp_ret{NULL};
bool VergilInfConcentration::cheaton{NULL};
int concentrationLevel     = 2;
float concentrationZero    = 0.0f;
float concentrationOne     = 100.0f;
float concentrationTwo     = 300.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code

        cmp byte ptr [VergilInfConcentration::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp dword ptr [concentrationLevel], 0
        je write0
        cmp dword ptr [concentrationLevel], 1
        je write1
        movss xmm2, [concentrationTwo]
        movss [rbx+00001B50h], xmm2
		jmp qword ptr [VergilInfConcentration::jmp_ret]

    write0:
        movss xmm2, [concentrationZero]
        movss [rbx+00001B50h], xmm2
		jmp qword ptr [VergilInfConcentration::jmp_ret]

    write1:
        movss xmm2, [concentrationOne]
        movss [rbx+00001B50h], xmm2
		jmp qword ptr [VergilInfConcentration::jmp_ret]

    code:
        movss xmm2,[rbx+00001B50h]
		jmp qword ptr [VergilInfConcentration::jmp_ret]
	}
}

// clang-format on

void VergilInfConcentration::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> VergilInfConcentration::on_initialize() {
  init_check_box_info();

  ischecked            = &VergilInfConcentration::cheaton;
  onpage               = vergilcheat;

  full_name_string     = "Concentration Lock (+)";
  author_string        = "SSSiyan";
  description_string   = "Lock Concentration meter to whatever level you want.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "F3 0F 10 93 50 1B 00 00");
  if (!addr) {
    return "Unable to find VergilInfConcentration pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilInfConcentration";
  }
  return Mod::on_initialize();
}

void VergilInfConcentration::on_config_load(const utility::Config& cfg) {
  concentrationLevel = cfg.get<int>("vergil_concentration_level").value_or(2);
}

void VergilInfConcentration::on_config_save(utility::Config& cfg) {
  cfg.set<int>("vergil_concentration_level", concentrationLevel);
}

void VergilInfConcentration::on_draw_ui() {
  ImGui::Text("Concentration level");
  ImGui::SliderInt("##concentrationslider", &concentrationLevel, 0, 2);
}
