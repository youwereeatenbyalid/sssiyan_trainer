#include "AllStartManual.hpp"
#include "PlayerTracker.hpp"
uintptr_t AllStartManual::jmp_ret{NULL};
bool AllStartManual::cheaton{NULL};
bool enemystepcancels;
float cancelfloat = 0;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() { // "DevilMayCry5.exe"+2C723DF
	__asm {
        cmp byte ptr [AllStartManual::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        push r11
        cmp [PlayerTracker::playerid], 0
        je nerocompare
        cmp [PlayerTracker::playerid], 1
        je dantecompare
        cmp [PlayerTracker::playerid], 4
        je vergilcompare
        jmp code

    nerocompare:
        cmp byte ptr [enemystepcancels], 1
        je nero_enemy_step_compare
        jmp popcode

    dantecompare:
        cmp byte ptr [enemystepcancels], 1
        je dante_enemy_step_compare
        jmp popcode

    vergilcompare:
        cmp byte ptr [enemystepcancels], 1
        je vergil_enemy_step_compare
        jmp popcode

//___________________________________________________________Nero

    nero_enemy_step_compare:
        mov r11, 32651591227605101 //myst (Enemystep_Fly)
        cmp r11, [rdx-0x5766]
        je nero_enemy_step_check
        jmp popcode

    nero_enemy_step_check:
        mov r11, 19703656398979173 // ep_F (Enemystep_Fly)
        cmp r11, [rdx-0x575E]
        je cancellable
        jmp popcode

//___________________________________________________________Dante

    dante_enemy_step_compare:
        mov r11, 32651591227605101 //myst (Enemystep_Fly)
        cmp r11, [rdx-0x6246]
        je dante_enemy_step_check
        jmp popcode

    dante_enemy_step_check:
        mov r11, 19703656398979173 // ep_F (Enemystep_Fly)
        cmp r11, [rdx-0x623E]
        je cancellable
        jmp popcode

//___________________________________________________________Vergil

    vergil_enemy_step_compare:
        mov r11, 32651591227605101 //myst (Enemystep_Fly)
        cmp r11, [rdx-0x60C6]
        je vergil_enemy_step_check
        jmp popcode

    vergil_enemy_step_check:
        mov r11, 19703656398979173 // ep_F (Enemystep_Fly)
        cmp r11, [rdx-0x60BE]
        je cancellable
        jmp popcode

//___________________________________________________________Code

    cancellable:
        movss xmm0, [cancelfloat]
        jmp popret

    code:
        movss xmm0,[rdx+0x04]
        jmp qword ptr [AllStartManual::jmp_ret]

    popcode:
        movss xmm0,[rdx+0x04]
    popret:
        pop r11
        jmp qword ptr [AllStartManual::jmp_ret]
	}
}

/*
Find AllStart value location
Subtract string location

Subtract 4 from difference, use this for first compare location
Subtract 8 from difference, use this for second compare

Use the int64 value from the string location for the first compare value
Use the int64 value from 8 bytes later than the string location for the second compare value
*/

// clang-format on

void AllStartManual::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> AllStartManual::on_initialize() {
  init_check_box_info();

  ischecked          = &AllStartManual::cheaton;
  onpage             = enemystep;

  full_name_string   = "AllStart 2 (+)";
  author_string      = "SSSiyan";
  description_string = "Allows you to cancel out of a selection of moves with any other move.";

  // auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  constexpr ptrdiff_t addr = 0x2C723DF;
  //auto addr = base + 0x2C723DF;
  // auto addr      = utility::scan(base, "66 C7 47 5E 00 00");
  // if (!addr) {
  //   return "Unable to find AllStartManual pattern.";
  // }
  if (!install_hook_offset(addr, m_function_hook, &detour, &jmp_ret, 5)) { // Couldn't find unique AoB, used offset
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize AllStartManual";
  }
  return Mod::on_initialize();
}

void AllStartManual::on_config_load(const utility::Config& cfg) {
  enemystepcancels = cfg.get<bool>("enemy_step_cancels_").value_or(true);
}
void AllStartManual::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("enemy_step_cancels", enemystepcancels);
}

void AllStartManual::on_draw_ui() {
  ImGui::Checkbox("Enemy Step Cancels", &enemystepcancels);
}
