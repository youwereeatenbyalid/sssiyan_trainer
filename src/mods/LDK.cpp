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
uintptr_t LDK::nopfunction1_jmp_ret2{NULL};
//uintptr_t enemyspawner_entity = 0x9B38;
uintptr_t LDK::waitTimeJmpRet{NULL};
uintptr_t LDK::nohitlns_ret{NULL};
uintptr_t LDK::nohitlns_ret_je{NULL};


uintptr_t LDK::missionmanager{NULL};
uintptr_t LDK::enemygentype{NULL};
bool LDK::cheaton{NULL};
bool LDK::pausespawn_enabled{false};
bool LDK::emDtVfxSkipOn = false;
bool LDK::showOldFixes = false;

uint32_t LDK::number{0};
uint32_t LDK::hardlimit{30};
uint32_t LDK::softlimit{25};
uint32_t LDK::limittype{0};
uint32_t lightningcounter = 0;

uint32_t LDK::container_limit_all{72};
uint32_t LDK::container_limit_damage_only{50};
uint32_t LDK::container_num{0};
uint32_t LDK::hardlimit_temp = LDK::hardlimit;
uint32_t LDK::enemydeath_count{0};

HitVfxState LDK::vfx_state{HitVfxState::DrawAll};

bool LDK::hitvfx_fix_on{false};
bool LDK::waitTimeEnabled{false};
bool LDK::nohitlines_enabled{false};

bool is_spawn_paused = false;
bool swap_hitvfx_settings = false;

bool canhitkill = true;
bool vergilflipper = false;
float LDK::hpoflasthitobj = 0.0f;
static glm::vec3 coordinate1{-34.0,-6.6,-34.0};
static glm::vec3 coordinate2{ -9.0,7.6,-35.0 };

//LDK::RegAddrBackup LDK::death_func_backup;
//LDK::RegAddrBackup LDK::redorbdrop_backup;
LDK::RegAddrBackup LDK::hitvfx_backup;

