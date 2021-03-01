#include "LDK.hpp"
#include "mods/PlayerTracker.hpp"
// clang-format off
// only in clang/icl mode on x64, sorry
uintptr_t LDK::enemynumber_jmp_ret{NULL};
uintptr_t LDK::capbypass_jmp_ret1{NULL};
uintptr_t LDK::capbypass_jmp_ret2{NULL};
uintptr_t LDK::capbypass_jmp_jnl{NULL};
uintptr_t LDK::capbypass_jmp_jle{NULL};
uintptr_t LDK::gethpoflasthitobject_jmp_ret{NULL};
uintptr_t LDK::multipledeathoptimize_jmp_ret{NULL};
uintptr_t LDK::multipledeathoptimize_jmp_jle{NULL};
uintptr_t LDK::canlasthitkill_jmp_ret{NULL};
uintptr_t LDK::nopfunction_jmp_ret1{NULL};
uintptr_t LDK::nopfunction_jmp_ret2{NULL};
uintptr_t LDK::nopfunction_1_call{NULL};

uintptr_t LDK::vergildivebomb_jmp_ret{NULL};

uintptr_t LDK::cavforcevalid_jmp_ret{NULL};
uintptr_t LDK::cavforcevalid_jmp_ret2{NULL};
uintptr_t LDK::cavforcevalid_jmp_je{NULL};
uintptr_t LDK::cavforcelightning1_jmp_ret{NULL};
uintptr_t LDK::cavforcelightning2_jmp_ret{NULL};
uintptr_t LDK::cavcoordinatechange_jmp_ret{NULL};

uintptr_t LDK::sswords_restriction_jmp{NULL};
uintptr_t LDK::sswords_restriction_jmp_ret{NULL};
uintptr_t LDK::hitvfxskip_jmp{NULL};
uintptr_t LDK::hitvfxskip_ret{NULL};
uintptr_t LDK::containernum_addr{NULL};
uintptr_t LDK::rax_backup{NULL};
uintptr_t LDK::rcx_backup{NULL};
uintptr_t LDK::nopfunction1_jmp_ret2{NULL};
//uintptr_t enemyspawner_entity = 0x9B38;


uintptr_t LDK::missionmanager{NULL};

bool LDK::cheaton{NULL};
bool LDK::pausespawn_enabled{true};

uint32_t LDK::number{0};
uint32_t LDK::hardlimit{30};
uint32_t LDK::softlimit{0};
uint32_t LDK::limittype{0};
uint32_t lightningcounter = 0;

uint32_t LDK::container_limit_all{76};
uint32_t LDK::container_limit_damage_only{40};
uint32_t LDK::container_num{0};
uint32_t LDK::hardlimit_temp{35};
uint32_t LDK::physicsfix_enable_num{16};

HitVfxState LDK::vfx_state{HitVfxState::DrawAll};

bool LDK::physics_fix_on{true};
bool LDK::hitvfx_fix_on{true};
bool LDK::default_redorbsdrop_enabled{true};

bool is_spawn_paused = false;

std::mutex mtx;

bool canhitkill = true;
bool vergilflipper = false;
float LDK::hpoflasthitobj = 0.0f;
static glm::vec3 coordinate1{-34.0,-6.6,-34.0};
static glm::vec3 coordinate2{ -9.0,7.6,-35.0 };

LDK::RegAddrBackup LDK::death_func_backup;

static void pause_spawn()
{
	mtx.lock();
	if (!is_spawn_paused)
	{
		is_spawn_paused = true;
		std::thread ([&]{
			LDK::hardlimit_temp = LDK::hardlimit;
			if (LDK::number <= 6)
			{
				//LDK::hardlimit = 6;
				is_spawn_paused = false;
				return;
			}
			else
				LDK::hardlimit = 0;
			Sleep(LDK::SPAWN_PAUSE_TIME*1000);
			is_spawn_paused = false;
			LDK::hardlimit = LDK::hardlimit_temp;}).detach();
	}
	mtx.unlock();
}

static naked void enemynumber_detour() {
	__asm {
		mov eax,[rax+0x70]
		mov [rdi+0x00000750],eax
		mov [LDK::number], eax

	hardcheck:
		cmp eax, [LDK::hardlimit]
		jb softcheck

		mov [LDK::limittype], 2
		jmp ret_jmp

	softcheck:
		cmp eax, [LDK::softlimit]
		jb belowsoftcapacity

		mov [LDK::limittype], 1
		jmp ret_jmp

	belowsoftcapacity:
		mov [LDK::limittype], 0
		ret_jmp:
		jmp qword ptr[LDK::enemynumber_jmp_ret]
	}
}
// clang-format on

