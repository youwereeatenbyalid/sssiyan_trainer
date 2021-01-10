
#include "MoveID.hpp"
#include "PlayerTracker.hpp"
uintptr_t MoveID::jmp_ret{NULL};
uint32_t MoveID::playermoveid{0};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
hitchmoveidnewmem: //this is allocated memory, you have read,write,execute access
//place your code here
		push r11
		push r12
		//move moveid char identifier into eax
		getentity:
		test rax, rax
		je hitchmoveidoriginalcode
		mov r11, [rax+0x18]
		test r11, r11
		je hitchmoveidoriginalcode
		mov r11, [r11+0x50]
		test r11, r11
		je hitchmoveidoriginalcode
		mov r11, [r11+0x18]
		test r11, r11
		je hitchmoveidoriginalcode
		cmp r11, [PlayerTracker::playerentity]
		je writemoveid
		jmp hitchmoveidoriginalcode



		writemoveid:
		//write the moveid of the character
		mov r11, [r14+0x000000D8]
		mov r12, [r11+0x2C]
		mov [MoveID::playermoveid], r12d
		mov r12, [r11+0x34]
		mov [MoveID::playermoveid+0x2], r12d


		hitchmoveidoriginalcode:
		pop r12
		pop r11
		mov edx,[rax+0x00000138]

		hitchmoveidexit:
		jmp qword ptr [MoveID::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> MoveID::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  if (!install_hook_offset(offsets::MOVE_ID, m_function_hook, &detour, &jmp_ret, 6)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize MoveID";
  }
  return Mod::on_initialize();
}

// during load
//void MoveID::on_config_load(const utility::Config &cfg) {}
// during save
//void MoveID::on_config_save(utility::Config &cfg) {}
// do something every frame
//void MoveID::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void MoveID::on_draw_debug_ui() {
  ImGui::Text("Move ID: %X", MoveID::playermoveid);
}
// will show up in main window, dump ImGui widgets you want here
//void MoveID::on_draw_ui() {}
