
#include "DanteVariableGuard.hpp"
#include "PlayerTracker.hpp"
uintptr_t DanteVariableGuard::jmp_ret{NULL};
bool  DanteVariableGuard::cheaton{NULL};
float inputGuardValue    = 10.0f;
float guardMult          = 3000.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code

        cmp byte ptr [DanteVariableGuard::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        movss xmm3, [inputGuardValue]
        mulss xmm3, [guardMult]
        movss [rdx+28h], xmm3
        jmp qword ptr [DanteVariableGuard::jmp_ret]

    code:
        movss xmm3, [rdx+28h]
		jmp qword ptr [DanteVariableGuard::jmp_ret]
	}
}

// clang-format on

void DanteVariableGuard::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DanteVariableGuard::on_initialize() {
  init_check_box_info();

  ischecked            = &DanteVariableGuard::cheaton;
  onpage               = dantecheat;

  full_name_string     = "Guard Meter Lock (+)";
  author_string        = "SSSiyan";
  description_string   = "Lock Guard meter to whatever level you want.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "F3 0F 10 5A 28 0F 57");
  if (!addr) {
    return "Unable to find DanteVariableGuard pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteVariableGuard";
  }
  return Mod::on_initialize();
}

void DanteVariableGuard::on_config_load(const utility::Config& cfg) {
  inputGuardValue = cfg.get<float>("dante_guard_level").value_or(10.0f);
}

void DanteVariableGuard::on_config_save(utility::Config& cfg) {
  cfg.set<float>("dante_guard_level", inputGuardValue);
}

void DanteVariableGuard::on_draw_ui() {
  ImGui::Text("Guard level");
  UI::SliderFloat("##guardlevelslider", &inputGuardValue, 0.0f, 10.0f, "%.0f");
}
