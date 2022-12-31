#include "VergilInfSDT.hpp"
#include "PlayerTracker.hpp"

void VergilInfSDT::dt_update_hook(uintptr_t threadCntx, uintptr_t pl0800)
{
    _mod->_pl0800DtUpdateHook->get_original<decltype(dt_update_hook)>()(threadCntx, pl0800);
    if (cheaton)
        *(float*)(pl0800 + 0x1B20) = 10000.0f;
}

// clang-format on

void VergilInfSDT::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> VergilInfSDT::on_initialize() {
  init_check_box_info();

  m_is_enabled            = &VergilInfSDT::cheaton;
  m_on_page               = Page_VergilSDT;

  m_full_name_string     = "Infinite SDT";
  m_author_string        = "SSSiyan, V.P.Zadov";
  m_description_string   = "Sets the SDT Bar to maximum and stops it from decreasing.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  auto addr = m_patterns_cache->find_addr(base, "40 53 57 48 83 EC 58 80");
  //DevilMayCry5.app_PlayerVergilPL__updateDevilTrigger113907
  if (!addr) {
    return "Unable to find VergilInfSDT pattern.";
  }

  _pl0800DtUpdateHook = std::make_unique<FunctionHook>(addr.value(), &dt_update_hook);
  if (!_pl0800DtUpdateHook->create())
      return "Unable to create VergilInfSDT._pl0800DtUpdateHook";
  return Mod::on_initialize();
}

// void VergilInfSDT::on_draw_ui(){}
