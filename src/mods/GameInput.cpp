
#include "GameInput.hpp"
#include "mods/PlayerTracker.hpp"
#include "mods/BreakerSwitcher.hpp"
#include "mods/HoldToMash.hpp"
uintptr_t GameInput::validcontrol_jmp_ret{NULL};
uintptr_t GameInput::hold_jmp_ret{NULL};
uintptr_t GameInput::clearhold_jmp_ret{NULL};
uintptr_t GameInput::press_jmp_ret{NULL};
uintptr_t GameInput::clearpress_jmp_ret{NULL};
uintptr_t GameInput::release_jmp_ret{NULL};
uintptr_t GameInput::releasewhenheld_jmp_ret{NULL};
uintptr_t GameInput::clearrelease_jmp_ret{NULL};
bool GameInput::cheaton{NULL};

uintptr_t GameInput::validcontrols{NULL};
uintptr_t GameInput::holdframes[20]{};
uintptr_t GameInput::pressframes[20]{};
uintptr_t GameInput::releaseframes[20]{};
    // clang-format off
// only in clang/icl mode on x64, sorry
static naked void validcontrol_detour() {
	__asm {
        mov rcx, rbx
        test rdx, rdx
        je validcontrol_jmp
        mov [GameInput::validcontrols],rdx

    validcontrol_jmp:
        jmp qword ptr [GameInput::validcontrol_jmp_ret]        

	}
}
static naked void hold_detour() {
	__asm {
    validation:
        push r8
        push r9
        push r10
        push r11
        push r12
        push r13
        push r14
        push r15
        cmp rdx, [GameInput::validcontrols]
        jne code
        cmp byte ptr [PlayerTracker::ingameplay], 1
        jne code
        mov r15, 0
        test [rdx+0x00000098],r8d
        jna charactercompare
        or GameInput::holdframes[0], rsi
        mov r15, 1
        jmp charactercompare

    code:
        test [rdx+0x00000098],r8d
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        pop r10
        pop r9
        pop r8
        jmp qword ptr [GameInput::hold_jmp_ret]

    charactercompare:
        cmp [PlayerTracker::playerid], 1 //go to dante compare
        je dantecompare
        jmp code

    dantecompare:
        cmp byte ptr [HoldToMash::cheaton], 1 //check if hold to mash
        jne code
        cmp r15, 1 //check if button held
        jne code
        cmp rsi, 2//check if gun button
        jne code
        mov r8, [PlayerTracker::danteentity]
        cmp dword ptr [r8+0x1A54], 8 //check if active action is ebony/ivory
        jne code
        or [rbx+0x48], rsi //add gun value to press?
        jmp code
    wrapup:
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        pop r10
        pop r9
        pop r8		
        jmp qword ptr [GameInput::hold_jmp_ret]
	}
}
static naked void clearhold_detour() {
	__asm {
	validation:
        cmp r14, [GameInput::validcontrols]
        jne clearholdoriginalcode
        cmp byte ptr [PlayerTracker::ingameplay], 1
        jne clearholdoriginalcode
        push r8
        push rcx
        push rax
        lea rcx, GameInput::holdframes[0]
        mov rax, 19
    iterate:
        dec rax
        mov r8, [rcx+rax*8]
        mov [rcx+rax*8+8], r8
        cmp rax, 0
        ja iterate
    
        mov GameInput::holdframes[0*8],0
        pop rax
        pop rcx
        pop r8

    clearholdoriginalcode:
        mov [r14+0x3C], r12d
        mov rax, [rsi+0x50]
        jmp qword ptr [GameInput::clearhold_jmp_ret]
	}
}

