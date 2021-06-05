#pragma once
#include "Mod.hpp"
#include "mods/VergilSDTFormTracker.hpp"
#include "mods/VergilSetMaxJJC.hpp"
#include "mods/VergilInfJdCs.hpp"
#include "sdk/ReClass.hpp"

class VergilAdditionalJJC : public Mod {
public:
  static inline bool cheaton{NULL};

  VergilAdditionalJJC() = default;
  std::string_view get_name() const override { return "VergilAdditionalJJC"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };

  inline std::optional<std::string> on_initialize() override {
    init_check_box_info();
    ischecked        = &cheaton;
    onpage           = vergilcheat;
    full_name_string = "Additional JJC in SDT";
    author_string    = "VPZadov";
    description_string = "Allow Vergil to perform an extra JJC while in SDT.";
    if (cheaton)
      VergilSetMaxJJC::cheaton = true;
    return Mod::on_initialize();
  }

  inline void on_draw_ui() override {
    ImGui::TextWrapped("This mod uses the \"Set maximum JJC in a row\" mod, and "
        "dynamically changes the max JJC value, so don't disable it when using this.");
    if (cheaton) {
      VergilSetMaxJJC::cheaton = true;
      VergilInfJdCs::cheaton   = false;
    }
  }
  // on_draw_debug_ui() is called when debug window shows up
  inline void on_draw_debug_ui() override{}

  inline void on_frame() override {//Mb I should move this check to SDTTracker, 'cause in that case this will be check and change not every frame, but only after Vergil's transforms...
    if (cheaton) {
      if (VergilSDTFormTracker::vergilform_state == 2)
        VergilSetMaxJJC::max_jjc = 4;
      else
        VergilSetMaxJJC::max_jjc = 3;
    }
  }

private:
  // function hook instance for our detour, convinient wrapper
  // around minhook
  inline void init_check_box_info() override {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
  }
  std::unique_ptr<FunctionHook> m_addjjc_insdt_hook;
};
