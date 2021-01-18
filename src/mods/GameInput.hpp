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
  uintptr_t static validcontrol_jmp_ret;
  uintptr_t static hold_jmp_ret;
  uintptr_t static clearhold_jmp_ret;
  uintptr_t static press_jmp_ret;
  uintptr_t static clearpress_jmp_ret;
  uintptr_t static release_jmp_ret;
  uintptr_t static releasewhenheld_jmp_ret;
  uintptr_t static clearrelease_jmp_ret;
  uintptr_t static cheaton;

  uintptr_t static validcontrols;

  uintptr_t static holdframes[20];
  uintptr_t static pressframes[20];
  uintptr_t static releaseframes[20];
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
