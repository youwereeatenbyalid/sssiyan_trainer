#include "BreakerSwitcher.hpp"
#include "mods/PlayerTracker.hpp"
uint32_t bsinputs[] = { Mod::sword,Mod::gun,Mod::jump,Mod::tauntinput,Mod::lockon,Mod::changetarget,Mod::dpad,Mod::deviltrigger,Mod::dpadup,Mod::dpaddown,Mod::dpadleft,Mod::dpadright,Mod::style,Mod::righttrigger,Mod::lefttrigger,Mod::resetcamera,Mod::SDT };
int breakaway_index = {0};
uintptr_t BreakerSwitcher::breakersize_jmp_ret{NULL};
uintptr_t BreakerSwitcher::nextbreaker_jmp_ret{NULL};
uintptr_t BreakerSwitcher::breakercontrol_jmp_ret{ NULL };
uintptr_t BreakerSwitcher::jmp_bringer_ret{ NULL };
uintptr_t BreakerSwitcher::jne_bringer_ret{NULL};
uintptr_t BreakerSwitcher::jmp_uireturn{ NULL };
uintptr_t BreakerSwitcher::jne_uireturn{NULL};
uintptr_t BreakerSwitcher::call_nero_creategauntlet{NULL};

uintptr_t BreakerSwitcher::breaker_busy{ 0 };
uintptr_t BreakerSwitcher::buffered_breaker_input{ 0 };
uintptr_t BreakerSwitcher::primary_breaker{ 0 };
uintptr_t BreakerSwitcher::do_ui_update{ 0 };
uintptr_t BreakerSwitcher::secondary_breaker{ 0 };
uintptr_t BreakerSwitcher::switcher_mode{ 1 };
uint32_t BreakerSwitcher::breakaway_type{0};
uintptr_t BreakerSwitcher::breakaway_button{0x1000};
bool BreakerSwitcher::infinite_breakers{false};
bool BreakerSwitcher::use_secondary{false};


bool BreakerSwitcher::cheaton{NULL};

uint32_t BreakerSwitcher::breakers[8]{};
uint32_t BreakerSwitcher::nextbreaker{3};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void breakersize_detour() {
	__asm {
	validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [BreakerSwitcher::infinite_breakers],1
        je cheatcode
        jmp code
    code:
        mov ecx,[rsi+0x000017CC]
        jmp qword ptr [BreakerSwitcher::breakersize_jmp_ret]        
    cheatcode:
        cmp [BreakerSwitcher::switcher_mode], 2
        je eightbreakers
        cmp [BreakerSwitcher::switcher_mode], 1
        jne code 
        cmp [BreakerSwitcher::use_secondary], 1
        je twobreakers
        mov byte ptr [rsi + 0x000017CC], 1
        mov ecx, [rsi + 0x000017CC]
        jmp qword ptr [BreakerSwitcher::breakersize_jmp_ret]

    twobreakers:
        mov byte ptr [rsi + 0x000017CC], 2
        mov ecx,[rsi+0x000017CC]
        jmp qword ptr [BreakerSwitcher::breakersize_jmp_ret]

    eightbreakers:
        mov byte ptr [rsi+0x000017CC], 8
        mov ecx, [rsi+0x000017CC]
        jmp qword ptr [BreakerSwitcher::breakersize_jmp_ret]
	}
}
static naked void nextbreaker_detour() {
	__asm {
	validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [BreakerSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        movsxd  r12,dword ptr [rax+0x20]
        test rdx,rdx
        jmp qword ptr [BreakerSwitcher::nextbreaker_jmp_ret]        
    cheatcode:
        movsxd r12, dword ptr [BreakerSwitcher::nextbreaker]
        test rdx,rdx
        jmp qword ptr [BreakerSwitcher::nextbreaker_jmp_ret]
	}
}


static naked void bringercontrol_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [BreakerSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        test cl, cl
        jne jne_bringer
        cmp byte ptr [rdx + 0x18C2], 0x0
        jmp qword ptr [BreakerSwitcher::jmp_bringer_ret]

    jne_bringer:
        jmp qword ptr [BreakerSwitcher::jne_bringer_ret]

    cheatcode:
    bringercontrol:

        push rdx
        push rdi
        push r8

        mov rdi, [rdx + 0x1800] //get gauntlet
        mov r8, [rdx + 0x1790] //get gauntlet list
    CheckSecondaryConditions:
        //if no input, we can just leave
        test cl, cl        
        je NormalInputExit

        cmp [BreakerSwitcher::use_secondary], 0
        je NormalInputExit

        //if snatching, we can also leave
        cmp byte ptr [rdx+0xED0], 0x0
        jne NormalInputExit

        //sanity check to make sure there's a gauntlet
        test rdi, rdi
        je NormalInputExit

        //ditto here
        test r8, r8
        je NormalInputExit

        //check magazine size greater than or equal to 0x2
        cmp dword ptr [rdx + 0x17CC], 0x2
        jb NormalInputExit


    CheckBreakerConditions:
        //get breaker id
        mov rdi, [rdi + 0x318]

        //is breaker being swapped?
        cmp byte ptr [BreakerSwitcher::breaker_busy], 0x1
        je BreakerInputExit

        mov r8, [r8 + 0x24] //get breaker in slot 0x2

        cmp [BreakerSwitcher::nextbreaker], r8d
        je BreakerInputExit

        mov [BreakerSwitcher::nextbreaker], r8d
        jmp BreakerInputExit


    BreakerInputExit:
        mov [BreakerSwitcher::buffered_breaker_input], 0x4
        mov cl, 0x0

    NormalInputExit:
        pop r8
        pop rdi
        pop rdx
        test cl, cl
        jmp code

    }
}

