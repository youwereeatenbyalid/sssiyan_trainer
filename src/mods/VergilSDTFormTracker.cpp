#include "VergilSDTFormTracker.hpp"

// clang-format off

uintptr_t VergilSDTFormTracker::vergilform_ret{NULL};
uint32_t VergilSDTFormTracker::vergilform_state{0};


static naked void getvergilform_detour() {
	__asm {
		mov [rdx+0x000009B0],r8d
		mov [VergilSDTFormTracker::vergilform_state], r8d
		mov rbx,rcx
		mov rax,[rcx+0x50]
		//mb I should check jjc count for additional jjc here?
		jmp [VergilSDTFormTracker::vergilform_ret]
  }
}

// clang-format on

void VergilSDTFormTracker::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> VergilSDTFormTracker::on_initialize() {
  init_check_box_info();
  auto base      = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto init_addr = patterns->find_addr(base, "44 89 82 B0 09 00 00 48");
  if (!init_addr) {
    return "Unanable to find VergilSDTFormTracker pattern.";
  }

  if (!install_hook_absolute(init_addr.value(), m_vergilsdtformtracker_hook, getvergilform_detour, &vergilform_ret, 0xE)){
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilSDTFormTracker";  
  }

  return Mod::on_initialize();
}

// void VergilSDTFormTracker::on_config_load(const utility::Config& cfg) {}

// void VergilSDTFormTracker::on_config_save(utility::Config& cfg) {}

// void VergilSDTFormTracker::on_frame() {}

// void VergilSDTFormTracker::on_draw_ui() {}

void VergilSDTFormTracker::on_draw_debug_ui() {
  ImGui::Text("[VergilSDTFormTracker] Vergil in form: %X", vergilform_state);
}
