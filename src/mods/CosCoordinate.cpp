#include "CosCoordinate.hpp"
#include "PlayerTracker.hpp"
uintptr_t CosCoordinate::jmp_ret{NULL};
float CosCoordinate::directioncosvalue{0.0};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
		newmem: 
		cmp rdi, [PlayerTracker::playerentity]
		jne coscoordinateoriginalcode

		movss dword ptr [CosCoordinate::directioncosvalue], xmm0

		coscoordinateoriginalcode:
		mulss xmm0,[rdi+0x00000F88]

		coscoordinateexit:
		jmp qword ptr [CosCoordinate::jmp_ret]
	}
}

// clang-format on

void CosCoordinate::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> CosCoordinate::on_initialize() {
  init_check_box_info();

  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  if (!install_new_detour(offsets::COS_COORDINATE, m_detour, &detour, &jmp_ret, 8)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize CosCoordinate";
  }
  return Mod::on_initialize();
}

// during load
//void CosCoordinate::on_config_load(const utility::Config &cfg) {}
// during save
//void CosCoordinate::on_config_save(utility::Config &cfg) {}
// do something every frame
//void CosCoordinate::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void CosCoordinate::on_draw_debug_ui() { 
	ImGui::Text("Cos Value: %.4f", CosCoordinate::directioncosvalue);
}
// will show up in main window, dump ImGui widgets you want here
// void CosCoordinate::on_draw_ui() {}