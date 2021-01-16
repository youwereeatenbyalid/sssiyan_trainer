
#include "PlayerTracker.hpp"
#include "NeroSwapWiresnatch.hpp"
uintptr_t NeroSwapWiresnatch::jmp_ret1{NULL};
uintptr_t NeroSwapWiresnatch::jmp_ret2{NULL};
uintptr_t NeroSwapWiresnatch::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour1() {
	__asm {
        cmp byte ptr [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        push rax
        mov rax, [NeroSwapWiresnatch::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
        cmp rbp, 3
        je code
        mov byte ptr [rax+10h], 43
		jmp qword ptr [NeroSwapWiresnatch::jmp_ret1]

    code:
        mov byte ptr [rax+10h], 0000001Ah
		jmp qword ptr [NeroSwapWiresnatch::jmp_ret1]
	}
}

static naked void detour2() {
	__asm {
        cmp byte ptr [PlayerTracker::playerid], 0 //change this to the char number obviously
        push rax
        mov rax, [NeroSwapWiresnatch::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code

    cheatcode:
        cmp rbp, 3
        je code
        mov byte ptr [rax+10h], 43
		jmp qword ptr [NeroSwapWiresnatch::jmp_ret2]

    code:
        mov byte ptr [rax+10h], 00000019h
        jmp qword ptr [NeroSwapWiresnatch::jmp_ret2]
	}
}

// clang-format on

std::optional<std::string> NeroSwapWiresnatch::on_initialize() {
  ischecked          = false;
  onpage             = neropage;
  full_name_string   = "Angel and Devil Snatch";
  author_string      = "SSSiyan";
  description_string = "Replaces Wiresnatch with rawhide snatch to the enemy and adds knockback.";
  NeroSwapWiresnatch::cheaton = (uintptr_t)&ischecked;
  auto base  = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr1 = utility::scan(base, "C7 40 10 1A 00 00 00 E9");
  if (!addr1) {
    return "Unable to find NeroSwapWiresnatch pattern.";
  }
  auto addr2 = utility::scan(base, "C7 40 10 19 00 00 00 48 8B D0");
  if (!addr2) {
    return "Unable to find NeroSwapWiresnatch pattern.";
  }

  if (!install_hook_absolute(addr1.value(), m_function_hook1, &detour1, &jmp_ret1, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroSwapWiresnatch1";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroSwapWiresnatch2";
  }
  return Mod::on_initialize();
}

void NeroSwapWiresnatch::on_draw_ui() {}