static naked void breakercontrol_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [BreakerSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        lea eax, [r9-0x01]
        mov rdi, rdx
        jmp qword ptr [BreakerSwitcher::breakercontrol_jmp_ret]
    cheatcode:
        push rdx
        push rdi
        push r8
        mov rdx, [PlayerTracker::playerentity]//get nero
        test rdx, rdx
        je NoInputExit
        mov rdi, [rdx+0x1800] //get gauntlet
        mov r8, [rdx+0x1790] //get gauntlet list
        jmp BreakerUpdateStart

    BreakerUpdateStart:
        //sanity check to m0xake sure there's a gauntlet
        test rdi, rdi
        je NoInputExit

        //ditto here
        test r8, r8
        je NoInputExit

        mov r8, [r8 + 0x20] //get first ID in gauntlet list
        mov rdi, [rdi + 0x318] // get current gauntlet ID
        cmp [BreakerSwitcher::nextbreaker], rdi //is next breaker = current breaker?
        jne AttemptCreateBreaker //if not, need to attempt new breaker creation.
        mov [BreakerSwitcher::breaker_busy], 0x0 //no breaker creation occuring, so not busy.

    MoveBufferDecriment:

        cmp [BreakerSwitcher::buffered_breaker_input], 0x1
        jb CheckNaturalInput
        dec [BreakerSwitcher::buffered_breaker_input]
        jmp CheckNaturalInput

    AttemptCreateBreaker:
        cmp [BreakerSwitcher::breaker_busy], 0x0 //if being created, don't call it again
        jne CheckNaturalInput
        mov [BreakerSwitcher::breaker_busy], 0x1
        push rax
        push rcx
        push rdx
        push rdi
        push r8
        push r9
        push r10
        push r11
        push r12
        push r14
        push r15

        //push r15 //replacement for alignment
        //set booleans to 0x0
        xor r8, r8
        xor r9, r9

        mov r15, [BreakerSwitcher::call_nero_creategauntlet]
        call r15

        //pop r15 //alignment shit
        pop r15
        pop r14
        pop r12
        pop r11
        pop r10
        pop r9
        pop r8
        pop rdi
        pop rdx
        pop rcx
        pop rax
        jmp CheckNaturalInput



    CheckNaturalInput:
    //check for natural breaker inputs
        cmp r9, 0x2
        je PrimaryBreakerComparison
        cmp r9, 0x3
        je PrimaryBreakerComparison


    CheckBufferedInput:
    //check for buffered breaker input
        cmp [BreakerSwitcher::buffered_breaker_input], 0x1
        jne NoInputExit

        //if the breaker is busy, maintain the buffer.
        cmp [BreakerSwitcher::breaker_busy], 0x1
        je NoInputExit

        //otherwise unload move from buffer
        mov [BreakerSwitcher::buffered_breaker_input], 0x0
        jmp InputExit


    PrimaryBreakerComparison:
        cmp [BreakerSwitcher::breaker_busy], 0x1 //if busy, move input into buffer
        je SetBufferedInput
        cmp [BreakerSwitcher::nextbreaker], r8d //compare first slot to BreakerSwitcher::next_breaker
        je InputExit //if it's 0xalre0xady prim0xary, we c0xan just input 0xa move 0xand 0xbe done.

        //otherwise we need to update BreakerSwitcher::next_breaker
        mov [BreakerSwitcher::nextbreaker], r8d
        //then, we buffer the input
    SetBufferedInput:
        mov [BreakerSwitcher::buffered_breaker_input], 0x4
        //and prevent the move from triggering.

    NoInputExit:
        mov r9, 0x0
        pop r8
        pop rdi
        pop rdx
        jmp code

    InputExit:
        mov r9, 0x2
    NormalExit:
        pop r8
        pop rdi
        pop rdx
        jmp code
    }
}


