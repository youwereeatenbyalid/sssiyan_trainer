#include "BreakerSwitcher.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t BreakerSwitcher::breakersize_jmp_ret{NULL};
uintptr_t BreakerSwitcher::nextbreaker_jmp_ret{NULL};
uintptr_t BreakerSwitcher::breakerui_jmp_ret{NULL};
uintptr_t BreakerSwitcher::cheaton{NULL};

uint32_t BreakerSwitcher::breakers[8]{};
uint32_t BreakerSwitcher::nextbreaker{3};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void breakersize_detour() {
	__asm {
	validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        push rax
        mov rax, [BreakerSwitcher::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code
    code:
        mov ecx,[rsi+0x000017CC]
        jmp qword ptr [BreakerSwitcher::breakersize_jmp_ret]        
    cheatcode:
		mov byte ptr [rsi+0x000017CC], 1
        mov ecx,[rsi+0x000017CC]
        jmp qword ptr [BreakerSwitcher::breakersize_jmp_ret]
	}
}
static naked void nextbreaker_detour() {
	__asm {
	validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        push rax
        mov rax, [BreakerSwitcher::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code
    code:
        movsxd  r12,dword ptr [rax+0x20]
        test rdx,rdx
        jmp qword ptr [BreakerSwitcher::nextbreaker_jmp_ret]        
    cheatcode:
		movsxd r12, dword ptr [BreakerSwitcher::nextbreaker]
        mov [rax+0x20], r12
        test rdx,rdx
        jmp qword ptr [BreakerSwitcher::nextbreaker_jmp_ret]
	}
}
static naked void breakerui_detour() {
	__asm {
	validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        push rax
        mov rax, [BreakerSwitcher::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code
    code:
        mov [r11+rcx],eax
        mov rdx,[rsi+0x00000108]
        jmp qword ptr [BreakerSwitcher::breakerui_jmp_ret]        
    cheatcode:
        mov eax, [BreakerSwitcher::nextbreaker]
		mov [r11+rcx],eax
        mov rdx,[rsi+0x00000108]
        jmp qword ptr [BreakerSwitcher::breakerui_jmp_ret]
	}
}
static naked void disablebreakaway_detour() {
	__asm {
	validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        push rax
        mov rax, [BreakerSwitcher::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
        jmp code
    code:
        mov [r11+rcx],eax
        mov rdx,[rsi+0x00000108]
        jmp qword ptr [BreakerSwitcher::breakerui_jmp_ret]        
    cheatcode:
        mov eax, [BreakerSwitcher::nextbreaker]
		mov [r11+rcx],eax
        mov rdx,[rsi+0x00000108]
        jmp qword ptr [BreakerSwitcher::breakerui_jmp_ret]
	}
}
naked void BreakerSwitcher::breakerpress_detour() {
	__asm {
	validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        push rax
        mov rax, [BreakerSwitcher::cheaton]
        //Why the fuck can this be called before it's properly initialized I hate this
        test rax, rax
        je validationexit
        cmp byte ptr [rax], 1
        pop rax
        je cheatcode
    

    code:
        ret    
    validationexit:
        pop rax
        ret
    cheatcode:
		cmp esi, 0x100
        je upbreaker
        cmp esi, 0x200
        je downbreaker
        cmp esi, 0x400
        je leftbreaker
        cmp esi, 0x800
        je rightbreaker
        jmp breakerpressexit

        upbreaker:
        mov r8d, BreakerSwitcher::breakers[0*4]
        test r9, 0x400
        ja upleftbreaker
        test r9, 0x800
        ja uprightbreaker
        jmp breakerwrite

        leftbreaker:
        mov r8d, BreakerSwitcher::breakers[2*4]
        test r9, 0x100
        ja upleftbreaker

        test r9, 0x200
        ja downleftbreaker
        jmp breakerwrite

        rightbreaker:
        mov r8d, BreakerSwitcher::breakers[3*4]
        test r9, 0x100
        ja uprightbreaker

        test r9, 0x200
        ja downrightbreaker
        jmp breakerwrite


        downbreaker:
        mov r8d, BreakerSwitcher::breakers[1*4]
        test r9, 0x400
        ja downleftbreaker

        test r9, 0x800
        ja downrightbreaker
        jmp breakerwrite
        
        upleftbreaker:
        mov r8d, BreakerSwitcher::breakers[4*4]
        jmp breakerwrite

        uprightbreaker:
        mov r8d, BreakerSwitcher::breakers[5*4]
        jmp breakerwrite

        downrightbreaker:
        mov r8d, BreakerSwitcher::breakers[6*4]
        jmp breakerwrite

        downleftbreaker:
        mov r8d, BreakerSwitcher::breakers[7*4]
        jmp breakerwrite
        
        breakerwrite:
        //cmp r8b, 0xD
        //je exit
        mov [BreakerSwitcher::nextbreaker], r8d
        jmp breakerpressexit
            
        breakerpressexit:
        mov r15, 1
        ret
	}
}

// clang-format on

std::optional<std::string> BreakerSwitcher::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  ischecked = false;
  onpage    = neropage;
  full_name_string     = "Breaker Switcher";
  author_string        = "The Hitchhiker (original version by Nino)";
  description_string   = "Press a button on the d-pad to switch breakers.";
  BreakerSwitcher::cheaton = (uintptr_t)&ischecked;

  auto breakersize_addr = utility::scan(base, "8B 8E CC 17 00 00 48 85");
  auto nextbreaker_addr = utility::scan(base, "4C 63 60 20 48 85 D2");
  auto breakerui_addr = utility::scan(base, "41 89 04 0B 48 8B 96 08 01 00 00");
  if (!breakersize_addr) {
    return "Unable to find breaker size pattern.";
  }
  if (!nextbreaker_addr) {
    return "Unable to find next breaker pattern.";
  }
  if (!breakerui_addr) {
    return "Unable to find breaker ui pattern.";
  }
  if (!install_hook_absolute(breakersize_addr.value(), m_breakersize_hook,
                             &breakersize_detour, &breakersize_jmp_ret, 6)) {
    //return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize breakersize";
  }
  if (!install_hook_absolute(nextbreaker_addr.value(), m_nextbreaker_hook,
                             &nextbreaker_detour, &nextbreaker_jmp_ret, 7)) {
    //return a error string in case something goes wrong 
      spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize nextbreaker";
  }
  if (!install_hook_absolute(breakerui_addr.value(), m_breakerui_hook,
                             &breakerui_detour, &breakerui_jmp_ret, 11)) {
    //return a error string in case something goes wrong 
      spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize breakerui";
  }
  return Mod::on_initialize();
}

// during load
void BreakerSwitcher::on_config_load(const utility::Config& cfg) {
  for (int i = 0; i < 8; i++) {
    BreakerSwitcher::breakers[i] =
        cfg.get<int>("breaker_slot_" + i).value_or(0);
  }
}
// during save
void BreakerSwitcher::on_config_save(utility::Config &cfg) {
  for (int i = 0; i < 8; i++) {
    cfg.set<int>("breaker_slot_" + i, BreakerSwitcher::breakers[i]);
  }
}
// do something every frame
void BreakerSwitcher::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void BreakerSwitcher::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void BreakerSwitcher::on_draw_ui() {
//ImGui::InputInt("breaker up", (int*)&BreakerSwitcher::breakers[0]);
//ImGui::InputInt("breaker down", (int*)&BreakerSwitcher::breakers[1]);
//ImGui::InputInt("breaker left", (int*)&BreakerSwitcher::breakers[2]);
//ImGui::InputInt("breaker right", (int*)&BreakerSwitcher::breakers[3]);
  auto breakerboxstring =
      "Overture\0Ragtime\0Helter Skelter\0Gerbera\0Punchline\0Buster "
      "Arm\0Rawhide\0Tomboy\0Mega Buster\0Gerbera GP01\0Pasta Breaker\0Sweet "
      "Surrender\0Monkey Business\0";
      ImGui::Combo("breaker up", (int*)&BreakerSwitcher::breakers[0],
             breakerboxstring);
ImGui::Combo("breaker down", (int*)&BreakerSwitcher::breakers[1],
             breakerboxstring);
ImGui::Combo("breaker left", (int*)&BreakerSwitcher::breakers[2],
             breakerboxstring);
ImGui::Combo("breaker right", (int*)&BreakerSwitcher::breakers[3],
             breakerboxstring);
ImGui::Combo("breaker up left", (int*)&BreakerSwitcher::breakers[4],
             breakerboxstring);
ImGui::Combo("breaker up right", (int*)&BreakerSwitcher::breakers[5],
             breakerboxstring);
ImGui::Combo("breaker down left", (int*)&BreakerSwitcher::breakers[7],
             breakerboxstring);
ImGui::Combo("breaker down right", (int*)&BreakerSwitcher::breakers[6],
             breakerboxstring);


}