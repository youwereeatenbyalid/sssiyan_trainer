
#include "DanteQuickSDT.hpp"
#include "PlayerTracker.hpp"

uintptr_t DanteQuickSDT::jmp_ret{NULL};
bool DanteQuickSDT::cheaton{NULL};
bool danteEvenFasterSDT{true};

float sdtspeedup = 3.0f;
float fastersdtspeedup = 5.0f;

    // clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        jne code

		cmp byte ptr [DanteQuickSDT::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [danteEvenFasterSDT], 1
        je cheatcode2
        movss xmm0, [rdi+00000128h]
        mulss xmm0, [sdtspeedup]
		jmp qword ptr [DanteQuickSDT::jmp_ret]

    cheatcode2:
        movss xmm0, [rdi+00000128h]
        mulss xmm0, [fastersdtspeedup]
		jmp qword ptr [DanteQuickSDT::jmp_ret]

    code:
        movss xmm0,[rdi+00000128h]
        jmp qword ptr [DanteQuickSDT::jmp_ret]
	}
}

// clang-format on

void DanteQuickSDT::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DanteQuickSDT::on_initialize() {
  init_check_box_info();

  ischecked            = &DanteQuickSDT::cheaton;
  onpage               = dantesdt;
  full_name_string     = "Quick SDT (+)";
  author_string        = "SSSiyan";
  description_string   = "Reduces the time you have to hold DT to enter SDT.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "FF F3 0F 10 8F 24 11 00 00 F3 0F 10 87 28 01 00 00");
  if (!addr) {
    return "Unable to find DanteQuickSDT pattern.";
  }

  if (!install_hook_absolute(addr.value()+9, m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteQuickSDT";
  }
  return Mod::on_initialize();
}

void DanteQuickSDT::on_config_load(const utility::Config& cfg) {
  danteEvenFasterSDT = cfg.get<bool>("dante_even_faster_sdt").value_or(false);
}

void DanteQuickSDT::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("dante_even_faster_sdt", danteEvenFasterSDT);
}

void DanteQuickSDT::on_draw_ui() {
  ImGui::Checkbox("Even faster SDT", &danteEvenFasterSDT);
}
