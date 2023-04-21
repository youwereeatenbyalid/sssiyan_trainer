
#include "DamageType.hpp"
#include "PlayerTracker.hpp"
uintptr_t DamageType::jmp_ret{ NULL };
bool danteNoHelmbreakerKnockdown{ false };
bool DamageType::cheaton{ false };

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
    __asm {
        cmp [PlayerTracker::playerid], 1
        je dantecode
        jmp code

    dantecode:
        cmp byte ptr [DamageType::cheaton], 1
        jne code
        cmp byte ptr [danteNoHelmbreakerKnockdown], 1
        je dantehelmbreaker
        jmp code

    dantehelmbreaker:
        cmp dword ptr [rax+0xA8], Dante_Rebellion_HelmBreaker_02
        je noKnockback
        cmp dword ptr [rax+0xA8], Dante_Spada_HelmBreaker_02
        je noKnockback
        cmp dword ptr [rax+0xA8], Dante_DevilSword_HelmBreaker_02
        je noKnockback
        jmp code

    noKnockback:
        mov ebp, 1
        jmp retcode

    code:
        mov ebp, [rax+0xCC]
    retcode:
        jmp qword ptr [DamageType::jmp_ret]
    }
}

// clang-format on

void DamageType::init_check_box_info() {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DamageType::on_initialize() {
    init_check_box_info();

    m_is_enabled = &DamageType::cheaton;
    m_on_page = Page_Mechanics;

    m_full_name_string = "Knockback Edits (+)";
    m_author_string = "Siyan";
    m_description_string = "Add or remove knockbacks/launches etc from moves.";

    set_up_hotkey();

    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
    auto addr = m_patterns_cache->find_addr(base, "8B A8 CC 00 00 00 E8");
    if (!addr) {
        return "Unable to find DamageType pattern.";
    }

    if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 6)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize DamageType";
    }
    return Mod::on_initialize();
}

void DamageType::on_config_load(const utility::Config& cfg) {
    danteNoHelmbreakerKnockdown = cfg.get<bool>("disable_dante_helmbreaker_knockdown").value_or(false);
}
void DamageType::on_config_save(utility::Config& cfg) {
    cfg.set<bool>("disable_dante_helmbreaker_knockdown", danteNoHelmbreakerKnockdown);
}

void DamageType::on_draw_ui() {
    ImGui::Text("Dante");
    ImGui::Checkbox("Remove Helmbreaker Knockdown", &danteNoHelmbreakerKnockdown);
}

/*
None = -1,
Minimum = 0,
NormalS = 1,
NormalL = 2,
NormalLL = 3,
Down = 4,
DownBound = 5,
Slam = 6,
SlamBound = 7,
Upper = 8,
HyperUpper = 9,
HyperUpperAir = 10,
Blown = 11,
Collapse = 12,
RollingUpper = 13,
CheckSpin = 14,
Spin = 15,
DiagonalBlown = 16,
WindS = 17,
WindL = 18,
SnatchRelase = 19,
StraightBlown = 20,
Tremor = 21,
Inhale = 22,
MotionContinue = 23,
Explosion = 24,
*/
