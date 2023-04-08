
#include "PlayerTracker.hpp"
#include "NeroDisableWiresnatch.hpp"
uintptr_t NeroDisableWiresnatch::jmp_ret1{NULL};
uintptr_t NeroDisableWiresnatch::jmp_jne1{NULL};

uintptr_t NeroDisableWiresnatch::jmp_ret2{NULL};
uintptr_t NeroDisableWiresnatch::jmp_jne2{NULL};

bool NeroDisableWiresnatch::cheaton{NULL};
bool NeroDisableWiresnatch::movementoverride{NULL};
bool NeroDisableWiresnatch::flipoverride{NULL};
//PlayerTracker::redirect
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour1() {
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [NeroDisableWiresnatch::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [NeroDisableWiresnatch::movementoverride], 0
        je retinfo
        cmp byte ptr [rdx+00000ED0h], 0 //lock-on check
        je retinfo
        cmp byte ptr [PlayerTracker::redirect], 1 //if movement threshhold allow snatch
        je flipcode
        cmp byte ptr[NeroDisableWiresnatch::flipoverride], 1 //if flip override jump to jne anyway
        je jnecode
    retinfo:
		jmp qword ptr [NeroDisableWiresnatch::jmp_ret1]

    flipcode:
        cmp byte ptr [NeroDisableWiresnatch::flipoverride], 0 //if not flip override jump to jne
        je jnecode
        jmp qword ptr[NeroDisableWiresnatch::jmp_ret1]

    code:
        cmp byte ptr [rdx+00000ED0h], 00
        jne jnecode
		jmp qword ptr [NeroDisableWiresnatch::jmp_ret1]

    jnecode:
        jmp qword ptr [NeroDisableWiresnatch::jmp_jne1]
	}
}

static naked void detour2() {
	__asm {
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code

        cmp byte ptr [NeroDisableWiresnatch::cheaton], 1
        je cheatcode
        jmp code

    cheatcode :
        cmp byte ptr [NeroDisableWiresnatch::movementoverride], 0
        je retinfo
        cmp byte ptr [rdx+00000ED0h], 0 //lock-on check
        je retinfo
        cmp byte ptr [PlayerTracker::redirect], 1 //if movement threshhold allow snatch
        je flipcode
        cmp byte ptr [NeroDisableWiresnatch::flipoverride], 1 //if flip override jump to jne anyway
        je jnecode
    retinfo:
        jmp qword ptr [NeroDisableWiresnatch::jmp_ret2]

    flipcode:
        cmp byte ptr [NeroDisableWiresnatch::flipoverride], 0 //if not flip override jump to jne
        je jnecode
        jmp qword ptr [NeroDisableWiresnatch::jmp_ret2]

    code:
        cmp byte ptr [rdx+00000ED0h], 00
        jne jnecode
        jmp qword ptr [NeroDisableWiresnatch::jmp_ret2]

    jnecode:
        jmp qword ptr [NeroDisableWiresnatch::jmp_jne2]
	}
}

// clang-format on

void NeroDisableWiresnatch::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> NeroDisableWiresnatch::on_initialize() {
  init_check_box_info();

  m_is_enabled          = &NeroDisableWiresnatch::cheaton;
  m_on_page             = Page_Wiresnatch;

  m_full_name_string   = "Disable Wiresnatch (+)";
  m_author_string      = "SSSiyan";
  m_description_string = "Disables Wiresnatch to allow breaker abilities while locked on.";

  set_up_hotkey();

  auto base  = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr1 = m_patterns_cache->find_addr(base, "80 BA D0 0E 00 00 00 75 21");
  if (!addr1) {
    return "Unable to find NeroDisableWiresnatch1 pattern.";
  }
  auto addr2 = m_patterns_cache->find_addr(base, "80 BA D0 0E 00 00 00 75 2A");
  if (!addr2) {
    return "Unable to find NeroDisableWiresnatch2 pattern.";
  }

  if (!install_hook_absolute(addr1.value(), m_function_hook1, &detour1, &jmp_ret1, 9)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroDisableWiresnatch1";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 9)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize NeroDisableWiresnatch2";
  }

  NeroDisableWiresnatch::jmp_jne1 = addr1.value() + 42;
  NeroDisableWiresnatch::jmp_jne2 = addr2.value() + 51;

  return Mod::on_initialize();
}


void NeroDisableWiresnatch::on_config_load(const utility::Config& cfg) {
    NeroDisableWiresnatch::movementoverride = cfg.get<bool>("disable_breakaway_movement_override").value_or(false);
    NeroDisableWiresnatch::flipoverride = cfg.get<bool>("disable_breakaway_flip_override").value_or(false);
}
void NeroDisableWiresnatch::on_config_save(utility::Config& cfg) {
    cfg.set<bool>("disable_breakaway_movement_override", NeroDisableWiresnatch::movementoverride);
    cfg.set<bool>("disable_breakaway_flip_override", NeroDisableWiresnatch::flipoverride);
}
void NeroDisableWiresnatch::on_draw_ui() {
    if (ImGui::Checkbox("Allow snatch when stick is not neutral", (bool*)&NeroDisableWiresnatch::movementoverride));
    if(movementoverride){
        ImGui::Checkbox("Invert behavior (Allow snatch when stick is neutral)", (bool*)&NeroDisableWiresnatch::flipoverride);
    }
}