void LDK::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}
static naked void capbypass_detour1() {
__asm {
	originalcode:
		cmp byte ptr [LDK::cheaton], 1
		je cheatcode
		jmp code

	cheatcode:
		cmp byte ptr [LDK::limittype], 2
		je ret_jnl
		jmp ret_jmp

	code:
		cmp eax, [rcx+0x30]
		jnl ret_jnl
		jmp ret_jmp

	ret_jnl:
		jmp qword ptr [LDK::capbypass_jmp_jnl]

	ret_jmp:
		jmp qword ptr [LDK::capbypass_jmp_ret1]
	}
}
static naked void capbypass_detour2() {
  __asm {
	originalcode:

		cmp byte ptr [LDK::cheaton], 1
		je cheatcode
		jmp code

	cheatcode:
		cmp byte ptr [LDK::limittype], 2
		je ret_jle
		jmp ret_jmp

	code:
		cmp r14d,eax
		jle ret_jle
		jmp ret_jmp

	ret_jle:
		jmp qword ptr [LDK::capbypass_jmp_jle]

	ret_jmp:
		jmp qword ptr [LDK::capbypass_jmp_ret2]
  }
}

static naked void gethpoflasthitobject_detour() {
	__asm {
		movss [LDK::hpoflasthitobj], xmm1
		movss [rdi+0x10], xmm1 // originalcode
		mov [canhitkill], 1
		jmp qword ptr[LDK::gethpoflasthitobject_jmp_ret]
	}
}

static naked void multipledeathoptimize_detour() {
	__asm {
		cmp byte ptr [LDK::physics_fix_on], 0
		je originalcode
		mov r15, qword ptr [LDK::physicsfix_enable_num]
		cmp [LDK::number], r15d
		jb belownum
		mov r15, 0x00000000
		push rsi
		mov rsi, [rsi+0x98]
		cmp [PlayerTracker::playerentity], rsi
		je friendlydeath
		cmp [PlayerTracker::shadowentity], rsi
		je friendlydeath
		cmp [PlayerTracker::griffonentity], rsi
		je friendlydeath
		pop rsi
		cmp byte ptr [LDK::cheaton], 0
		je originalcode

		//cmp dword ptr [LDK::limittype], 0
		//je originalcode

		cmp [LDK::hpoflasthitobj], 0.0
		jle checklasthit
		jmp originalcode

	checklasthit:
		cmp byte ptr [canhitkill], 1
		je physicsdisable
		jmp originalcode
	
	friendlydeath:
		pop rsi

	originalcode:
		mov rcx, [rbx+0x50]
		cmp qword ptr [rcx+0x18], 00
		jmp qword ptr [LDK::multipledeathoptimize_jmp_ret]

	belownum:
        mov r15, 0x00000000
		jmp originalcode

	physicsdisable:
		jmp qword ptr [LDK::multipledeathoptimize_jmp_jle]
	}
}

static naked void canlasthitkill_detour() {
	__asm {
		mov byte ptr [canhitkill], 0
		mov dword ptr [rdi+0x10], 0x3F800000 // originalcode
		jmp qword ptr [LDK::canlasthitkill_jmp_ret]
	}
}

