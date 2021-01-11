
#include "DanteMaxSDT.hpp"
#include "utility/Scan.hpp"

uintptr_t DanteMaxSDT::jmp_ret{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
		mov dword ptr [rdi+0x00001A14], 0x461c4000 // 10,000
        movss xmm1, [rdi+0x00001A14]
		jmp qword ptr [DanteMaxSDT::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> DanteMaxSDT::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr      = utility::scan(base, "F3 0F 10 8F 14 1A 00 00 BA");
  if (!addr) {
    return "Unable to find DanteMaxSDT pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DanteMaxSDT";
  }
  return Mod::on_initialize();
}

// during load
// void MoveID::on_config_load(const utility::Config &cfg) {}
// during save
// void MoveID::on_config_save(utility::Config &cfg) {}
// do something every frame
// void MoveID::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void DeepTurbo::on_draw_debug_ui() {
// ImGui::Text("Deep Turbo : %.0f", turbospeed);
// }
// will show up in main window, dump ImGui widgets you want here
// void MoveID::on_draw_ui() {}
