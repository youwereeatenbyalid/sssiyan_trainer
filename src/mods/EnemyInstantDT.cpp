
#include "EnemyInstantDT.hpp"
#include "PlayerTracker.hpp"

uintptr_t EnemyInstantDT::jmp_ret{NULL};
bool EnemyInstantDT::cheaton{NULL};

    // clang-format off
    // only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        //cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        //jne code
        cmp byte ptr [EnemyInstantDT::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		jmp qword ptr [EnemyInstantDT::jmp_ret]

    code:
        movss xmm1, [rax+10h]
        jmp qword ptr [EnemyInstantDT::jmp_ret]
	}
}

// clang-format on

void EnemyInstantDT::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> EnemyInstantDT::on_initialize() {
  init_check_box_info();

  ischecked               = &EnemyInstantDT::cheaton;
  onpage                  = gamemode;

  full_name_string        = "Instant Enemy DT";
  author_string           = "SSSiyan";
  description_string      = "Forces enemies to enter DT immediately when on DMD";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "F3 0F 10 48 10 0F 5A C0 0F 5A C9 66 41 0F 2E C0 7A 16 75 14 ? ? C0 48 ? ? 41 8D 50 39 E8 ? ? ? ? 0F 57 C0 EB 08 F2 0F 5E C8 66 0F 5A C1 48 8B 43 50 48 39");
  if (!addr) {
    return "Unable to find EnemyInstantDT pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize EnemyInstantDT";
  }

  return Mod::on_initialize();
}

void EnemyInstantDT::on_draw_ui() {}

/*
{
// ORIGINAL CODE - INJECTION POINT: "DevilMayCry5.exe"+1532349

"DevilMayCry5.exe"+153232B: 48 8B 4B 50              -  mov rcx,[rbx+50]
"DevilMayCry5.exe"+153232F: 48 39 69 18              -  cmp [rcx+18],rbp
"DevilMayCry5.exe"+1532333: 74 05                    -  je DevilMayCry5.exe+153233A
"DevilMayCry5.exe"+1532335: 0F 57 C0                 -  xorps xmm0,xmm0
"DevilMayCry5.exe"+1532338: EB 3F                    -  jmp DevilMayCry5.exe+1532379
"DevilMayCry5.exe"+153233A: 48 85 C0                 -  test rax,rax
"DevilMayCry5.exe"+153233D: 75 05                    -  jne DevilMayCry5.exe+1532344
"DevilMayCry5.exe"+153233F: 0F 57 C0                 -  xorps xmm0,xmm0
"DevilMayCry5.exe"+1532342: EB 35                    -  jmp DevilMayCry5.exe+1532379
"DevilMayCry5.exe"+1532344: F3 0F 10 40 14           -  movss xmm0,[rax+14]
// ---------- INJECTING HERE ----------
"DevilMayCry5.exe"+1532349: F3 0F 10 48 10           -  movss xmm1,[rax+10]
// ---------- DONE INJECTING  ----------
"DevilMayCry5.exe"+153234E: 0F 5A C0                 -  cvtps2pd xmm0,xmm0
"DevilMayCry5.exe"+1532351: 0F 5A C9                 -  cvtps2pd xmm1,xmm1
"DevilMayCry5.exe"+1532354: 66 41 0F 2E C0           -  ucomisd xmm0,xmm8
"DevilMayCry5.exe"+1532359: 7A 16                    -  jp DevilMayCry5.exe+1532371
"DevilMayCry5.exe"+153235B: 75 14                    -  jne DevilMayCry5.exe+1532371
"DevilMayCry5.exe"+153235D: 45 33 C0                 -  xor r8d,r8d
"DevilMayCry5.exe"+1532360: 48 8B CB                 -  mov rcx,rbx
"DevilMayCry5.exe"+1532363: 41 8D 50 39              -  lea edx,[r8+39]
"DevilMayCry5.exe"+1532367: E8 14 F4 00 01           -  call DevilMayCry5.exe+2541780
"DevilMayCry5.exe"+153236C: 0F 57 C0                 -  xorps xmm0,xmm0
}
*/