void pause_spawn()
{
	if (!is_spawn_paused)
	{
		if (LDK::number <= 8) {
			return;
		}
		LDK::hardlimit_temp = LDK::hardlimit;
		LDK::hardlimit = 0;
		is_spawn_paused = true;
		std::thread ([&]{
		Sleep(LDK::SPAWN_PAUSE_TIME*1000);
		is_spawn_paused = false;
		LDK::hardlimit = LDK::hardlimit_temp;}).detach();
	}
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

static naked void nopfunction_detour1() {
  __asm {
		cmp byte ptr [LDK::cheaton], 0
		je originalcode
		cmp byte ptr [LDK::pausespawn_enabled], 1
		je pausespawn

		originalcode:
		call [LDK::nopfunction_1_call] // call DevilMayCry5.exe+59EE90
		jmp qword ptr[LDK::nopfunction_jmp_ret1]

		pausespawn:
		push rax
		push rbx
		push rcx
		push rdx
		push r8
		push r9
		push r10
		push r11
		sub rsp, 32
		call qword ptr [pause_spawn]
		add rsp, 32
		pop r11
		pop r10
		pop r9
		pop r8
		pop rdx
		pop rcx
		pop rbx
		pop rax
		jmp originalcode
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
	    mov [LDK::hitvfx_backup.rax], rax
		cmp byte ptr [LDK::emDtVfxSkipOn], 1
		je skipdtvfx
		settings:
		cmp [LDK::container_limit_all], 0
		je nothing
		cmp qword ptr [LDK::number], SAFE_NUMBER
		jbe originalcode
		jmp containernumcheck

		containernumcheck:
        // container num 
		//mov [LDK::hitvfx_backup.rax], rax
		mov rax, [LDK::containernum_addr]
		mov rax, [rax]
		mov rax, [rax+0x158]
		mov rax, [rax+0x638]
		mov rax, [rax+0x60]
		mov rax, [rax+0x788]
		mov rax, [rax+0xE28]
		mov [LDK::container_num], rax
		mov rax, [LDK::hitvfx_backup.rax]
        //-------------------------------//
		/*mov [LDK::hitvfx_backup.rcx], rcx
		mov [LDK::hitvfx_backup.rdx], rdx*/
		push rax
		push rdx
		push rcx
		sub rsp, 32
		call [set_hitvfxstate]
		add rsp, 32
		pop rcx
		pop rdx
		pop rax
		/*mov rax, [LDK::hitvfx_backup.rax]
		mov rcx, [LDK::hitvfx_backup.rcx]
		mov rdx, [LDK::hitvfx_backup.rdx]*/
		test rbp,rbp
		je skip
		cmp [LDK::vfx_state], 0 // DrawAll
		je drawall
		cmp [LDK::vfx_state], 1 // DamageOnly
		je damageonly
		cmp [LDK::vfx_state], 2 // Nothing
		je nothing

		drawall:
		jmp originalcode

		damageonly:
		mov ax, word ptr [rbp]
		cmp byte ptr [swap_hitvfx_settings], 1
		je swapped_settings
		cmp ax, CHAR_DAMAGE
		je restorereg
		jmp originalcode

		swapped_settings:
		cmp ax, CHAR_HITS 
		je restorereg
		jmp originalcode

		nothing:
		mov ax, word ptr [rbp]
		cmp ax, CHAR_HITS 
		je restorereg
		cmp ax, CHAR_DAMAGE 
		je restorereg
		jmp originalcode

		skipdtvfx:
		mov ax, word ptr [rbp]
		cmp ax, EM_DT
		je restorereg
		jmp settings

		originalcode:
		test rbp,rbp
		je skip
		jmp qword ptr [LDK::hitvfxskip_ret]

		restorereg:
		mov rax, [LDK::hitvfx_backup.rax]

		skip:
		jmp qword ptr [LDK::hitvfxskip_jmp]
  }
}

static naked void wait_time_spawn_detour() {
	__asm {
		cmp byte ptr [LDK::waitTimeEnabled], 1
		je waittime

		originalcode:
		movsxd rax, dword ptr [r8+0x24]
		mov [rsp+0x50],rbx
		jmp qword ptr [LDK::waitTimeJmpRet]

		waittime:
		movss xmm0, [LDK::waitTime]
		movss [r8+0x68], xmm0
		jmp originalcode
  }
}

static naked void hitvfx_nohitlines_detour() {
	__asm {
		cmp byte ptr [LDK::cheaton], 0
		je originalcode
		cmp byte ptr [LDK::nohitlines_enabled], 0
		je originalcode
		jmp qword ptr [LDK::nohitlns_ret]

		originalcode:
		cmp byte ptr [rax+0x51], 0
		je ret_je
		jmp qword ptr [LDK::nohitlns_ret]

		ret_je:
		jmp qword ptr [LDK::nohitlns_ret_je]
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
  auto canlasthitkill_addr = utility::scan(base, "C7 47 10 00 00 80 3F 32");
  if (!canlasthitkill_addr) {
	  return "Unable to find canlasthitkill pattern.";
  }
  auto nopfunction_addr1 = utility::scan(base, "E8 D7 0C 07 FF");
  if (!nopfunction_addr1) {
	  return "Unable to find nop function 1 pattern.";
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
  auto hitvfxskip_addr = utility::scan(base, "75 F3 48 85 ED");
  if (!hitvfxskip_addr) {
    return "Unable to find hitvfxskip_addr pattern.";
  }

  auto waittime_addr = utility::scan(base, "49 63 40 24 48 89 5C 24 50"); //DevilMayCry5.exe+11C5E9C 
  if (!waittime_addr) {
    return "Unable to find waittime_addr pattern.";
  }

  auto dontdrawhitlines_addr = utility::scan(base, "76 80 78 51 00 74 04"); //DevilMayCry5.exe+916E9D
  if (!dontdrawhitlines_addr) {
    return "Unable to find LDK.dontdrawhitlines_addr pattern.";
  }

  
  LDK::nopfunction_1_call = nopfunction_1_call.value();
  LDK::capbypass_jmp_jnl = capbypass_addr1.value() + 0x17;
  LDK::capbypass_jmp_jle = capbypass_addr2.value() + 0x57;

  LDK::cavforcevalid_jmp_ret2 = cavforcevalid_addr.value()+0x6;
  LDK::cavforcevalid_jmp_je = cavforcevalid_addr.value()+0x8;

  LDK::missionmanager = staticbase+0x7E836F8;
  LDK::enemygentype = staticbase+0x58E15A0;
 
  LDK::hitvfxskip_jmp              = hitvfxskip_addr.value() - 0xB;
  LDK::nopfunction1_jmp_ret2       = nopfunction_addr1.value() + 0x83;
  nohitlns_ret_je                  = dontdrawhitlines_addr.value() + 0xA + 0x1;

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
  if (!install_hook_absolute(nopfunction_addr1.value(), m_nopfunction_hook1, &nopfunction_detour1, &nopfunction_jmp_ret1, 5)) {
	//  return a error string in case something goes wrong
	spdlog::error("[{}] failed to initialize", get_name());
	return "Failed to initialize nopfunction 1";
  }

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

  if (!install_hook_absolute(hitvfxskip_addr.value(), m_hitvfxskip_hook, &hitvfxskip_detour, &hitvfxskip_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize hitvfxskip_addr";
  }

  if (!install_hook_absolute(waittime_addr.value(), m_wait_spawn_time_hook, &wait_time_spawn_detour, &waitTimeJmpRet, 0x9)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize hitvfxskip_addr";
  }

  if (!install_hook_absolute(dontdrawhitlines_addr.value()+0x1, m_hitvfx_dontdraw_hitlines_hook, &hitvfx_nohitlines_detour, &nohitlns_ret, 0x6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize LDK.dontdrawhitlines_addr";
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
void LDK::on_config_load(const utility::Config &cfg) {
  hitvfx_fix_on  = cfg.get<bool>("hitvfx_fix_on").value_or(true);
  hardlimit = cfg.get<uint32_t>("hardlimit").value_or(30);
  container_limit_damage_only = cfg.get<uint32_t>("container_limit_damage_only").value_or(50);
  container_limit_all = cfg.get<uint32_t>("container_limit_all").value_or(72);
  swap_hitvfx_settings = cfg.get<bool>("swap_hitvfx_settings").value_or(false);
  waitTimeEnabled      = cfg.get<bool>("LDK_waitTimeEnabled").value_or(false);
  nohitlines_enabled = cfg.get<bool>("LDK.nohitlines_enabled").value_or(false);
  emDtVfxSkipOn      = cfg.get<bool>("LDK.emDtVfxSkipOn").value_or(false);
  pausespawn_enabled = cfg.get<bool>("LDK.pausespawn_enabled").value_or(false);
}
// during save
void LDK::on_config_save(utility::Config &cfg) {
  cfg.set<bool>("hitvfx_fix_on", hitvfx_fix_on);
  cfg.set<uint32_t>("hardlimit", hardlimit);
  cfg.set<uint32_t>("container_limit_damage_only", container_limit_damage_only);
  cfg.set<uint32_t>("container_limit_all", container_limit_all);
  cfg.set<bool>("swap_hitvfx_settings", swap_hitvfx_settings);
  cfg.set<bool>("LDK_waitTimeEnabled", waitTimeEnabled);
  cfg.set<bool>("LDK.nohitlines_enabled", nohitlines_enabled);
  cfg.set<bool>("LDK.emDtVfxSkipOn", emDtVfxSkipOn);
  cfg.set<bool>("LDK.pausespawn_enabled", pausespawn_enabled);
}
// do something every frame
//void LDK::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void LDK::on_draw_debug_ui() {
  ImGui::Text("Enemy Limit type is currently %X", LDK::limittype);
  ImGui::Text("Hard limit is: %X", LDK::hardlimit);
  ImGui::Text("Soft limit is: %X", LDK::softlimit);
  ImGui::Text("HitVfx states: 0 - draw all; 1 - only damage, 2 - nothing");
  ImGui::Text("HitVfx state is: %d", LDK::vfx_state);
  ImGui::Text("Container num: %d", LDK::container_num);
  ImGui::Selectable("Select me?? IDK lol", true);
}

// will show up in main window, dump ImGui widgets you want here
void LDK::on_draw_ui() {
  ImGui::Text("Enemy Hard Limit");
  ImGui::TextWrapped("This controls the maximum number of enemies that can be active simultaneously in encounters.");
  ImGui::SliderInt("##Enemy Hard Limit Slider", (int*)&LDK::hardlimit, 1, 50);
  ImGui::Separator();

  if (ImGui::CollapsingHeader("Performance Optimization"))
  {
  ImGui::Checkbox("Disable Hit VFX", (bool*)&LDK::hitvfx_fix_on);
  ImGui::TextWrapped("This will disable some visual effects on objects when they take damage to increase overall performance.");
  //ImGui::TextWrapped("ContainerNum value before last vfx func call: %d", container_num);
  
  ImGui::Spacing();
  if(LDK::hitvfx_fix_on){
	  ImGui::TextWrapped("When the number of effects queued exceeds this value, the game will only draw white hit effects.");
	  ImGui::SliderInt("##ContainerNum limit to draw only hit effects", (int*)&LDK::container_limit_damage_only, 0, 180);
	  ImGui::Checkbox("Disable limit", (bool*)&swap_hitvfx_settings);
	  LDK::set_container_limit_blood_only(LDK::container_limit_damage_only);

	  ImGui::Spacing();
  
	  ImGui::TextWrapped("When the number of effects queued exceeds this value, the game will not draw any hit effects.");
	  ImGui::SliderInt("##ContainerNum limit to draw nothing", (int*)&LDK::container_limit_all,  0, 310);
	  LDK::set_container_limit_all(LDK::container_limit_all);

	  ImGui::Spacing();

	  //ImGui::TextWrapped("When this on, the game will draw hits (blood, hit lines, etc.) instead of draw only damage (white flash effects) when container num >= \"ContainerNum limit to draw only damage\". "
	  //	  "This may improve overall visual quality, but also increase hits effects count, that will decrease overall performance.");
  
	  ImGui::TextWrapped("Disable enemy dt visual effects to slightly increase performance.");
	  ImGui::Checkbox("Disable enemy DT VFX", &emDtVfxSkipOn);

	  ImGui::Spacing();
  }


  ImGui::Separator();

  ImGui::TextWrapped("Don't draw hit lines on enemies. Increases performance.");
  ImGui::Checkbox("Don't draw hitlines", &nohitlines_enabled);

  ImGui::Separator();

  ImGui::Checkbox("Enable pause spawn", (bool*)&LDK::pausespawn_enabled);
  ImGui::TextWrapped("Briefly delay enemies spawning after an enemy is killed. Desyncs in Co-op, use \"Co-op pause spawn\" instead.");
  //ImGui::TextWrapped("P.S. this shit actually sets hardlimit to 0 for a few seconds after killing an enemy when current enemy num on a wave > 8. If you skip cutscene after which "
  //  "enemies should spawn when hardlimit = 0, game may be softlocked :(.");
  if (pausespawn_enabled)
    waitTimeEnabled = false;

  ImGui::Separator();

  ImGui::Checkbox("Enable pause spawn for coop", &waitTimeEnabled);
  //ImGui::TextWrapped("Enable pause for spawning enemies before each enemy spawns, include preloaded enemies (like cainas on start of mission 1). Enemies will spawn by groops of a few "
      //"pieces after \"Wait time\" property. That should decrease a load to PC while playing LDK + coop.");
  //ImGui::TextWrapped("Delay before spawn");
  //ImGui::SliderFloat("##DelaySlider", &waitTime, 0.5f, 5.0f, "%.1f");
  if (waitTimeEnabled)
    pausespawn_enabled = false;

  //ImGui::Separator();
  }
}
