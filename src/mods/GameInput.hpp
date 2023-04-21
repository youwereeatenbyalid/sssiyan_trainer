#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class GameInput : public Mod {
public:
  GameInput() = default;
  // mod name string for config
  std::string_view get_name() const override { return "GameInput"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  static uintptr_t validcontrol_jmp_ret;
  static uintptr_t hold_jmp_ret;
  static uintptr_t clearhold_jmp_ret;
  static uintptr_t press_jmp_ret;
  static uintptr_t clearpress_jmp_ret;
  static uintptr_t release_jmp_ret;
  static uintptr_t releasewhenheld_jmp_ret;
  static uintptr_t clearrelease_jmp_ret;
  static bool cheaton;

  static uintptr_t validcontrols;

  static uintptr_t holdframes[20];
  static uintptr_t pressframes[20];
  static uintptr_t releaseframes[20];
  // Override this things if you want to store values in the config file
  // void on_config_load(const utility::Config& cfg) override;
  // void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  // void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  // void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  // void on_draw_debug_ui() override;
private:

  // function hook instance for our detour, convinient wrapper 
  // around minhook
  void init_check_box_info() override;

  std::shared_ptr<Detour_t> m_validcontrol_detour;
  std::shared_ptr<Detour_t> m_hold_detour;
  std::shared_ptr<Detour_t> m_clearhold_detour;
  std::shared_ptr<Detour_t> m_press_detour;
  std::shared_ptr<Detour_t> m_clearpress_detour;
  std::shared_ptr<Detour_t> m_release_detour;
  std::shared_ptr<Detour_t> m_releasewhenheld_detour;
  std::shared_ptr<Detour_t> m_clearrelease_detour;
};
