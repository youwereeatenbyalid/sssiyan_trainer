#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class BreakerSwitcher : public Mod {
public:
  BreakerSwitcher() = default;
  // mod name string for config
  std::string_view get_name() const override { return "BreakerSwitcher"; }
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  uintptr_t static breakersize_jmp_ret;
  uintptr_t static nextbreaker_jmp_ret;
  uintptr_t static breakerui_jmp_ret;
  uintptr_t static cheaton;

  uint32_t static breakers[8];
  uint32_t static nextbreaker;
  void static breakerpress_detour(); 
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
  std::unique_ptr<FunctionHook> m_breakersize_hook;
  std::unique_ptr<FunctionHook> m_nextbreaker_hook;
  std::unique_ptr<FunctionHook> m_breakerui_hook;
};