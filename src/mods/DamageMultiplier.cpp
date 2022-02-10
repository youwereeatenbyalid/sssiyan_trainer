
#include "DamageMultiplier.hpp"

uintptr_t DamageMultiplier::jmp_ret{NULL};
bool DamageMultiplier::cheaton{NULL};
float DamageMultiplier::enemyhpvalue = 0.0f;

// float playerdamagemultiplier         = 1.0f;
// float enemydamagemultiplier          = 1.0f;

// clang-format off
// only in clang/icl mode on x64, sorry

// NOTE: A LOT OF THIS IS COMMENTED OUT AS WE NOW ONLY USE THIS CHEAT FOR BACKING UP ENEMY HP

static naked void detour() {
	__asm {                                // Compares will play out even without cheat enabled so we can get a backup of last hit entitiy's HP
        // cmp dword ptr [rdi+134h], 7077968  // 'Pl' in 'Play'
        // je enemydamageoutput
        // cmp dword ptr [rdi-18Ch], 6225996  // 'L_' in L_B_Hand'
        // je enemydamageoutput
        // cmp dword ptr [rdi-16Ch], 6881357  // 'Mi' in 'Miss'
        // je BPcheck
        jmp playerdamageoutput

    BPcheck:
        cmp dword ptr [rdi+14h], 469c4000h // 20,000.0f
        je enemydamageoutput
        jmp playerdamageoutput

    enemydamageoutput:
        // push rax
        // mov rax, [DamageMultiplier::cheaton]
        // cmp byte ptr [rax], 1
        // pop rax
        // je enemydamageoutputaffect          // If cheat is ticked, affect incoming damage. If not, don't
        jmp code

    // enemydamageoutputaffect:
        // movss xmm3, [rdi+10h]
        // subss xmm3, xmm1
        // movss xmm1, [rdi+10h]
        // mulss xmm3, [enemydamagemultiplier]
        // subss xmm1, xmm3
        // jmp code

    playerdamageoutput:
        // push rax
        // mov rax, [DamageMultiplier::cheaton]
        // cmp byte ptr [rax], 1
        // pop rax
        // je playerdamageoutputaffect         // If cheat is ticked, affect outgoing damage. If not, don't
        jmp backupenemyhp

    // playerdamageoutputaffect:
        // movss xmm2, [rdi+10h]
        // subss xmm2, xmm1
        // movss xmm1, [rdi+10h]
        // mulss xmm2, [playerdamagemultiplier]
        // subss xmm1, xmm2
    backupenemyhp:
        movss [DamageMultiplier::enemyhpvalue], xmm1
        jmp code

    code:
        movss [rdi+10h], xmm1
        jmp qword ptr [DamageMultiplier::jmp_ret]
	}
}

// clang-format on

void DamageMultiplier::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DamageMultiplier::on_initialize() {
  init_check_box_info();

  // ischecked             = &DamageMultiplier::cheaton;
  // onpage                = gamemode;

  m_full_name_string      = "Damage Multiplier (+)";
  m_author_string         = "SSSiyan";
  m_description_string    = "Allows you to adjust the damage output of players and enemies.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "C9 F3 0F 11 4F 10 48");
  if (!addr) {
    return "Unable to find DamageMultiplier pattern.";
  }

  if (!install_hook_absolute(addr.value()+1, m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DamageMultiplier";
  }
  return Mod::on_initialize();
}

// void DamageMultiplier::on_draw_ui() {
  // ImGui::PushItemWidth(100);
  // ImGui::InputFloat("Player Damage Output", &playerdamagemultiplier, 0.1f);
  // ImGui::PopItemWidth();
  // ImGui::PushItemWidth(100);
  // ImGui::InputFloat("Enemy Damage Output", &enemydamagemultiplier, 0.1f);
  // ImGui::PopItemWidth();
// 
