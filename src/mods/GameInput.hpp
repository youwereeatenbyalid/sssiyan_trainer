#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class GameInput : public Mod {
public:
  GameInput() = default;
  // mod name string for config
  std::string_view get_name() const override { return "GameInput"; }
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
  void on_config_load(const utility::Config& cfg) override;
  void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  void on_draw_debug_ui() override;
private:

  // function hook instance for our detour, convinient wrapper 
  // around minhook
  std::unique_ptr<FunctionHook> m_validcontrol_hook;
  std::unique_ptr<FunctionHook> m_hold_hook;
  std::unique_ptr<FunctionHook> m_clearhold_hook;
  std::unique_ptr<FunctionHook> m_press_hook;
  std::unique_ptr<FunctionHook> m_clearpress_hook;
  std::unique_ptr<FunctionHook> m_release_hook;
  std::unique_ptr<FunctionHook> m_releasewhenheld_hook;
  std::unique_ptr<FunctionHook> m_clearrelease_hook;
};