static naked void breakerui_detour() {
    __asm {
    validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr[BreakerSwitcher::cheaton], 1
        je cheatcode
        jmp code
    code:
        test al, al
        jne jneretcode
        jmp qword ptr [BreakerSwitcher::jmp_uireturn]

    cheatcode:
        test al, al
        //is natural update?
        jne CopyBreakers
        //is ui update?
        cmp [BreakerSwitcher::do_ui_update], 0x1
        je CopyBreakers

        jmp NoUpdateExit

    CopyBreakers:
        cmp [BreakerSwitcher::switcher_mode], 0x1
        jne ResetUIFlag
        push r8
        push r9
        push rdi
        //get array
        //mov rdi, [rdi + 0x108]
        //mov rdi, [rdi + 0x1790]
        //xor r8, r8
        //xor r9, r9
        //get primary and second breakers
        //mov r8d, [BreakerSwitcher::primary_breaker]
        //mov r9d, [BreakerSwitcher::secondary_breaker]

        //add breakers back to first & second slots.
        //mov [rdi + 0x20], r8d
        //mov [rdi + 0x24], r9d

        pop rdi
        pop r9
        pop r8

    ResetUIFlag:
        mov [BreakerSwitcher::do_ui_update], 0x0
    UpdateExit:
        mov al, 0x01
        test al, al
        jmp uicode

    NoUpdateExit:
        mov al, 0x00
        test al, al
        jmp uicode

    uicode:
        jne jneretcode
        jmp qword ptr [BreakerSwitcher::jmp_uireturn]

    jneretcode:
        jmp qword ptr [BreakerSwitcher::jne_uireturn]
    }
}
static naked void disablebreakaway_detour() {
	__asm {
	validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [BreakerSwitcher::cheaton]
        je cheatcode
        jmp code
    code:
        mov [r11+rcx],eax
        mov rdx,[rsi+0x00000108]
        jmp qword ptr [BreakerSwitcher::jmp_uireturn]
    cheatcode:
        mov eax, [BreakerSwitcher::nextbreaker]
		mov [r11+rcx],eax
        mov rdx,[rsi+0x00000108]
        jmp qword ptr [BreakerSwitcher::jmp_uireturn]
	}
}
naked void BreakerSwitcher::breakerpress_detour() {
	__asm {
	validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        //push rax
        //mov rax, [BreakerSwitcher::cheaton]
        //Why the fuck can this be called before it's properly initialized I hate this
        //test rax, rax
        //je validationexit
        cmp byte ptr [BreakerSwitcher::cheaton], 1
        je cheatcode
    

    code:
        ret    
    //validationexit:
    //    pop rax
    //    ret
    cheatcode:
        cmp rsi, 0x100
        je DPadButtonPress
        cmp rsi, 0x200
        je DPadButtonPress
        cmp rsi, 0x400
        je DPadButtonPress
        cmp rsi, 0x800
        je DPadButtonPress
        cmp rsi, 0x10000
        je CheckBreakawayType
        jmp breakerpressexit
    DPadButtonPress:
        //check if in switcher mode
        cmp byte ptr [BreakerSwitcher::switcher_mode], 0x1
        jne breakerpressexit
        //check if we're doing secondary breakers
        cmp [BreakerSwitcher::use_secondary], 0x1
        jne UpdateFirstBreaker
        //check for exceed
        shr r10, 0xE
        and r10, 0x1
        //if exceed held go to second breaker
        jne UpdateSecondBreaker
        jmp UpdateFirstBreaker


       //load primary or secondary breaker addresses into r11

    UpdateFirstBreaker:
        lea r11, [BreakerSwitcher::primary_breaker]
        jmp MoveDPadIntoRegister

    UpdateSecondBreaker:
        lea r11, [BreakerSwitcher::secondary_breaker]
        jmp MoveDPadIntoRegister


    MoveDPadIntoRegister:
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
        mov [r11], r8d
        mov [BreakerSwitcher::do_ui_update], 0x1

        mov r12, [PlayerTracker::playerentity]
        test r12, r12
        je breakerpressexit
        mov r12, [r12+0x1790]
        test r12, r12
        je breakerpressexit
        mov r8d, [BreakerSwitcher::primary_breaker]
        mov r9d, [BreakerSwitcher::secondary_breaker]
        //add breakers back to first & second slots.
        mov [r12 + 0x20], r8d
        mov [r12 + 0x24], r9d

        jmp breakerpressexit



    CheckBreakawayType:
        mov r12, [PlayerTracker::playerentity]
        test r12, r12
        je breakerpressexit
        //# of breakers
        xor r13,r13
        mov r13d, [r12+0x17CC]

        mov r12, [r12 +0x1790] //get array
        test r12, r12
        je breakerpressexit
        //if never breakaway, go to cycle breakers
        cmp dword ptr [BreakerSwitcher::breakaway_type], 0x0
        je CycleBreakers

        //check for breakaway button
        test r10, [BreakerSwitcher::breakaway_button]
        //if held, skip cycle and trigger breakaway
        ja breakerpressexit

        //if always breakaway, skip cycle.
        cmp [BreakerSwitcher::breakaway_type], 2
        je breakerpressexit



    CycleBreakers:
        cmp [BreakerSwitcher::switcher_mode], 2 //check for cycle mode
        //if not cycle mode, exit with no breakaway
        jne nobreakawayexit


        //check if at least 2 elements
        cmp r13, 2
        jb CycleBreakerEnd


        xor r9, r9
        xor r8, r8

        //get element 0
        xor r9, r9
        mov r9d, [r12+0x20]



    CycleBreakerLoop:
        //decrement iterator
        dec r13
        //access element
        mov r8d, [r12+r13*0x4+0x20]
        //push element
        push r8
        //load element address
        lea r8, [r12+r13*0x4+0x20]
        //override element with previous element
        mov [r8], r9d
        //pop element
        pop r8
        //move element into write position
        mov r9d, r8d
        //check if at 0th element
        cmp r13, 0
        ja CycleBreakerLoop
        //if so, exit and trigger UI update.

    CycleBreakerEnd:
        mov [BreakerSwitcher::do_ui_update], 1
        jmp nobreakawayexit


            
    breakerpressexit:
        mov r15, 1
        ret


    nobreakawayexit:
        mov r15, 0
        ret
	}
}

