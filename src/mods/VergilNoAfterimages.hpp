#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include "PlayerTracker.hpp"

class VergilNoAfterimages : public Mod {
public:
  static uintptr_t afterimages_jne;
  static uintptr_t afterimages_jmp_ret;

  static uint32_t vergilafterimage_state; //Disable afterimages: 0 - human, 1 - SDT, 2 - all, 3 - default;

  static bool cheaton;
  static inline bool isNoDrawIfObjHidden = false;

  VergilNoAfterimages() = default;
  std::string_view get_name() const override { return "VergilNoAfterimages"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };

 // Override this things if you want to store values in the config file
  void on_config_load(const utility::Config& cfg) override;
  void on_config_save(utility::Config& cfg) override;
  std::optional<std::string> on_initialize() override;

  // on_frame() is called every frame regardless whether the gui shows up.
  // void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  // void on_draw_debug_ui() override;

  private:

  // function hook instance for our detour, convinient wrapper
  // around minhook
  void init_check_box_info() override;
  std::shared_ptr<Detour_t> m_vergilafterimages_detour;
};
