
#include "SinCoordinate.hpp"
#include "PlayerTracker.hpp"
uintptr_t SinCoordinate::jmp_ret{NULL};
float SinCoordinate::directionsinvalue{0.0};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
		sincoordinatenewmem: //this is allocated memory, you have read,write,execute access
		//place your code here

		cmp rdi, [PlayerTracker::playerentity]
		jne sincoordinateoriginalcode

		movss dword ptr [SinCoordinate::directionsinvalue], xmm0

		sincoordinateoriginalcode:
		xorps xmm6,xmm6
		cvtss2sd xmm6,xmm0

		sincoordinateexit:
		jmp qword ptr [SinCoordinate::jmp_ret]
	}
}

// clang-format on

void SinCoordinate::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> SinCoordinate::on_initialize() {
  init_check_box_info();

  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  if (!install_hook_offset(offsets::SIN_COORDINATE, m_function_hook, &detour, &jmp_ret, 7)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize SinCoordinate";
  }
  return Mod::on_initialize();
}

// during load
//void SinCoordinate::on_config_load(const utility::Config &cfg) {}
// during save
//void SinCoordinate::on_config_save(utility::Config &cfg) {}
// do something every frame
//void SinCoordinate::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void SinCoordinate::on_draw_debug_ui() {
  ImGui::Text("Sin Value: %.4f", SinCoordinate::directionsinvalue);
}
// will show up in main window, dump ImGui widgets you want here
void SinCoordinate::on_draw_ui() {}
