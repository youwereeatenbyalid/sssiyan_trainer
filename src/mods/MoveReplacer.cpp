#if 0
#include "MoveReplacer.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t MoveReplacer::jmp_ret{NULL};
uintptr_t MoveReplacer::cheaton{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        je cheatcode
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jmp code
    nerocode:

        jmp qword ptr [MoveReplacer::jmp_ret]        
    dantecode:
		
    vergilcode:

    enemystep:
        cmp [inertiatoggle], 1
        jne moveswaporiginalcode
        cmp [rsp+60],"DevilMayCry5.exe"+24E8348
        je inertiastore
        cmp [rsp+60], "DevilMayCry5.exe"+24E72DE
        je inertiapreserve
        jmp moveswaporiginalcode


        airhike:
        //cmp [rsp+60],"DevilMayCry5.exe"+24E8348
        //je startairhiketimer
        //cmp [rsp+60], "DevilMayCry5.exe"+24E72DE
        //je endairhiketimer
        jmp moveswaporiginalcode


        startairhiketimer:
        mov [airhiketimer], 800
        jmp inertiastore

        endairhiketimer:
        mov [airhiketimer], 0
        jmp inertiapreserve

        guardfly:
        //cmp [rsp+60],"DevilMayCry5.exe"+24E8348
        //je inertiapreserve
        jmp moveswaporiginalcode

        inertiastore:
        push r8
           //move the x inertia into XMM13
           mov r8, [Xinertia]

           test r8, r8
           je inertiastoresafety

           movss xmm13, dword ptr [r8]
           //move the z inertia into XMM14
           mov r8, [Zinertia]
           movss xmm14, dword ptr [r8]

           movss dword ptr [backupxinertia], xmm13
           movss dword ptr [backupzinertia], xmm14

           //square x and z, sum them, and move the square root of the sum into XMM14
           mulss xmm13, xmm13
           mulss xmm14, xmm14
           addss xmm13, xmm14
           sqrtss xmm14, xmm13
           //move the composite inertia into xmm14
           movss dword ptr [backupinertia], xmm14
        inertiastoresafety:
           //clear xmm13 and xmm14
           movss xmm13, xmm15
           movss xmm14, xmm15
        pop r8
        jmp moveswaporiginalcode



        inertiapreserve:
        cmp [threshholdmet], 1
        je inertiaredirect
        push r8
           //move the x and z values into xmm13 and xmm14
           movss xmm13, dword ptr [backupxinertia]
           movss xmm14, dword ptr [backupzinertia]
           //move them into their respective addresses
           mov r8, [Xinertia]
           test r8, r8
           je inertiapreservesafety

           movss dword ptr [r8],xmm13
           mov r8, [Zinertia]
           movss dword ptr [r8],xmm14

        inertiapreservesafety:
           //clear the xmm13 and xmm14 registers
           movss xmm13, xmm15
           movss xmm14, xmm15
        pop r8
        jmp moveswaporiginalcode


        inertiaredirect:
        push r8
           //move the composite inertia into xmm14
           movss xmm14, dword ptr [backupinertia]
           //move the sin value into xmm13
           movss xmm13, dword ptr [directionsinvalue]
           //multiply the two together to get the new X inertia
           mulss xmm13, xmm14
           //move the new value into the X inertia address
           mov r8, [Xinertia]
           test r8, r8
           je inertiaredirectsafety

           movss dword ptr [r8],xmm13
           //move the cos value into xmm13
           movss xmm13, dword ptr [directioncosvalue]
           //multiply the two together to get the new X inertia
           mulss xmm13, xmm14
           //move the new value into the X inertia address
           mov r8, [Zinertia]
           movss dword ptr [r8],xmm13

        inertiaredirectsafety:
           //clear the xmm13 and xmm14 registers
           movss xmm13, xmm15
           movss xmm14, xmm15
        pop r8
        jmp moveswaporiginalcode

        inertiaairhike:
        //time to get funky kids

        cmp [threshholdmet], 1
        jne moveswaporiginalcode

        push r8
           //move the x inertia into XMM13
           mov r8, [Xinertia]
           test r8, r8
           je inertiaairhikesafety

           movss xmm13, dword ptr [r8]
           //move the z inertia into XMM14
           mov r8, [Zinertia]
           movss xmm14, dword ptr [r8]

           movss dword ptr [backupxinertia], xmm13
           movss dword ptr [backupzinertia], xmm14

           //square x and z, sum them, and move the square root of the sum into XMM14
           mulss xmm13, xmm13
           mulss xmm14, xmm14
           addss xmm13, xmm14
           sqrtss xmm14, xmm13

           //move the sin value into xmm13
           movss xmm13, dword ptr [directionsinvalue]
           //multiply the two together to get the new X inertia
           mulss xmm13, xmm14
           //move the new value into the X inertia address
           mov r8, [Xinertia]
           movss dword ptr [r8],xmm13
           //move the cos value into xmm13
           movss xmm13, dword ptr [directioncosvalue]
           //multiply the two together to get the new X inertia
           mulss xmm13, xmm14
           //move the new value into the X inertia address
           mov r8, [Zinertia]
           movss dword ptr [r8],xmm13

           inertiaairhikesafety:
           //clear the xmm13 and xmm14 registers
           movss xmm13, xmm15
           movss xmm14, xmm15
        pop r8
        jmp moveswaporiginalcode
    moveswaporiginalcode:
        mov ebx,[r14]
        mov rcx,r13
  

        jmp qword ptr [MoveReplacer::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> MoveReplacer::on_initialize() {
  // auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  //ischecked = false;
  //onpage    = commonpage;
  //full_name_string     = "MoveReplacer Full Name";
  //author_string        = "Author";
  //description_string   = "This is the description of MoveReplacer.";
  //MoveReplacer::cheaton = (uintptr_t)&ischecked;

  //auto addr = utility::scan(base, "F3 0F 10 8F 14 1A 00 00 BA");
  //if (!addr) {
  //  return "Unable to find MoveReplacer pattern.";
  //}
  //if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
  //  return a error string in case something goes wrong
  //  spdlog::error("[{}] failed to initialize", get_name());
  //  return "Failed to initialize MoveReplacer";
  //}
  return Mod::on_initialize();
}

// during load
//void MoveReplacer::on_config_load(const utility::Config &cfg) {}
// during save
//void MoveReplacer::on_config_save(utility::Config &cfg) {}
// do something every frame
//void MoveReplacer::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void MoveReplacer::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
//void MoveReplacer::on_draw_ui() {}
#endif
