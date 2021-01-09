#include "PlayerTracker.hpp"
  uintptr_t PlayerTracker::jmp_ret{NULL};
  uintptr_t PlayerTracker::playerentity{NULL};
  uint32_t PlayerTracker::playerid{0};
  uintptr_t PlayerTracker::groundedmem{NULL};
  uint32_t PlayerTracker::isgrounded{0};
  uintptr_t PlayerTracker::playertransform{NULL};
  
  uintptr_t PlayerTracker::neroentity{NULL};
  uintptr_t PlayerTracker::nerotransform{NULL};
  
  uintptr_t PlayerTracker::danteentity{NULL};
  uintptr_t PlayerTracker::dantetransform{NULL};
  
  uintptr_t PlayerTracker::ventity{NULL};
  uintptr_t PlayerTracker::vtransform{NULL};
  
  uintptr_t PlayerTracker::vergilentity{NULL};
  uintptr_t PlayerTracker::vergiltransform{NULL};
  
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
		manualplayer:
		//playerentity

		cmp [rdx+60h], 0
		je playerarray

		mov r9, [rdx+60h]
		mov [PlayerTracker::playerentity], r9

		//playerid
		mov r9, [r9+E64]
		mov [PlayerTracker::playerid], r9


		//groundmem
		mov r9, [rdx+60h]
		mov r9, [r9+3B0h]
		lea r9, [r9+10h]
		mov [PlayerTracker::groundedmem], r9

		//isgrounded
		mov r9, [rdx+60h]
		mov r9, [r9+3B0h]
		mov r9, [r9+18h]
		mov [PlayerTracker::isgrounded], r9

		//playertransform
		mov r9, [rdx+60h]
		mov r9, [r9+1F0h]
		mov [PlayerTracker::playertransform], r9

		playerarray:
		push r8
		cmp [rdx+78h], 0
		je playerexit

		mov r9, [rdx+78h]

		playernero:
		cmp [r9+20h], 0
		je playerdante

		mov r8, [r9+20h]
		mov [PlayerTracker::neroentity], r8
		mov r8, [r8+1F0h]
		mov [PlayerTracker::nerotransform], r8

		playerdante:
		cmp [r9+28h], 0
		je playerv

		mov r8, [r9+28h]
		mov [PlayerTracker::danteentity], r8
		mov r8, [r8+1F0h]
		mov [PlayerTracker::dantetransform], r8

		playerv:
		cmp [r9+30h], 0
		je playervergil

		mov r8, [r9+30h]
		mov [PlayerTracker::ventity], r8
		mov r8, [r8+1F0h]
		mov [PlayerTracker::vtransform], r8

		playervergil:
		cmp [r9+40h], 0
		je playerexit

		mov r8, [r9+40h]
		mov [PlayerTracker::vergilentity], r8
		mov r8, [r8+1F0h]
		mov [PlayerTracker::vergiltransform], r8


		playerexit:
		pop r8
		code:
		  mov r9,rcx
		  cmp r8d,-01h
		  jmp ret_jmp

		ret_jmp:
			jmp qword ptr [PlayerTracker::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> PlayerTracker::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  if (!install_hook_offset(offsets::PLAYER_TRACKER, m_function_hook, &detour, &jmp_ret, 7)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize PlayerTracker";
  //}
  return Mod::on_initialize();
}

// during load
void PlayerTracker::on_config_load(const utility::Config &cfg) {}
// during save
void PlayerTracker::on_config_save(utility::Config &cfg) {}
// do something every frame
void PlayerTracker::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void PlayerTracker::on_draw_debug_ui() {
	ImGui::Text("PlayerTracker Debug")
	Imgui::Text("Is Grounded:")
	//Imgui::Text(PlayerTracker::isgrounded)
}
// will show up in main window, dump ImGui widgets you want here
void PlayerTracker::on_draw_ui() {
	Imgui::Text("PlayerTracker Main")
}