#include "VergilAirTrickNoSS.hpp"

// clang-format off

bool VergilAirTrickNoSS::check_air_trick_hook(uintptr_t threadCntxt, uintptr_t pl0800)
{
    if (!cheaton)
        return _mod->_checkAirTrickHook->get_original<decltype(check_air_trick_hook)>()(threadCntxt, pl0800);
    auto lockOnTarget = *(uintptr_t*)(pl0800 + 0x428);
    if (lockOnTarget == 0)
        return false;
    auto target = *(uintptr_t*)(lockOnTarget + 0x10);
    if (target == 0)
        return false;
    auto targetGameObj = *(uintptr_t*)(target + 0x48);
    if (targetGameObj == 0)
        return false;
    auto trickTargetCntrl = *(uintptr_t*)(pl0800 + 0x19B0);
    if (trickTargetCntrl == 0)
        return false;
    _mod->_trickTargetCntrStartAirTrickMethod->call(threadCntxt, trickTargetCntrl, pl0800);
    _mod->_pl0800SetAirTrickActionMethod->call(threadCntxt, pl0800, targetGameObj);
    return true;
}

// clang-format on

void VergilAirTrickNoSS::init_check_box_info() {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> VergilAirTrickNoSS::on_initialize() {
    init_check_box_info();

    m_is_enabled = &VergilAirTrickNoSS::cheaton;
    m_on_page = Page_VergilTrick;

    m_full_name_string = "Disable Embedded Swords";
    m_author_string = "V.P.Zadov";
    m_description_string = "Allows you to trick to enemies without using an embedded sword.";

    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

    auto checkAirTrickAddr = m_patterns_cache->find_addr(base, "FE 01 EB 8F CC CC CC CC CC 48 89 5C 24 18");
    //DevilMayCry5.app_PlayerVergilPL__checkAirTrick113998 (-0x9)
    if (!checkAirTrickAddr) {
        return "Unable to find VergilAirTrickNoSS.checkAirTrickAddr pattern.";
    }

    _checkAirTrickHook = std::make_unique<FunctionHook>(checkAirTrickAddr.value() + 0x9, &check_air_trick_hook);
    if (!_checkAirTrickHook->create())
        return "Unable to create VergilAirTrickNoSS._checkAirTrickHook.";

    return Mod::on_initialize();
}

// void VergilAirTrickNoSS::on_draw_ui() {}
