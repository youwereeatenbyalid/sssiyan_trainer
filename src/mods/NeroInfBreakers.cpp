
#include "NeroInfBreakers.hpp"
#include "PlayerTracker.hpp"
#include "BreakerSwitcher.hpp"
uintptr_t NeroInfBreakers::jmp_ret1{NULL};
uintptr_t NeroInfBreakers::jmp_ret2{NULL};

int breaker01;
int breaker02;
int breaker03;
int breaker04;
int breaker05;
int breaker06;
int breaker07;
int breaker08;

bool NeroInfBreakers::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour1() {
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [BreakerSwitcher::infinite_breakers], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp r9d, 01
        je lab04
        cmp r9d, [breaker06]
        je lab01
        jmp code

    lab04:
        mov eax, [r10+rdx]
        mov [breaker05], eax
        mov eax, [rdx+1Ch]
        dec eax
        mov [breaker06], eax
        jmp code

    lab01:
        mov eax, [r10+rdx+04]
        push rax
        mov eax, [breaker05]
        mov [r10+rdx+04], eax
        pop rax
        jmp retcode

    code: // lab02
        mov eax,[r10+rdx+04]

    retcode: // lab03
        jmp qword ptr [NeroInfBreakers::jmp_ret1]
	}
}

static naked void detour2() {
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [NeroInfBreakers::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        mov rax, [rsi+00001790h]
        mov eax, [rax+1Ch]
        mov [rsi+000017CCh], eax
		jmp qword ptr [NeroInfBreakers::jmp_ret2]

    code:
        mov eax, [rsi+000017CCh]
        jmp qword ptr [NeroInfBreakers::jmp_ret2]
	}
}

// clang-format on

void NeroInfBreakers::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NeroInfBreakers::on_initialize() {
  init_check_box_info();

  ischecked          = &NeroInfBreakers::cheaton;
  //onpage             = Mod::page_enum::;

  full_name_string   = "Infinite Devil Breakers";
  author_string      = "DeepDarkKapustka";
  description_string = "When using 8 Devil Breakers, this will function like the Void option.\n"
                       "When using less than 8, this will bug and give you Overtures.";

  auto base  = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr1 = utility::scan(base, "FF 41 8B 44 12 04");
  if (!addr1) {
    return "Unable to find NeroInfBreakers1 pattern.";
  }
  auto addr2 = utility::scan(base, "8B 86 CC 17 00 00 85");
  if (!addr2) {
    return "Unable to find NeroInfBreakers2 pattern.";
  }

  if (!install_hook_absolute(addr1.value()+1, m_function_hook1, &detour1, &jmp_ret1, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroInfBreakers1";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroInfBreakers2";
  }
  return Mod::on_initialize();
}

void NeroInfBreakers::on_draw_ui() {}
