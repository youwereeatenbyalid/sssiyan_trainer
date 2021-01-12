
#include "DamageMultiplier.hpp"
#include "utility/Scan.hpp"

uintptr_t DamageMultiplier::jmp_ret{NULL};
bool damagemultipliercheck;
float DamageMultiplier::enemyhpvalue = 0.0f;

float playerdamagemultiplier         = 1.0f;
float enemydamagemultiplier          = 1.0f;


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [damagemultipliercheck],1
        je damagemultcode
        jmp code

    damagemultcode:
        cmp dword ptr [rdi+134h], 7077968  // 'Pl' in 'Play'
        je enemydamageoutput
        cmp dword ptr [rdi-18Ch], 6225996  // 'L_' in L_B_Hand'
        je enemydamageoutput

        cmp dword ptr [rdi-16Ch], 6881357  // 'Mi' in 'Miss'
        je BPcheck
        jmp playerdamageoutput

    BPcheck:
        cmp dword ptr [rdi+14h], 469c4000h // 20,000.0f
        je enemydamageoutput
        jmp playerdamageoutput

    playerdamageoutput:
        movss xmm2, [rdi+10h]
        subss xmm2, xmm1
        movss xmm1, [rdi+10h]
        jmp effectdamage

    enemydamageoutput:
        movss xmm3, [rdi+10h]
        subss xmm3, xmm1
        movss xmm1, [rdi+10h]
        mulss xmm3, [enemydamagemultiplier]
        subss xmm1, xmm3
        jmp code

    effectdamage:
        mulss xmm2, [playerdamagemultiplier]
        subss xmm1, xmm2
        movss [DamageMultiplier::enemyhpvalue], xmm1
        jmp code

    code:
        movss [rdi+10h], xmm1
        jmp qword ptr [DamageMultiplier::jmp_ret]
	}
}

// clang-format on

std::optional<std::string> DamageMultiplier::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "C9 F3 0F 11 4F 10 48");
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

void DamageMultiplier::on_draw_ui() {
  ImGui::Checkbox("Damage Multiplier", &damagemultipliercheck);
  ImGui::PushItemWidth(100);
  ImGui::InputFloat("Player Damage Output", &playerdamagemultiplier, 0.1f);
  ImGui::PopItemWidth();
  ImGui::PushItemWidth(100);
  ImGui::InputFloat("Enemy Damage Output", &enemydamagemultiplier, 0.1f);
  ImGui::PopItemWidth();
}
