#include "LandCancels.hpp"
#include "PlayerTracker.hpp"

uintptr_t LandCancels::jmp_ret{NULL};
bool LandCancels::cheaton{NULL};
bool timedChargeShotCancels{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [LandCancels::cheaton], 1
        jne code
        jmp cheatcode

    cheatcode:
        cmp byte ptr [PlayerTracker::isgrounded], 1
        je code
        cmp [PlayerTracker::playerid], 0
        je nerocheat
        jmp retcode

    nerocheat:
        cmp byte ptr [timedChargeShotCancels], 1 // move this down when we have more toggles
        jne retcode
        push r11
        push r12
        push r9

        // some kind of action and grounded check? only shows 1-4 when rbp = treelayer
        cmp rsi,rdi 
        jne popret
        // hitch stuff: get layer offset
        mov r9, [rbp+0x1F80]
        and r9, 1
        imul r9, r9, 0xFB0

        // charge shot cancels
        cmp dword ptr [rbp+r9+0xA8], 60 // Motion Bank
        jne popret
        cmp dword ptr [rbp+r9+0xB0], 700 // Motion ID
        jne popret
        cmp dword ptr [rbp+r9+0xC8], 0x41400000 // PrevFrame // 12.0f
        ja forceland
        jmp popcode

    forceland:
        pop r9
        pop r12
        pop r11
        mov dword ptr [rdx+34h], 2 // can land
        jmp qword ptr [LandCancels::jmp_ret]

    popcode:
        pop r9
        pop r12
        pop r11
    code:
        mov dword ptr [rdx+34h], 0 // can't land
    retcode:
        jmp qword ptr [LandCancels::jmp_ret]

    popret:
        pop r9
        pop r12
        pop r11
        jmp qword ptr [LandCancels::jmp_ret]
	}
}

// clang-format on

void LandCancels::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> LandCancels::on_initialize() {
  init_check_box_info();

  ischecked = &LandCancels::cheaton;
  onpage    = mechanics;

  full_name_string = "Land Cancels (+)";
  author_string    = "SSSiyan";
  description_string = "Touching the floor will cancel your current aerial attack.";

    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns.find_addr(base, "C7 42 34 00 00 00 00 C3 CC CC 48");
  if (!addr) {
    return "Unable to find LandCancels pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize LandCancels";
  }
  return Mod::on_initialize();
}

void LandCancels::on_config_load(const utility::Config& cfg) {
  timedChargeShotCancels = cfg.get<bool>("timed_charge_shot_cancels").value_or(false);
}
void LandCancels::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("timed_charge_shot_cancels", timedChargeShotCancels);
}

void LandCancels::on_draw_ui() {
  ImGui::TextWrapped("Nero");
  ImGui::Checkbox("Timed Charge Shot Cancels", &timedChargeShotCancels);
  ImGui::ShowHelpMarker("Enabling this will disable land cancelling charge shots until the bullet has come out.", 450.0F);
}