// clang-format on

void BreakerSwitcher::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> BreakerSwitcher::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &BreakerSwitcher::cheaton;
  m_on_page    = breaker;

  m_full_name_string     = "Breaker Switcher (+)";
  m_author_string        = "The Hitchhiker (original version by Nino)";
  m_description_string   = "Make sure your d-pad is bound to breakaway, then "
                         "press a button on the d-pad to switch breakers.";


  set_up_hotkey();
  auto breakersize_addr = utility::scan(base, "8B 8E CC 17 00 00 48 85");
  auto nextbreaker_addr = utility::scan(base, "4C 63 60 20 48 85 D2");
  auto NeroUIOverride_addr = utility::scan(base, "0F 85 DC 02 00 00 48 8B 87 08");
  auto breakerinputcontrol_addr = utility::scan(base, "41 8D 41 FF 48 8B FA");
  auto call_nero_creategauntlet_addr = utility::scan(base, "C3 CC CC 40 53 56 41 55");
  auto bringerinputcontroller_addr = utility::scan(base, "75 4E 80 BA C2 18 00 00 00");


  BreakerSwitcher::call_nero_creategauntlet = call_nero_creategauntlet_addr.value()+0x3;
  BreakerSwitcher::jne_uireturn = NeroUIOverride_addr.value() + 0x2E2;
  BreakerSwitcher::jne_bringer_ret = bringerinputcontroller_addr.value()+0x50;
  if (!breakersize_addr) {
    return "Unable to find breaker itemSize pattern.";
  }
  if (!nextbreaker_addr) {
    return "Unable to find next breaker pattern.";
  }
  if (!NeroUIOverride_addr) {
      return "Unable to find NeroUIOverride pattern.";
  }
  if (!breakerinputcontrol_addr) {
      return "Unable to find breakerinputcontrol pattern.";
  }
  if (!bringerinputcontroller_addr) {
      return "Unable to find bringerinputcontroller pattern.";
  }


  if (!install_hook_absolute(breakersize_addr.value(), m_breakersize_hook, &breakersize_detour, &breakersize_jmp_ret, 6)) {
    //return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize breakersize";
  }
  if (!install_hook_absolute(nextbreaker_addr.value(), m_nextbreaker_hook, &nextbreaker_detour, &nextbreaker_jmp_ret, 7)) {
    //return a error string in case something goes wrong 
      spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize nextbreaker";
  }
  if (!install_hook_absolute(NeroUIOverride_addr.value(), m_NeroUIOverride_hook, &breakerui_detour, &jmp_uireturn, 6)) {
      //return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize NeroUIOverride";
  }
  if (!install_hook_absolute(breakerinputcontrol_addr.value(), m_breakerinputcontrol_hook, &breakercontrol_detour, &breakercontrol_jmp_ret, 7)) {
      //return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize breakerinputcontrol";
  }
  if (!install_hook_absolute(bringerinputcontroller_addr.value(), m_bringerinputcontroller_hook, &bringercontrol_detour, &jmp_bringer_ret, 9)) {
      //return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize bringerinputcontroller";
  }
  return Mod::on_initialize();
}

