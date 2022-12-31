#include "VergilSDTAccumulateRework.hpp"

void VergilSDTAccumulateRework::on_pl_add_dt(uintptr_t threadCtxt, uintptr_t pl, float *val, int dtAddType, bool fixedValue)
{
    if (!cheaton || pl == 0 || *(int*)(pl + 0xE64) != 4 || *val < 0 || DMC3JCE::jceController->is_executing() || _vergilQSMod->is_quicksilver_active())
        return;
    float sdtAdd = isConstInc ? sdtPointsToAdd : *val;
    float *curSdt = (float*)(pl + 0x1B20);
    if (*curSdt + sdtAdd >= 10000.0f)
        *curSdt = 10000.0f;
    else
        *curSdt += sdtAdd;
}

void VergilSDTAccumulateRework::on_config_load(const utility::Config &cfg) {
  isConstInc = cfg.get<bool>("VergilSDTAccumulateRework.isConstInc").value_or(false);
  sdtPointsToAdd = cfg.get<float>("vergilSdtPointsToAdd").value_or(55.0);
}

void VergilSDTAccumulateRework::on_config_save(utility::Config &cfg) {
  cfg.set<float>("vergilSdtPointsToAdd", sdtPointsToAdd);
  cfg.set<bool>("VergilSDTAccumulateRework.isConstInc", isConstInc);
}

std::optional<std::string> VergilSDTAccumulateRework::on_initialize() {
  init_check_box_info();
  m_is_enabled = &VergilSDTAccumulateRework::cheaton;
  m_on_page = Page_VergilSDT;
  m_full_name_string = "SDT Accumulate Rework (+)";
  m_author_string = "V.P.Zadov";
  m_description_string = "Vergil's SDT meter fills alongside his DT meter.";

  set_up_hotkey();

  return Mod::on_initialize();
}

void VergilSDTAccumulateRework::on_draw_ui() {
  ImGui::Checkbox("Set custom amount of SDT gained on DT increase", &isConstInc);
  if (isConstInc)
  {
      ImGui::TextWrapped("Amount of SDT points gained on DT increase:");
      UI::SliderFloat("##SDT Points Slider", &sdtPointsToAdd, 50.0, 180.0, "%.2f");
  }
}

// void VergilSDTAccumulateRework::on_draw_debug_ui() {}

void VergilSDTAccumulateRework::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