static naked void press_detour() {
	__asm {
        push r8
        push r9
        push r10
        push r11
        push r12
        push r13
        push r14
        push r15
        cmp rdx, [GameInput::validcontrols]
        jne code
        cmp byte ptr [PlayerTracker::ingameplay], 1
        jne code
        mov r15, 0
        test [rbx+0x00000090],ebp
        jna code
        or GameInput::pressframes[0], rsi
        mov r15, 1
        jmp charactercompare

    code:
        test [rbx+0x00000090],ebp
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        pop r10
        pop r9
        pop r8
        jmp qword ptr [GameInput::press_jmp_ret]        

    charactercompare:
        mov r9, GameInput::pressframes[0*8]
        or r9, GameInput::pressframes[1*8]
        or r9, GameInput::pressframes[2*8]
        or r9, GameInput::pressframes[3*8]
        or r9, GameInput::pressframes[4 * 8]
        or r9, GameInput::pressframes[5 * 8]
        or r9, GameInput::pressframes[6 * 8]
        //or r9, GameInput::pressframes[7 * 8]
        //or r9, GameInput::pressframes[8 * 8]
        mov r10, GameInput::holdframes[0*8]
        or r10, GameInput::holdframes[1*8]
        or r10, GameInput::holdframes[2 * 8]
        push r8
        lea r8, BreakerSwitcher::breakerpress_detour
        call r8
        pop r8

    pressexit:
        cmp r15,0
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        pop r10
        pop r9
        pop r8		
        jmp qword ptr [GameInput::press_jmp_ret]
	}
}
static naked void clearpress_detour() {
	__asm {
    validation:
        cmp r14, [GameInput::validcontrols]
        jne clearpressoriginalcode

        cmp byte ptr [PlayerTracker::ingameplay], 1
        jne clearpressoriginalcode
        push r8
        push rcx
        push rax
        lea rcx, GameInput::pressframes[0]
        mov rax, 19

    iterate:
        dec rax
        mov r8, [rcx+rax*8]
        mov [rcx+rax*8+8], r8
        cmp rax, 0
        ja iterate
    
        mov GameInput::pressframes[0*8], 0
        pop rax
        pop rcx
        pop r8

    clearpressoriginalcode:
        mov [r14+0x48],r12d
        mov rax, [rsi+0x50]
        jmp qword ptr [GameInput::clearpress_jmp_ret]
	}
}

static naked void release_detour() {
	__asm {
	validation:
        push r8
        push r9
        push r10
        push r11
        push r12
        push r13
        push r14
        push r15
        cmp rdx, [GameInput::validcontrols]
        jne code
        cmp byte ptr [PlayerTracker::ingameplay], 1
        jne code
        mov r15, 0
        test [rbx+0x00000094], ebp
        jna releasefinish
        or GameInput::releaseframes[0], rsi
        mov r15, 1
        jmp charactercompare

    code:
        test [rbx+0x00000094], ebp
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        pop r10
        pop r9
        pop r8
        jmp qword ptr [GameInput::release_jmp_ret]       

    charactercompare:
        cmp byte ptr [PlayerTracker::playerid], 0 //check for nero
        jne code
        cmp rsi, 0x1000 //check for breaker
        je nerobreakervalid 
        jmp code

    nerobreakervalid:
        mov r12, [PlayerTracker::playerentity] //get nero
        test r12, r12
        je code
        mov r12, [r12+0x17E0] //get nero track
        test r12, r12
        je code
        cmp byte ptr [r12+0x57], 1 //can we start command?
        jne code
        mov byte ptr [BreakerSwitcher::force_primary_input], 1 //force breaker
        jmp code


    releasefinish:
        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        pop r10
        pop r9
        pop r8		
        jmp qword ptr [GameInput::release_jmp_ret]
	}
}
static naked void releasewhenheld_detour() {
	__asm {
        test [rbx+0x3C], esi
        jmp qword ptr [GameInput::releasewhenheld_jmp_ret]
	}
}
static naked void clearrelease_detour() {
	__asm {
    validation:
        cmp r14, [GameInput::validcontrols]
        jne clearpressoriginalcode
        cmp byte ptr [PlayerTracker::ingameplay], 1
        jne clearpressoriginalcode
        push r8
        push rcx
        push rax
        lea rcx, GameInput::releaseframes[0]
        mov rax, 19

    iterate:
        dec rax
        mov r8, [rcx+rax*8]
        mov [rcx+rax*8+8], r8
        cmp rax, 0
        ja iterate
    
        mov GameInput::releaseframes[0*8],0
        pop rax
        pop rcx
        pop r8

    clearpressoriginalcode:   
        mov [r14+0x44], r12d
        mov rax, [rsi+0x50]
        jmp qword ptr [GameInput::clearrelease_jmp_ret]
	}
}