// during load
void BreakerSwitcher::on_config_load(const utility::Config& cfg) {
  for (int i = 0; i < 8; i++) {
    BreakerSwitcher::breakers[i] =
        cfg.get<int>("breaker_slot_" + std::to_string(i)).value_or(0);
  }
  BreakerSwitcher::switcher_mode = cfg.get<int>("switcher_mode").value_or(0);
  BreakerSwitcher::breakaway_type = cfg.get<int>("breakaway_type").value_or(0);
  breakaway_index = cfg.get<int>("breakaway_button").value_or(0);
  BreakerSwitcher::infinite_breakers = cfg.get<bool>("infinite_breakers").value_or(false);
  BreakerSwitcher::use_secondary = cfg.get<bool>("use_secondary").value_or(false);
}
// during save
void BreakerSwitcher::on_config_save(utility::Config &cfg) {
  for (int i = 0; i < 8; i++) {
    cfg.set<int>("breaker_slot_" + std::to_string(i), BreakerSwitcher::breakers[i]);
  }
  cfg.set<int>("switcher_mode", BreakerSwitcher::switcher_mode);
  cfg.set<int>("breakaway_type", BreakerSwitcher::breakaway_type);
  cfg.set<int>("breakaway_button", breakaway_index);
  cfg.set<bool>("infinite_breakers",BreakerSwitcher::infinite_breakers);
  cfg.set<bool>("use_secondary", BreakerSwitcher::use_secondary);
}
// do something every frame
// void BreakerSwitcher::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void BreakerSwitcher::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void BreakerSwitcher::on_draw_ui() {
//ImGui::InputInt("breaker up", (int*)&BreakerSwitcher::breakers[0]);
//ImGui::InputInt("breaker down", (int*)&BreakerSwitcher::breakers[1]);
//ImGui::InputInt("breaker left", (int*)&BreakerSwitcher::breakers[2]);
//ImGui::InputInt("breaker right", (int*)&BreakerSwitcher::breakers[3]);
ImGui::Combo("Breaker Type", (int*)&BreakerSwitcher::switcher_mode,"Off\0Switcher\0Cycler\0");

