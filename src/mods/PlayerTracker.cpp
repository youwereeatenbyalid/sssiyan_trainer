#include "PlayerTracker.hpp"
  uintptr_t PlayerTracker::jmp_ret1{NULL};
  uintptr_t PlayerTracker::jmp_ret2{NULL};
  uintptr_t PlayerTracker::incombat_jmp_ret{NULL};
  uintptr_t PlayerTracker::jmp_je{NULL};

  uintptr_t PlayerTracker::playerentity{NULL};
  uint32_t PlayerTracker::playerid{0};
  uintptr_t PlayerTracker::groundedmem{NULL};
  uint32_t PlayerTracker::isgrounded{0};
  uintptr_t PlayerTracker::playertransform{NULL};

  uint32_t PlayerTracker::playermoveid{0};
  
  uintptr_t PlayerTracker::neroentity{NULL};
  uintptr_t PlayerTracker::nerotransform{NULL};
  
  uintptr_t PlayerTracker::danteentity{NULL};
  uintptr_t PlayerTracker::dantetransform{NULL};
  uintptr_t PlayerTracker::danteweapon{NULL}; // DevilMayCry5.exe+1986263 - mov [rdi+000018B0],r15d Vergil exe 1
  
  uintptr_t PlayerTracker::ventity{NULL};
  uintptr_t PlayerTracker::vtransform{NULL};

  uintptr_t PlayerTracker::shadowcontroller{NULL};
  uintptr_t PlayerTracker::shadowentity{NULL};
  uintptr_t PlayerTracker::shadowtransform{NULL};

  uintptr_t PlayerTracker::griffoncontroller{NULL};
  uintptr_t PlayerTracker::griffonentity{NULL};
  uintptr_t PlayerTracker::griffontransform{NULL};

  uintptr_t PlayerTracker::nightmarecontroller{NULL};
  uintptr_t PlayerTracker::nightmareentity{NULL};
  uintptr_t PlayerTracker::nightmaretransform{NULL};
  
  uintptr_t PlayerTracker::vergilentity{NULL};
  uintptr_t PlayerTracker::vergiltransform{NULL};

  uint32_t PlayerTracker::incombat{0};
  
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour1() {
	__asm {
		//playerentity
		push r8
		push r10
		cmp qword ptr [rdx+0x60], 0
		je playerarray

		mov r9, [rdx+0x60]
		mov [PlayerTracker::playerentity], r9

		//playerid
		mov r9, [r9+0xE64]
		mov [PlayerTracker::playerid], r9


		//groundmem
		mov r9, [rdx+0x60]
		mov r9, [r9+0x3B0]
		lea r9, [r9+0x10]
		mov [PlayerTracker::groundedmem], r9

		//isgrounded
		mov r9, [rdx+0x60]
		mov r9, [r9+0x3B0]
		mov r9, [r9+0x18]
		mov [PlayerTracker::isgrounded], r9

		//playertransform
		mov r9, [rdx+0x60]
		mov r9, [r9+0x1F0]
		mov [PlayerTracker::playertransform], r9
			
		mov r9, [rdx+0x60]

		mov r9, [r9+0x000002E0]
		test r9, r9
		je playerarray

		mov r9, [r9+0x000000D8]
		test r9, r9
		je playerarray

		mov r10, [r9+0x2C]
		mov [PlayerTracker::playermoveid], r10d
		mov r10, [r9+0x34]
		mov [PlayerTracker::playermoveid+2], r10d

		playerarray:

		
		cmp qword ptr [rdx+0x78], 0
		je playerexit

		mov r9, [rdx+0x78]

		playernero:
		cmp qword ptr [r9+0x20], 0
		je playerdante

		mov r8, [r9+0x20]
		mov [PlayerTracker::neroentity], r8
		mov r8, [r8+0x1F0]
		mov [PlayerTracker::nerotransform], r8

		playerdante:
		cmp qword ptr [r9+0x28], 0
		je playerv

		mov r8, [r9+0x28]
		mov [PlayerTracker::danteentity], r8
		mov r8, [r8+0x1F0]
		mov [PlayerTracker::dantetransform], r8

		mov r8,[r9+0x28]
		mov r8, [r8+0x18B0]
		mov [PlayerTracker::danteweapon], r8 // @HELPSIYAN

		playerv:
		cmp qword ptr [r9+0x30], 0
		je playervergil

		mov r8, [r9+0x30]
		mov [PlayerTracker::ventity], r8
		mov r8, [r8+0x1F0]
		mov [PlayerTracker::vtransform], r8

		playervergil:
		cmp qword ptr [r9+0x40], 0
		je playerexit

		mov r8, [r9+0x40]
		mov [PlayerTracker::vergilentity], r8
		mov r8, [r8+0x1F0]
		mov [PlayerTracker::vergiltransform], r8


		playerexit:
		pop r10
		pop r8
		code:
		  mov r9,rcx
		  cmp r8d,-0x01
		  jmp ret_jmp

		ret_jmp:
			jmp qword ptr [PlayerTracker::jmp_ret1]
	}
}
static naked void detour2() {
	__asm {
		//this is allocated memory, you have read,write,execute access
		//place your code here
		//64 = status
		//D0-> Down to enemy
		//B18 = enemy ID
		//108 = is enemy
		//1D2 = master lock-on
		//1F0-> down to transform
		//30 = x coordinate
		//34 = y coordinate
		//38 = z coordinate
		
		push r8
		push r9

		mov r8, [rdi+0xD0]
		mov r9, [r8+0x1F0]
		cmp dword ptr [r8+0xB18], 0x20
		je writegriffon
		cmp dword ptr [r8+0xB18], 0x21
		je writeshadow
		cmp dword ptr [r8+0xB18], 0x22
		je writenightmare
		jmp originalcode

		writeshadow:
			mov [PlayerTracker::shadowcontroller], rdi
			mov [PlayerTracker::shadowentity], r8
			mov [PlayerTracker::shadowtransform], r9

		writegriffon:
			mov [PlayerTracker::griffoncontroller], rdi
			mov [PlayerTracker::griffonentity], r8
			mov [PlayerTracker::griffontransform], r9

		writenightmare:
			mov [PlayerTracker::nightmarecontroller], rdi
			mov [PlayerTracker::nightmareentity], r8
			mov [PlayerTracker::nightmaretransform], r9

		originalcode:
			pop r9
			pop r8
			cmp dword ptr [rdi+0x64],ebp
			je je_jmp
			jmp ret_jmp

		je_jmp:
			jmp qword ptr [PlayerTracker::jmp_je] //DevilMayCry5.exe+3F0756 
		ret_jmp:
		jmp qword ptr [PlayerTracker::jmp_ret2]
	}
}
static naked void incombat_detour(){
	__asm{
			mov byte ptr [PlayerTracker::incombat], 0
			cmp byte ptr [rax+0x00000ECA],sil
			je jmp_ret
			mov byte ptr [PlayerTracker::incombat], 1
		jmp_ret:
			jmp qword ptr[PlayerTracker::incombat_jmp_ret]
	}
}
    // clang-format on