// clang-format on

void GameInput::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> GameInput::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &GameInput::cheaton;
  m_on_page    = -1;
  m_full_name_string     = "Game Input Hook";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Hooks the virtual inputs.";

  set_up_hotkey();

  auto validcontrol_addr    = patterns->find_addr(base, "A1 01 00 00 48 8B 97 C0 00 00 00 48 8B CB 48 85 D2");
  auto hold_addr            = patterns->find_addr(base, "44 85 82 98 00 00 00");
  auto clearhold_addr       = patterns->find_addr(base, "45 89 66 3C 48 8B 46 50");

  auto press_addr      = patterns->find_addr(base, "85 AB 90 00 00 00");
  auto clearpress_addr = patterns->find_addr(base, "45 89 66 48 48 8B 46 50");

  auto release_addr         = patterns->find_addr(base, "85 AB 94 00 00 00");
  auto releasewhenheld_addr = patterns->find_addr(base, "85 73 3C 75 03");
  auto clearrelease_addr    = patterns->find_addr(base, "45 89 66 44 48 8B 46 50");
  if (!validcontrol_addr) 
    return "Unable to find ValidControl pattern.";
   if (!hold_addr)
    return "Unable to find Hold pattern.";
  if (!clearhold_addr)
    return "Unable to find Clear Hold pattern.";

  if (!press_addr)
    return "Unable to find Press pattern.";
  if (!clearpress_addr)
    return "Unable to find Clear Press pattern.";

  if (!release_addr)
    return "Unable to find release pattern.";
  if (!releasewhenheld_addr)
    return "Unable to find release when held pattern.";
  if (!clearrelease_addr)
    return "Unable to find Clear release pattern.";

  if (!install_hook_absolute(validcontrol_addr.value() + 0xB,
                             m_validcontrol_hook, &validcontrol_detour,
                             &validcontrol_jmp_ret, 6)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize valid control";
  }
  if (!install_hook_absolute(hold_addr.value(),
                             m_hold_hook, &hold_detour,
                             &hold_jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize hold";
  }
  if (!install_hook_absolute(clearhold_addr.value(),
                             m_clearhold_hook, &clearhold_detour,
                             &clearhold_jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize clear hold";
  }
  if (!install_hook_absolute(press_addr.value(), m_press_hook,
                             &press_detour, &press_jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize press";
  }
  if (!install_hook_absolute(clearpress_addr.value(), m_clearpress_hook,
                             &clearpress_detour, &clearpress_jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize clear press";
  }

    if (!install_hook_absolute(release_addr.value(), m_release_hook, &release_detour,
                             &release_jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize release";
  }
    if (!install_hook_absolute(releasewhenheld_addr.value(), m_releasewhenheld_hook,
                               &releasewhenheld_detour, &releasewhenheld_jmp_ret, 5)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize release when held";
    }

  if (!install_hook_absolute(clearrelease_addr.value(), m_clearrelease_hook,
                             &clearrelease_detour, &clearrelease_jmp_ret, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize clear release";
  }
  return Mod::on_initialize();
}

// during load
// void GameInput::on_config_load(const utility::Config &cfg) {}
// during save
// void GameInput::on_config_save(utility::Config &cfg) {}
// do something every frame
// void GameInput::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void GameInput::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void GameInput::on_draw_ui() {}