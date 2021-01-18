#include "AllOrNothing.hpp"
#include "mods/PlayerTracker.hpp"
#include "mods/HeavyDay.hpp"
#include "mods/StyleRank.hpp"
uintptr_t AllOrNothing::jmp_ret{NULL};
uintptr_t AllOrNothing::cheaton{NULL};
uint32_t AllOrNothing::stylebar{0};
bool noonetakesdamage;
bool onehitkill;
float zerovalue = 0.0;
float playermult = 10.0;
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
    newmem:
        push r8
        mov r8, [r14+0x98]
        test r8, r8
        je originalcode

	validation:
        cmp byte ptr [noonetakesdamage], 1
        je nodamage

    cheatcode:
        //if enemy, jump to allornothing
        cmp byte ptr [r8+0xF4], 1
        je allornothing
        //we pvp bois?
        push rax
        mov rax, [HeavyDay::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        jne originalcode
        //if networked enemy don't do damage
        cmp byte ptr [r8+0xF4], 3
        je nodamage

        //if it's the master aka player character, multiply incoming damage
        //because players have stupid amounts of health.
        cmp byte ptr [r8+0xF4], 2
        je playerdamage

        //idk why you'd be here lol
        jmp originalcode
    allornothing:
    //Raw IK?
    cmp byte ptr [onehitkill], 1
    je alldamage
    //if we're not in combat, jump to the original code
    push rax
    mov rax, [AllOrNothing::cheaton]
    cmp byte ptr [rax], 0
    pop rax
    je originalcode
    cmp byte ptr [PlayerTracker::incombat], 0
    je originalcode
    //don't need the enemy entity anymore, so move the style bar in
    //style bar is the style rank we must equal or be above to do damage
    mov r8, [AllOrNothing::stylebar]
    cmp byte ptr [StyleRank::rank],r8b
    //if below, do no damage to the enemy.
    jb nodamage
    jmp originalcode
    nodamage:
        //makes the damage done 0
        mulss xmm6, [zerovalue]
        jmp originalcode
    playerdamage:
        //player multiplier for PVP damage.
        mulss xmm6, [playermult]
        jmp originalcode
    alldamage:
        //this makes the damage done = the enemies current health.
        movss xmm6, [rdi+0x10]
        jmp originalcode
    originalcode:
        pop r8
        movss xmm1,[rdi+0x10]
        jmp qword ptr [AllOrNothing::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> AllOrNothing::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  ischecked = false;
  onpage    = gamepage;
  full_name_string     = "Must Style (+)";
  author_string        = "The Hitchhiker";
  description_string   = "Must Style is a gamemode where you must be above a certain style rank to do damage.";
  AllOrNothing::cheaton = (uintptr_t)&ischecked;

  auto addr = utility::scan(base, "F3 0F 10 4F 10 0F 57 C0 0F 5A");
  if (!addr) {
    return "Unable to find AllOrNothing pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize AllOrNothing";
  }
  return Mod::on_initialize();
}

// during load
//void AllOrNothing::on_config_load(const utility::Config &cfg) {}
// during save
//void AllOrNothing::on_config_save(utility::Config &cfg) {}
// do something every frame
//void AllOrNothing::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void AllOrNothing::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void AllOrNothing::on_draw_ui() {
  ImGui::Checkbox("No one takes damage", &noonetakesdamage);
  ImGui::Checkbox("One hit kill", &onehitkill);
  ImGui::SliderInt("Style Rank to beat", (int*)&AllOrNothing::stylebar, 0, 7);
}