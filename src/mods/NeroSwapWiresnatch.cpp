
#include "PlayerTracker.hpp"
#include "NeroSwapWiresnatch.hpp"
uintptr_t NeroSwapWiresnatch::jmp_ret1{NULL};
uintptr_t NeroSwapWiresnatch::jmp_ret2{NULL};
bool NeroSwapWiresnatch::cheaton{NULL};
bool swapInputs{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour1() { // Air
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [NeroSwapWiresnatch::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [swapInputs], 1
        je cheatcode2
        cmp rbp, 3
        je code
        mov byte ptr [rax+10h], 43
		jmp qword ptr [NeroSwapWiresnatch::jmp_ret1]

    cheatcode2:
        cmp rbp, 3 // 0
        jne code
        mov byte ptr [rax+10h], 43
		jmp qword ptr [NeroSwapWiresnatch::jmp_ret1]

    code:
        mov byte ptr [rax+10h], 0000001Ah
		jmp qword ptr [NeroSwapWiresnatch::jmp_ret1]
	}
}

static naked void detour2() { // Ground
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [NeroSwapWiresnatch::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [swapInputs], 1
        je cheatcode2
        cmp rbp, 3
        je code
        mov byte ptr [rax+10h], 43
		jmp qword ptr [NeroSwapWiresnatch::jmp_ret2]

    cheatcode2:
        cmp rbp, 3 // 0
        jne code
        mov byte ptr [rax+10h], 43
		jmp qword ptr [NeroSwapWiresnatch::jmp_ret2]

    code:
        mov byte ptr [rax+10h], 00000019h
        jmp qword ptr [NeroSwapWiresnatch::jmp_ret2]
	}
}

// clang-format on

void NeroSwapWiresnatch::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NeroSwapWiresnatch::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &NeroSwapWiresnatch::cheaton;
  m_on_page             = wiresnatch;

  m_full_name_string   = "Angel and Devil Snatch";
  m_author_string      = "SSSiyan";
  m_description_string = "Replaces Wiresnatch with rawhide snatch to the enemy and adds knockback.";

  set_up_hotkey();

  auto base  = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr1 = utility::scan(base, "C7 40 10 1A 00 00 00 E9");
  if (!addr1) {
    return "Unable to find NeroSwapWiresnatch pattern.";
  }
  auto addr2 = utility::scan(base, "C7 40 10 19 00 00 00 48 8B D0");
  if (!addr2) {
    return "Unable to find NeroSwapWiresnatch pattern.";
  }

  if (!install_hook_absolute(addr1.value(), m_function_hook1, &detour1, &jmp_ret1, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroSwapWiresnatch1";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroSwapWiresnatch2";
  }
  return Mod::on_initialize();
}

void NeroSwapWiresnatch::on_config_load(const utility::Config& cfg) {
  swapInputs = cfg.get<bool>("nero_angel_devil_swap_inputs").value_or(false);
}
void NeroSwapWiresnatch::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("nero_angel_devil_swap_inputs", swapInputs);
}

void NeroSwapWiresnatch::on_draw_ui() {
  ImGui::Checkbox("Swap Inputs", &swapInputs);
}