ImGui::Combo("Breakaway Type", (int*)&BreakerSwitcher::breakaway_type, "Off\0Hold Breaker\0On\0");
if (BreakerSwitcher::breakaway_type == 1){
    if (ImGui::Combo("Override button", &breakaway_index,
        "Sword\0Gun\0Jump\0Taunt\0"
        "Lock-on\0Change Target\0Breakaway\0Devil Trigger\0"
        "Dpad Up\0Dpad Down\0Dpad Left\0Dpad Right\0"
        "Breaker Action\0Exceed\0Buster\0Reset Camera\0")) {
        BreakerSwitcher::breakaway_button = bsinputs[breakaway_index];
    }
}
ImGui::Checkbox("Use Secondary Breaker", (bool*)&BreakerSwitcher::use_secondary);
ImGui::Checkbox("Infinite Breakers", (bool*)&BreakerSwitcher::infinite_breakers);

  auto breakerboxstring =
      "Overture\0Ragtime\0Helter Skelter\0Gerbera\0Punchline\0Buster "
      "Arm\0Rawhide\0Tomboy\0Mega Buster\0Gerbera GP01\0Pasta Breaker\0Sweet "
      "Surrender\0Monkey Business\0";
ImGui::Combo("breaker up", (int*)&BreakerSwitcher::breakers[0], breakerboxstring);
ImGui::Combo("breaker down", (int*)&BreakerSwitcher::breakers[1], breakerboxstring);
ImGui::Combo("breaker left", (int*)&BreakerSwitcher::breakers[2], breakerboxstring);
ImGui::Combo("breaker right", (int*)&BreakerSwitcher::breakers[3], breakerboxstring);
ImGui::Combo("breaker up left", (int*)&BreakerSwitcher::breakers[4], breakerboxstring);
ImGui::Combo("breaker up right", (int*)&BreakerSwitcher::breakers[5], breakerboxstring);
ImGui::Combo("breaker down left", (int*)&BreakerSwitcher::breakers[7], breakerboxstring);
ImGui::Combo("breaker down right", (int*)&BreakerSwitcher::breakers[6], breakerboxstring);
}