static naked void nopfunction_detour1() {
	__asm {
		cmp byte ptr [LDK::cheaton], 0
		je original
		jmp cheatcode
		/*cmp byte ptr [LDK::pausespawn_enabled], 1
		je cheatcode
		jmp original*/

		original:
		call [LDK::nopfunction_1_call] // call DevilMayCry5.exe+59EE90
		jmp qword ptr[LDK::nopfunction_jmp_ret1]

		cheatcode:
		cmp byte ptr [LDK::default_redorbsdrop_enabled], 0
		je noorbs
		call [LDK::nopfunction_1_call] // call DevilMayCry5.exe+59EE90
		cmp byte ptr [LDK::pausespawn_enabled], 1
		je pausespawn
		jmp qword ptr[LDK::nopfunction_jmp_ret1]

		noorbs:
		cmp byte ptr [LDK::pausespawn_enabled], 1
		je pausespawn
		jmp qword ptr[LDK::nopfunction_jmp_ret1]

		pausespawn:
		mov [LDK::death_func_backup.rax], rax
		mov [LDK::death_func_backup.rcx], rcx
		mov [LDK::death_func_backup.rdx], rdx
		mov [LDK::death_func_backup.r8], r8
		mov [LDK::death_func_backup.r9], r9
		mov [LDK::death_func_backup.r10], r10
		mov [LDK::death_func_backup.r11], r11
		call [pause_spawn]
		mov rax, qword ptr [LDK::death_func_backup.rax]
		mov rcx, qword ptr [LDK::death_func_backup.rcx]
		mov rdx, qword ptr [LDK::death_func_backup.rdx]
		mov r8, qword ptr [LDK::death_func_backup.r8] 
		mov r9, qword ptr [LDK::death_func_backup.r9]
		mov r10, qword ptr [LDK::death_func_backup.r10] 
		mov r11, qword ptr [LDK::death_func_backup.r11]
		jmp qword ptr[LDK::nopfunction_jmp_ret1]

	}
}

static naked void nopfunction_detour2() {
	__asm {
		cmp byte ptr [LDK::cheaton], 1
		je cheatcode

		mov r9, [rax-0x10]
		call qword ptr [r9+0x58]
		jmp qword ptr[LDK::nopfunction_jmp_ret2]

	cheatcode:
		mov r9, [rax-0x10]
		jmp qword ptr[LDK::nopfunction_jmp_ret2]

	}
}


static naked void vergildivebomb_detour() {
	__asm {
		cmp byte ptr [LDK::cheaton], 1
		jne code
		push r12
		mov r12, [LDK::missionmanager]
		mov r12, [r12]
		cmp byte ptr [r12+0x7C], 7
		pop r12
		je cheatcode
		jmp code
	
	code:
		mov byte ptr [rax+0x10], 0x15
		jmp qword ptr [LDK::vergildivebomb_jmp_ret]

	cheatcode:
		xor [vergilflipper], 1
		cmp [vergilflipper], 1
		je jce
		mov byte ptr [rax+0x10], 0x34
		jmp qword ptr [LDK::vergildivebomb_jmp_ret]
	jce:
		mov byte ptr [rax+0x10], 0xB
		jmp qword ptr [LDK::vergildivebomb_jmp_ret]
	}
}


static naked void cavforcevalid_detour() {
	__asm {
		cmp byte ptr [LDK::cheaton], 1
		jne code
		push r12
		mov r12, [LDK::missionmanager]
		mov r12, [r12]
		cmp byte ptr [r12+0x7C], 5
		pop r12
		je cheatcode
		jmp code

	code:
		cmp eax, 0x17
		je je_ret
		xor al, al
		jmp jmp_ret

	cheatcode:
		jmp je_ret

	jmp_ret:
		jmp qword ptr [LDK::cavforcevalid_jmp_ret2]

	je_ret:
		jmp qword ptr [LDK::cavforcevalid_jmp_je]
	}
}

static naked void cavforcelightning1_detour() {
	__asm {
		cmp byte ptr [LDK::cheaton], 1
		jne code
		push r12
		mov r12, [LDK::missionmanager]
		mov r12, [r12]
		cmp byte ptr [r12+0x7C], 5
		pop r12
		je cheatcode
		jmp code

	code:
		mov [r13+0x00000E34], eax
		jmp qword ptr [LDK::cavforcelightning1_jmp_ret]

	cheatcode:
		cmp eax, 1
		je code
		cmp [lightningcounter], 3
		jae code
		mov eax, 2
		jmp code
	}
}

static naked void cavforcelightning2_detour() {
	__asm {
		cmp byte ptr [LDK::cheaton], 1
		jne code
		push r12
		mov r12, [LDK::missionmanager]
		mov r12, [r12]
		cmp byte ptr [r12+0x7C], 5
		pop r12
		je cheatcode
		jmp code

	code:
		mov [r13+0x00000E30], ecx
		jmp qword ptr [LDK::cavforcelightning2_jmp_ret]

	cheatcode:
		cmp eax, 1
		je code
		lock inc [lightningcounter]
		cmp [lightningcounter], 3
		jae resetcounter
		mov ecx, 2
		jmp code

	resetcounter:
		and [lightningcounter], 0
		jmp code
	}
}

