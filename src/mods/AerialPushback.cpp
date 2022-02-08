#include "AerialPushback.hpp"
#include "PlayerTracker.hpp"
#include "AerialPushbackVertical.hpp"
uintptr_t AerialPushback::jmp_ret{NULL};
uintptr_t AerialPushback::jmp_ret2{NULL};
bool AerialPushback::cheaton{NULL};

bool kalinaPushback{NULL};

/*
float pushBackTest    = -2.0f; // negative = backwards
float pushResolveTest =  0.90f;
*/

float kaPushbackFloat = 0.90f;

// clang-format off
// only in clang/icl mode on x64, sorry

/*static naked void detour() { // impulse given to character, sadly called multiple times during an attack
	__asm {
        cmp byte ptr [AerialPushback::cheaton], 1
        jne code
        cmp [PlayerTracker::playerid], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp dword ptr [PlayerTracker::playermoveid], 
        je morepushback
        jmp code

    morepushback:
        movss xmm2,
        jmp code

    code:
        movss [rdi+28h], xmm2
        jmp qword ptr [AerialPushback::jmp_ret]
	}
}*/

static naked void detour2() { // speed at which the character returns to 0 inertia
	__asm {
        cmp byte ptr [AerialPushback::cheaton], 1
        jne code
        cmp [PlayerTracker::playerid], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp dword ptr [PlayerTracker::playermoveid], 03E80141h // DKA L air
        je kalinapush
        cmp dword ptr [PlayerTracker::playermoveid], 03ED0141h // DKA R air
        je kalinapush
        cmp dword ptr [PlayerTracker::playermoveid], 03E80140h // KA air
        je kalinapush
        jmp code

    kalinapush:
        cmp byte ptr [kalinaPushback], 1
        jne code
        mulss xmm15, [kaPushbackFloat]
        jmp code

    code:
        movss [rdi+28h], xmm15
        jmp qword ptr [AerialPushback::jmp_ret2]
	}
}

// clang-format on

void AerialPushback::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> AerialPushback::on_initialize() {
    init_check_box_info();

  m_is_enabled = &AerialPushback::cheaton;
  m_on_page    = dantecheat;
  m_full_name_string   = "Move Pushback Edits (+)";
  m_author_string      = "SSSiyan";
  m_description_string = "Your character will be pushed back more than usual when performing certain moves.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  /*auto addr = AerialPushbackVertical::jmp_ret; // patterns->find_addr(base, "F3 0F 11 57 28 48 8B 43 50 48 83 78 18 00 0F"); // DevilMayCry5.exe+1445670
  if (!addr) {
    return "Unable to find AerialPushback pattern.";
  }*/
  auto addr2 = AerialPushbackVertical::jmp_ret; // patterns->find_addr(base, "F3 44 0F 11 7F 28");
  if (!addr2) {
    return "Unable to find AerialPushback pattern2.";
  }
  /*
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize AerialPushback";
  }*/
   if (!install_hook_absolute(addr2, m_function_hook2, &detour2, &jmp_ret2, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize AerialPushback2";
  }
  return Mod::on_initialize();
}

void AerialPushback::on_config_load(const utility::Config& cfg) {
  kalinaPushback = cfg.get<bool>("kalina_pushback").value_or(true);
}
void AerialPushback::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("kalina_pushback", kalinaPushback);
}

void AerialPushback::on_draw_ui() {
  ImGui::Checkbox("Kalina air shot", &kalinaPushback);
}