std::optional<std::string> PlayerTracker::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  //player tracker
  auto addr1 = utility::scan(base, "4C 8B C9 41 83 F8 FF 74");
  if (!addr1) {
    return "Unable to find Player Tracker pattern.";
  }
  //summon tracker
  auto addr2 = utility::scan(base, "39 6F 64 0F 84 52 01 00 00");
  if (!addr2) {
    return "Unable to find Summon Tracker pattern.";
  }
  auto incombat_addr = utility::scan(base, "40 38 B0 CA 0E 00 00 0F 84 04");
  if (!incombat_addr) {
    return "Unable to find In Combat pattern.";
  }
  if (!install_hook_absolute(addr1.value(), m_function_hook1, &detour1,
                             &jmp_ret1, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize player tracker";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2,
                             &jmp_ret2, 9)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize summon tracker";
  }

  if (!install_hook_absolute(incombat_addr.value(), m_incombat_hook, &incombat_detour,
                             &incombat_jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize In Combat";
  }
  PlayerTracker::jmp_je = addr2.value() + 0x15B;


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
	ImGui::Text("[PlayerTracker] Player ID: %X", PlayerTracker::playerid);
	ImGui::Text("[PlayerTracker] Is Grounded: %X",PlayerTracker::isgrounded);
    ImGui::Text("[PlayerTracker] Move ID: %X", PlayerTracker::playermoveid);
    ImGui::Text("[PlayerTracker] In Combat: %X", PlayerTracker::incombat);
	//Imgui::Text(PlayerTracker::isgrounded)
}
// will show up in main window, dump ImGui widgets you want here
void PlayerTracker::on_draw_ui() {

}