static naked void cavcoordinatechange_detour() {
	__asm {
		cmp byte ptr [LDK::cheaton], 1
		jne code
		push r12
		mov r12, [LDK::missionmanager]
		mov r12, [r12]
		cmp byte ptr [r12+0x7C], 5
		pop r12
		je cheatcode
		jmp code

	code:
		mov rcx,[r13+0x10]
		movss [rbp-0x70],xmm0
		jmp qword ptr [LDK::cavcoordinatechange_jmp_ret]

	cheatcode:
		push r8
		mov r8d, [rcx+0x10]
		cmp r8d, [coordinate1.x]
		pop r8

		je floatupdate
		movss xmm0, [coordinate1.x]
		movss xmm1, [coordinate1.z]
		movss xmm2, [coordinate1.y]
		jmp floatreplace

	floatupdate:
		movss xmm0, [coordinate2.x]
		movss xmm1, [coordinate2.z]
		movss xmm2, [coordinate2.y]
		jmp floatreplace
	floatreplace:
		movss [rcx+0x10], xmm0
		movss [rcx+0x14], xmm1
		movss [rcx+0x18], xmm2
		jmp code
	}
}

static naked void sswords_restriction_detour() { // Only 1 embeded sword in enemy body allowed
  __asm {
		cmp byte ptr[LDK::cheaton], 0
		je originalcode
		jmp qword ptr [LDK::sswords_restriction_jmp]

		originalcode:
		cmp eax,[r14+0x18]
		jl back
		jmp qword ptr [LDK::sswords_restriction_jmp]

		back:
		jmp qword ptr [LDK::sswords_restriction_jmp_ret]
  }
}

void set_hitvfxstate() {
  if (LDK::container_limit_all == 0) {
    LDK::vfx_state = HitVfxState::Nothing;
    return;
  }

  /*if (LDK::number > 16) {
    LDK::vfx_state = HitVfxState::Nothing;
    return;
  }*/

  if (LDK::container_num < LDK::container_limit_damage_only)
    LDK::vfx_state = HitVfxState::DrawAll;
  else {
    if (LDK::container_num >= LDK::container_limit_damage_only && LDK::container_num < LDK::container_limit_all)
      LDK::vfx_state = HitVfxState::DamageOnly;
    else
      LDK::vfx_state = HitVfxState::Nothing;
  }
}

static naked void hitvfxskip_detour() {
  __asm {
    // Can skip prev check 'cause it's depended on thing that doesn't change
    // in-game itself, only with REFramework
		cmp byte ptr [LDK::cheaton], 0
		je originalcode
		cmp byte ptr [LDK::hitvfx_fix_on], 0
		je originalcode
		cmp qword ptr [LDK::number], SAFE_NUMBER
		jle originalcode
		jmp containernumcheck

		containernumcheck:
        // container num 
		mov rax, [LDK::containernum_addr]
		mov rax, [rax]
		mov rax, [rax+0x158]
		mov rax, [rax+0x638]
		mov rax, [rax+0x60]
		mov rax, [rax+0x788]
		mov rax, [rax+0xE28]
		mov [LDK::container_num], rax
        //-------------------------------
        // C++ void func will change rcx, that cause a crash later, so just
        // backup it
		mov [LDK::rcx_backup], rcx
		call [set_hitvfxstate]
		mov rcx, qword ptr [LDK::rcx_backup]
		cmp [LDK::vfx_state], 0 // DrawAll
		je drawall
		cmp [LDK::vfx_state], 1 // DamageOnly
		je damageonly
		cmp [LDK::vfx_state], 2 // Nothing
		je nothing

		drawall:
		jmp originalcode

		damageonly:
		mov ah, [rbp]
		cmp ah, CHAR_DAMAGE
		je skip
		jmp originalcode

		nothing:
		mov ah, [rbp]
		cmp ah, CHAR_HITS 
		je skip
		cmp ah, CHAR_DAMAGE 
		je skip
		jmp originalcode

		originalcode:
		test rbp,rbp
		je skip
		jmp qword ptr [LDK::hitvfxskip_ret]

		skip:
		jmp qword ptr [LDK::hitvfxskip_jmp]
  }
}


