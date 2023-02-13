#include "VergilTrickUpLockedOn.hpp"
#include "PlayerTracker.hpp"

uintptr_t VergilTrickUpLockedOn::jmp_ret1{NULL};
uintptr_t VergilTrickUpLockedOn::jmp_jne1{NULL};

uintptr_t VergilTrickUpLockedOn::jmp_ret2{NULL};
uintptr_t VergilTrickUpLockedOn::jmp_jne2{NULL};

uintptr_t VergilTrickUpLockedOn::jmp_ret3{NULL};
uintptr_t VergilTrickUpLockedOn::jmp_je3{NULL};

float leftstickthreshold{0.0f};
float xmm0backup1{0.0f};
float xmm0backup2{0.0f};
bool VergilTrickUpLockedOn::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour1() { // Disable Trick To
	__asm {
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        jne code
        cmp byte ptr [VergilTrickUpLockedOn::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        movss [xmm0backup1], xmm0
        movss xmm0, [rdx+00000FF4h]
        comiss xmm0, [leftstickthreshold]
        movss xmm0, [xmm0backup1]
        ja cheatcode2
        jmp cheatcode2

    code:
        cmp [rdx+00000ED0h], sil
        jne jnejmp
        jmp qword ptr [VergilTrickUpLockedOn::jmp_ret1]

    cheatcode2:
		jmp qword ptr [VergilTrickUpLockedOn::jmp_ret1]

    jnejmp:
        jmp qword ptr [VergilTrickUpLockedOn::jmp_jne1]
	}
}

static naked void detour2() { // Enable Trick Up While Locked On
	__asm {
        cmp [PlayerTracker::playerid], 4
        jne code
        cmp byte ptr [VergilTrickUpLockedOn::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        movss [xmm0backup2], xmm0
        movss xmm0, [rdx+00000FF4h]
        comiss xmm0, [leftstickthreshold]
        movss xmm0, [xmm0backup2]
        ja cheatcode2

    code:
        cmp byte ptr [rdx+00000ED0h], 00
        jne jnejmp
        jmp qword ptr [VergilTrickUpLockedOn::jmp_ret2]

    cheatcode2:
		jmp qword ptr [VergilTrickUpLockedOn::jmp_ret2]

    jnejmp:
        jmp qword ptr [VergilTrickUpLockedOn::jmp_jne2]
	}
}

static naked void detour3() { // Disable Directional Dodges
	__asm {
        cmp [PlayerTracker::playerid], 4
        jne code
        cmp byte ptr [VergilTrickUpLockedOn::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [rdi+00000ED0h], 00 // is not locked on?
        je code
        comiss xmm0, [leftstickthreshold]
        ja cheatcode2

    code:
        test al, al
        je jejmp
        jmp qword ptr [VergilTrickUpLockedOn::jmp_ret3]

    cheatcode2:
		jmp qword ptr [VergilTrickUpLockedOn::jmp_je3]

    jejmp:
        jmp qword ptr [VergilTrickUpLockedOn::jmp_je3]
	}
}

// clang-format on

void VergilTrickUpLockedOn::on_config_load(const utility::Config& cfg)
{
    leftStickAngle = cfg.get<float>("VergilTrickUpLockedOn.leftStickAngle").value_or(25.0f);
}

void VergilTrickUpLockedOn::on_config_save(utility::Config& cfg)
{
    cfg.set<float>("VergilTrickUpLockedOn.leftStickAngle", leftStickAngle);
}

void VergilTrickUpLockedOn::on_draw_ui()
{
    ImGui::TextWrapped("Left stick forward angle threshold:");
    UI::SliderFloat("##_angleForwardThreshold", &leftStickAngle, 3.5f, 60.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
}

void VergilTrickUpLockedOn::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> VergilTrickUpLockedOn::on_initialize() {
  init_check_box_info();

  m_is_enabled            = &VergilTrickUpLockedOn::cheaton;
  m_on_page               = Page_VergilTrick;

  m_full_name_string     = "Trick Up On Forward + Trick (+)";
  m_author_string        = "SSSiyan";
  m_description_string   = "Trick Up without letting go of Lock On.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr1 = m_patterns_cache->find_addr(base, "40 38 B2 D0 0E 00 00 75");
  if (!addr1) {
    return "Unable to find VergilTrickUpLockedOn pattern1.";
  }
  VergilTrickUpLockedOn::jmp_jne1 = addr1.value() + 69;

  auto addr2 = m_patterns_cache->find_addr(base, "80 BA D0 0E 00 00 00 0F 85 86");
  if (!addr2) {
    return "Unable to find VergilTrickUpLockedOn pattern2.";
  }
  VergilTrickUpLockedOn::jmp_jne2 = addr2.value() + 147;
  
  auto addr3 = m_patterns_cache->find_addr(base, "84 C0 0F 84 E7 02 00 006");
  if (!addr3) {
    return "Unable to find VergilTrickUpLockedOn pattern3.";
  }
  VergilTrickUpLockedOn::jmp_je3 = addr3.value() + 751;

  if (!install_hook_absolute(addr1.value(), m_function_hook1, &detour1, &jmp_ret1, 9)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilTrickUpLockedOn1";
  }
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 13)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilTrickUpLockedOn2";
  }
  if (!install_hook_absolute(addr3.value(), m_function_hook3, &detour3, &jmp_ret3, 8)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilTrickUpLockedOn3";
  }
  return Mod::on_initialize();
}

// void VergilTrickUpLockedOn::on_draw_ui() {}
