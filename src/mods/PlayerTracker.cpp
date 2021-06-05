#include "PlayerTracker.hpp"
	uintptr_t PlayerTracker::player_jmp_ret{NULL};
	uintptr_t PlayerTracker::summon_jmp_ret{NULL};
	uintptr_t PlayerTracker::incombat_jmp_ret{NULL};
	uintptr_t PlayerTracker::summon_jmp_je{NULL};
	uintptr_t PlayerTracker::sin_jmp_ret{NULL};
	uintptr_t PlayerTracker::cos_jmp_ret{NULL};
	uintptr_t PlayerTracker::threshhold_jmp_ret{NULL};
	uintptr_t PlayerTracker::threshhold_jmp_jb{NULL};
	uintptr_t PlayerTracker::vergildata_jmp_ret{NULL};
	uintptr_t PlayerTracker::playerentity{NULL};
	uint32_t PlayerTracker::playerid{0};
	uintptr_t PlayerTracker::groundedmem{NULL};
	uint32_t PlayerTracker::isgrounded{0};
	uintptr_t PlayerTracker::playertransform{NULL};
	uintptr_t PlayerTracker::playerinertiax{NULL};
	uintptr_t PlayerTracker::playerinertiay{NULL};
	uintptr_t PlayerTracker::playerinertiaz{NULL};
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
	uintptr_t PlayerTracker::yamatomodel{NULL};
	uintptr_t PlayerTracker::yamatocommonparameter{NULL};
	uintptr_t PlayerTracker::forceedgemodel{NULL};
	uintptr_t PlayerTracker::beowulfmodel{NULL};

	uintptr_t PlayerTracker::doppelentity{NULL};
	uintptr_t PlayerTracker::doppeltransform{NULL};
	uintptr_t PlayerTracker::doppelweaponmodel{NULL};
	float PlayerTracker::sinvalue{0};
	float PlayerTracker::cosvalue{0};
	bool PlayerTracker::redirect{0};
	float threshholdsubstitute = 0.35;
  
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void player_detour() {
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
	mov [PlayerTracker::isgrounded], r9b

	//playertransform
		
	mov r8, [rdx+0x60]
	lea r9, [r8+0x1140]
	mov [PlayerTracker::playerinertiax],r9
	lea r9, [r8+0x1144]
	mov [PlayerTracker::playerinertiay],r9
	lea r9, [r8+0x1148]
	mov [PlayerTracker::playerinertiaz],r9
	mov r9, [r8+0x1F0]
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
	//copy address of vergil entity
    mov r10, r8
	mov r8, [r8+0x1F0]
	mov [PlayerTracker::vergiltransform], r8
	//weapon array vergil
	mov r8, [r10+0x1970]
    mov r8, [r8+ 0x18]
	//force edge
    mov r10, [r8+0x60]
	mov [PlayerTracker::forceedgemodel], r10
	//beowulf
	mov r10, [r8+0x48]
	mov [PlayerTracker::beowulfmodel], r10
	//yamato
	mov r10, [r8+0x30]
	mov [PlayerTracker::yamatomodel], r10
	//yamatocommonparameter
    mov r10, [r10+0x10]
    mov r10, [r10+0x300]
	mov [PlayerTracker::yamatocommonparameter], r10
	//vergil entity 
	mov r8, [r9+0x40]
	//doppelganger
	mov r8, [r8+0x18B0]
	mov [PlayerTracker::doppelentity], r8
	//doppelganger transform
	mov r10, [r8+0x1F0]
	mov [PlayerTracker::doppeltransform], r10
	//doppelganger weapon
	mov r10, [r8+0x1970]
	mov r10, [r10+0x18]
	mov r10, [r10+0x30]
	mov [PlayerTracker::doppelweaponmodel], r10

	playerexit:
	pop r10
	pop r8
	code:
		mov r9,rcx
		cmp r8d,-0x01
		jmp ret_jmp

	ret_jmp:
		jmp qword ptr [PlayerTracker::player_jmp_ret]
	}
}
static naked void summon_detour() {
	__asm {
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
		jmp qword ptr [PlayerTracker::summon_jmp_je] //DevilMayCry5.exe+3F0756 
	ret_jmp:
	jmp qword ptr [PlayerTracker::summon_jmp_ret]
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
static naked void sin_detour() {
	__asm {
	sincoordinatenewmem:
		cmp rdi, [PlayerTracker::playerentity]
		jne sincoordinateoriginalcode

		movss dword ptr [PlayerTracker::sinvalue], xmm0

	sincoordinateoriginalcode:
		xorps xmm6,xmm6
		cvtss2sd xmm6,xmm0

	sincoordinateexit:
		jmp qword ptr [PlayerTracker::sin_jmp_ret]
	}
}
static naked void cos_detour() {
	__asm {
		newmem: 
		cmp rdi, [PlayerTracker::playerentity]
		jne coscoordinateoriginalcode

		movss dword ptr [PlayerTracker::cosvalue], xmm0

		coscoordinateoriginalcode:
		mulss xmm0,[rdi+0x00000F88]

		coscoordinateexit:
		jmp qword ptr [PlayerTracker::cos_jmp_ret]
	}
}
static naked void threshhold_detour() {
	__asm {
	newmem: 
		mov byte ptr [PlayerTracker::redirect], 1
		jb jbexit
		mov byte ptr [PlayerTracker::redirect], 0
		movss xmm0, [threshholdsubstitute]
		jmp qword ptr [PlayerTracker::threshhold_jmp_ret]
	jbexit:
		jmp qword ptr [PlayerTracker::threshhold_jmp_jb]
	}
}
static naked void vergildata_detour() {
	__asm {
	newmem:
		//original code line
		mov rdi, [rdi+0x10]
		cmp [PlayerTracker::playerid],4
		jne code
		//verify vergil is even loaded
		cmp [PlayerTracker::vergilentity], 0
		je code
		//verify this is a real address
		test r15, r15
		je code
		cmp r15d, 0xFFFFFFFF
		je code
		cmp r15d, 0x1000
		jb code
		//verify it's a vergil
		push rdi
		//get the player vergil
		mov rdi, [PlayerTracker::vergilentity]
		//get his id
		mov rdi, [rdi]
		//compare to id of the incoming one to confirm big vergil
		cmp [r15], rdi
		pop rdi
		jne code
		//check if it's a doppelganger
		cmp byte ptr [r15+0x17F0], 1
		je isdoppelweapon
		cmp r9, 1
		je isbeowulf
		cmp r9, 2
		je isforceedge

	isyamato:
		mov [PlayerTracker::yamatomodel], rdi
		push rdi
		mov rdi, [rdi+0x10]
		mov rdi, [rdi+0x300]
		mov [PlayerTracker::yamatocommonparameter], rdi
		pop rdi
		jmp code

	isforceedge:
		mov[PlayerTracker::forceedgemodel], rdi
		jmp code

	isbeowulf:
		mov[PlayerTracker::beowulfmodel], rdi
		jmp code

	isdoppelweapon:
		mov [PlayerTracker::doppelentity], r15
		push r15
		mov r15, [r15+0x1F0]
		mov [PlayerTracker::doppeltransform], r15
		pop r15

		mov [PlayerTracker::doppelweaponmodel], rdi
		jmp code

	code:
		//mov rdi,[rdi+10]
		test r11, r11
		jmp qword ptr [PlayerTracker::vergildata_jmp_ret]
	}
}
    // clang-format on

void PlayerTracker::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> PlayerTracker::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  //player tracker
  auto player_addr = utility::scan(base, "4C 8B C9 41 83 F8 FF 74");
  if (!player_addr) {
    return "Unable to find Player Tracker pattern.";
  }
  //summon tracker
  auto summon_addr = utility::scan(base, "39 6F 64 0F 84 52 01 00 00");
  if (!summon_addr) {
    return "Unable to find Summon Tracker pattern.";
  }
  auto incombat_addr = utility::scan(base, "40 38 B0 CA 0E 00 00 0F 84 04");
  if (!incombat_addr) {
    return "Unable to find In Combat pattern.";
  }
  auto sin_addr = utility::scan(base, "0F 57 F6 F3 0F 5A F0 0F 28 C7 E8 D1 36");
  if (!sin_addr) {
    return "Unable to find Sin pattern.";
  }

  auto cos_addr = utility::scan(base, "F3 0F 59 87 88 0F 00 00");
  if (!cos_addr) {
    return "Unable to find Cos pattern.";
  }
  auto threshhold_addr = utility::scan(base, "72 12 F3 0F 10 05 87 9D AC 02");
  if (!threshhold_addr) {
    return "Unable to find threshhold pattern.";
  }
  auto vergildata_addr = utility::scan(base, "48 8B 7F 10 4D 85 DB");
  if (!vergildata_addr) {
	  return "Unable to find vergildata pattern.";
  }
  if (!install_hook_absolute(player_addr.value(), m_player_hook, &player_detour, &player_jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize player tracker";
  }
  if (!install_hook_absolute(summon_addr.value(), m_summon_hook, &summon_detour, &summon_jmp_ret, 9)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize summon tracker";
  }

  if (!install_hook_absolute(incombat_addr.value(), m_incombat_hook, &incombat_detour, &incombat_jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize In Combat";
  }
  if (!install_hook_absolute(sin_addr.value(), m_sin_hook, &sin_detour, &sin_jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize Sin coordinate";
  }

  if (!install_hook_absolute(cos_addr.value(), m_cos_hook, &cos_detour, &cos_jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize Cos coordinate";
  }
  if (!install_hook_absolute(threshhold_addr.value(), m_threshhold_hook, &threshhold_detour, &threshhold_jmp_ret, 10)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize stick threshhold";
  }
  //if (!install_hook_absolute(vergildata_addr.value(), m_vergildata_hook, &vergildata_detour, &vergildata_jmp_ret, 7)) {
	//  //  return a error string in case something goes wrong
	//  spdlog::error("[{}] failed to initialize", get_name());
	//  return "Failed to initialize vergildata";
  //}
  PlayerTracker::summon_jmp_je     = summon_addr.value() + 0x15B;
  PlayerTracker::threshhold_jmp_jb = threshhold_addr.value() + 0x14;

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
    ImGui::Text("[PlayerTracker] Cos Value: %.4f",PlayerTracker::cosvalue);
    ImGui::Text("[PlayerTracker] Sin Value: %.4f", PlayerTracker::sinvalue);
    ImGui::Text("[PlayerTracker] Stick Pushed: %X", PlayerTracker::redirect);

	//Imgui::Text(PlayerTracker::isgrounded)
}
// will show up in main window, dump ImGui widgets you want here
void PlayerTracker::on_draw_ui() {

}