std::optional<std::string> LDK::on_initialize() {
  init_check_box_info();

  ischecked            = &LDK::cheaton;
  onpage               = gamemode;

  full_name_string     = "Legendary Dark Knights (+)";
  author_string        = "The HitchHiker, Dr. Penguin, DeepDarkKapusta, VPZadov";
  description_string   = "Enables the Legendary Dark Knights Gamemode.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  uintptr_t staticbase = g_framework->get_module().as<uintptr_t>();
  LDK::containernum_addr = staticbase + 0x07E836F8;


  auto enemynumber_addr = utility::scan(base, "8B 40 70 89 87 50 07 00 00");
  if (!enemynumber_addr) {
    return "Unable to find Enemy Number pattern.";
  }
  auto capbypass_addr1 = utility::scan(base, "3B 41 30 7D 12");
  if (!capbypass_addr1) {
    return "Unable to find Cap bypass 1 pattern.";
  }
  auto capbypass_addr2 = utility::scan(base, "44 3B F0 7E 52");
  if (!capbypass_addr2) {
    return "Unable to find Cap bypass 2 pattern.";
  }
  auto gethpoflasthitobject_addr = utility::scan(base, "C9 F3 0F 11 4F 10 48");
  if (!gethpoflasthitobject_addr) {
	  return "Unable to find gethpoflasthitobject pattern.";
  }
  auto multipledeathoptimize_addr = utility::scan(base, "48 8B 4B 50 48 83 79 18 00 75 57 83 BF");
  if (!multipledeathoptimize_addr) {
	  return "Unable to find  multipledeathoptimize pattern.";
  }
  auto canlasthitkill_addr = utility::scan(base, "C7 47 10 00 00 80 3F 32");
  if (!canlasthitkill_addr) {
	  return "Unable to find canlasthitkill pattern.";
  }
  auto nopfunction_addr1 = utility::scan(base, "E8 D7 0C 07 FF");
  if (!nopfunction_addr1) {
	  return "Unable to find nop function 1 pattern.";
  }
  auto nopfunction_addr2 = utility::scan(base, "4C 8B 48 F0 41 FF 51 58 48 8B 5C");
  if (!nopfunction_addr2) {
	  return "Unable to find nop function 2 pattern.";
  }
  auto nopfunction_1_call = utility::scan(base, "48 8B C4 55 56 57 41 54 41 56 41 57 48 8D A8");
  if (!nopfunction_1_call) {
	  return "Unable to find nop function call pattern.";
  }


  auto vergildivebomb_addr = utility::scan(base, "C7 40 10 15 00 00 00 48");
  if (!vergildivebomb_addr) {
	  return "Unable to find vergildivebomb pattern.";
  }


  auto cavforcevalid_addr = utility::scan(base, "74 04 32 C0 EB 02 B0 01 0F B6 D0");
  if (!cavforcevalid_addr) {
	  return "Unable to find cavforcevalid pattern.";
  }
  auto cavforcelightning_addr = utility::scan(base, "41 89 8D 30 0E 00 00");
  if (!cavforcelightning_addr) {
	  return "Unable to find cavforcelightning pattern.";
  }
  auto cavcoordinatechange_addr = utility::scan(base, "49 8B 4D 10 F3 0F 11 45 90");
  if (!cavcoordinatechange_addr) {
	  return "Unable to find cavcoordinatechange pattern.";
  }

  auto sswords_restriction_addr =
      utility::scan(base, "41 3B 46 18 0F 8C A0 01 00 00");
  if (!sswords_restriction_addr) {
    return "Unable to find sswords_restriction_jmp_addr pattern.";
  }
  auto hitvfxskip_addr = utility::scan(base, "75 F3 48 85 ED");
  if (!hitvfxskip_addr) {
    return "Unable to find hitvfxskip_addr pattern.";
  }
  
  LDK::nopfunction_1_call = nopfunction_1_call.value();
  LDK::capbypass_jmp_jnl = capbypass_addr1.value() + 0x17;
  LDK::capbypass_jmp_jle = capbypass_addr2.value() + 0x57;

  LDK::cavforcevalid_jmp_ret2 = cavforcevalid_addr.value()+0x6;
  LDK::cavforcevalid_jmp_je = cavforcevalid_addr.value()+0x8;

  LDK::missionmanager = staticbase+0x7E836F8;
  LDK::multipledeathoptimize_jmp_jle = multipledeathoptimize_addr.value()+0x616; //DevilMayCry5.exe+24E4374
 
  LDK::hitvfxskip_jmp              = hitvfxskip_addr.value() - 0xB;
  LDK::sswords_restriction_jmp_ret = sswords_restriction_addr.value() + 0x1AA;
  LDK::nopfunction1_jmp_ret2       = nopfunction_addr1.value() + 0x83;

  if (!install_hook_absolute(enemynumber_addr.value(), m_enemynumber_hook, &enemynumber_detour, &enemynumber_jmp_ret, 9)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize Enemy Number";
  }
  if (!install_hook_absolute(capbypass_addr1.value(), m_capbypass_hook1, &capbypass_detour1, &capbypass_jmp_ret1, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize Cap bypass 1";
  }
  if (!install_hook_absolute(capbypass_addr2.value(), m_capbypass_hook2, &capbypass_detour2, &capbypass_jmp_ret2, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize Cap bypass 2";
  }

  if (!install_hook_absolute(gethpoflasthitobject_addr.value()+1, m_gethpoflasthitobject_hook, &gethpoflasthitobject_detour,
	  &gethpoflasthitobject_jmp_ret, 5)) {
	//  return a error string in case something goes wrong
	spdlog::error("[{}] failed to initialize", get_name());
	return "Failed to initialize getlasthpofhitobject";
  }
  if (!install_hook_absolute(multipledeathoptimize_addr.value(), m_multipledeathoptimize_hook, &multipledeathoptimize_detour,
	  &multipledeathoptimize_jmp_ret, 9)) {
	//  return a error string in case something goes wrong
	spdlog::error("[{}] failed to initialize", get_name());
	return "Failed to initialize multipledeathoptimize";
  }
  if (!install_hook_absolute(canlasthitkill_addr.value(), m_canlasthitkill_hook, &canlasthitkill_detour,
	&canlasthitkill_jmp_ret, 7)) {
	//  return a error string in case something goes wrong
	spdlog::error("[{}] failed to initialize", get_name());
	return "Failed to initialize canlasthitkill";
  }
  if (!install_hook_absolute(nopfunction_addr1.value(), m_nopfunction_hook1, &nopfunction_detour1, &nopfunction_jmp_ret1, 5)) {
	//  return a error string in case something goes wrong
	spdlog::error("[{}] failed to initialize", get_name());
	return "Failed to initialize nopfunction 1";
  }
  /*if (!install_hook_absolute(nopfunction_addr2.value(), m_nopfunction_hook2, &nopfunction_detour2, &nopfunction_jmp_ret2, 8)) {
	//  return a error string in case something goes wrong
	spdlog::error("[{}] failed to initialize", get_name());
	return "Failed to initialize nopfunction 2";
  }*/


  if (!install_hook_absolute(vergildivebomb_addr.value(), m_vergildivebomb_hook, &vergildivebomb_detour,
	  &vergildivebomb_jmp_ret, 7)) {
	  //  return a error string in case something goes wrong
	  spdlog::error("[{}] failed to initialize", get_name());
	  return "Failed to initialize Vergil Dive bomb";
  }



  if (!install_hook_absolute(cavforcevalid_addr.value(), m_cavforcevalid_hook, &cavforcevalid_detour,
	  &cavforcevalid_jmp_ret, 6)) {
	  //  return a error string in case something goes wrong
	  spdlog::error("[{}] failed to initialize", get_name());
	  return "Failed to initialize Cav force valid";
  }

  if (!install_hook_absolute(cavforcelightning_addr.value() + 0x11, m_cavforcelightning1_hook, &cavforcelightning1_detour,
	  &cavforcelightning1_jmp_ret, 7)) {
	  //  return a error string in case something goes wrong
	  spdlog::error("[{}] failed to initialize", get_name());
	  return "Failed to initialize Cav force lightning 1";
  }

  if (!install_hook_absolute(cavforcelightning_addr.value(), m_cavforcelightning2_hook, &cavforcelightning2_detour,
	  &cavforcelightning2_jmp_ret, 7)) {
	  //  return a error string in case something goes wrong
	  spdlog::error("[{}] failed to initialize", get_name());
	  return "Failed to initialize Cav force lightning 2";
  }

  if (!install_hook_absolute(cavcoordinatechange_addr.value(), m_cavcoordinatechange_hook, &cavcoordinatechange_detour,
	  &cavcoordinatechange_jmp_ret, 9)) {
	  //  return a error string in case something goes wrong
	  spdlog::error("[{}] failed to initialize", get_name());
	  return "Failed to initialize Vergil Dive bomb";
  }

  if (!install_hook_absolute(sswords_restriction_addr.value(), m_ssowrds_restriction_hook, &sswords_restriction_detour, &sswords_restriction_jmp, 10)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize sswords_restriction_addr";
  }
  if (!install_hook_absolute(hitvfxskip_addr.value(), m_hitvfxskip_hook, &hitvfxskip_detour, &hitvfxskip_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize hitvfxskip_addr";
  }

  return Mod::on_initialize();
}

void LDK::set_container_limit_all(uint32_t num) {
  if (num < 0) {
    LDK::container_limit_all = 2;
    return;
  }
  if (num == 0) {
    LDK::container_limit_all = 0;
    return;
  }
  if (num <= LDK::container_limit_damage_only)
    LDK::container_limit_all = LDK::container_limit_damage_only + 1;
  else
    LDK::container_limit_all = num;
}

void LDK::set_container_limit_blood_only(uint32_t num) {
  if (num < 0) {
    LDK::container_limit_damage_only = 1;
    return;
  }
  if (LDK::container_limit_all == 0) {
    LDK::container_limit_damage_only = 0;
    return;
  }
  if (num >= LDK::container_limit_all)
    LDK::container_limit_damage_only = LDK::container_limit_all - 1;
  else
    LDK::container_limit_damage_only = num;
}


// during load
void LDK::on_config_load(const utility::Config &cfg) {}
// during save
void LDK::on_config_save(utility::Config &cfg) {}
// do something every frame
//void LDK::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void LDK::on_draw_debug_ui() {
  ImGui::Text("Enemy Limit type is currently %X", LDK::limittype);
  ImGui::Text("Hard limit is: %X", LDK::hardlimit);
  ImGui::Text("Soft limit is: %X", LDK::softlimit);
  ImGui::Text("HitVfx states: 0 - draw all; 1 - only damage, 2 - nothing");
  ImGui::Text("HitVfx state is: %X", LDK::vfx_state);
  ImGui::Text("Container num: %X", LDK::container_num);
  ImGui::Selectable("Select me?? IDK lol", true);
}

// will show up in main window, dump ImGui widgets you want here
void LDK::on_draw_ui() {
  ImGui::Text("Enemy Hard Limit");
  ImGui::TextWrapped("This controls the maximum number of enemies that can be active simultaneously in encounters.");
  ImGui::SliderInt("##Enemy Hard Limit Slider", (int*)&LDK::hardlimit, 1, 50);
  ImGui::Separator();
  ImGui::Text("Enemy Soft Limit");
  ImGui::TextDisabled("This controls how many enemies can be active simultaneously before optimized death physics are enabled.\n"
	"Past this point, death animations are disabled to prevent additional stress on the game. "
	"This currently can cause issues with enemy spawners not being destroyed.");
  ImGui::TextWrapped("Set any value :D");
  
  ImGui::SliderInt("##Enemy Soft Limit Slider", (int*)&LDK::softlimit, 0, 50);

  ImGui::SliderInt("Draw damage only container num",
                   (int*)&LDK::container_limit_damage_only, 0, 95);
  LDK::set_container_limit_blood_only(LDK::container_limit_damage_only);
  ImGui::SliderInt("Draw nothing container num", (int*)&LDK::container_limit_all,
                   0, 110);
  LDK::set_container_limit_all(LDK::container_limit_all);

  ImGui::Text("Enemy num physics fix disabled");
  ImGui::TextWrapped("This controls how many enemies can be active simultaneously before optimized death physics are enabled.");
  ImGui::SliderInt("##Enemy num physics fix disabled slider", (int*)&LDK::physicsfix_enable_num, 0, 18);

  ImGui::Checkbox("Physics fix enable", (bool*)&LDK::physics_fix_on);
  ImGui::Checkbox("HitVfx fix enable", (bool*)&LDK::hitvfx_fix_on);

  ImGui::TextWrapped("Enable pause for spawn enemies after kill.");
  ImGui::Checkbox("Enable pause spawn", (bool*)&LDK::pausespawn_enabled);

  ImGui::Text("Enable default red orbs drop from enemies on ldk. DO NOT USE THIS with enemylimit > 30 or without hitvfx, physics and spawn pause fixes.");
  ImGui::Checkbox("Default red orb drops on LDK", (bool*)&LDK::default_redorbsdrop_enabled);
}
