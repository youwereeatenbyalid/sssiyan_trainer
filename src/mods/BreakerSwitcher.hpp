#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include "ImGuiExtensions/ImGuiExtensions.h"
class BreakerSwitcher : public Mod {
public:
  BreakerSwitcher() = default;
  // mod name string for config
  std::string_view get_name() const override { return "BreakerSwitcher"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  static uintptr_t breakersize_jmp_ret;
  static uintptr_t nextbreaker_jmp_ret;
  uintptr_t static breakercontrol_jmp_ret;
  uintptr_t static jmp_bringer_ret;
  uintptr_t static jne_bringer_ret;
  uintptr_t static jmp_uireturn;
  uintptr_t static jne_uireturn;
  static uintptr_t call_nero_creategauntlet;

  uintptr_t static breaker_busy;
  uintptr_t static buffered_breaker_input;
  uintptr_t static primary_breaker;
  uintptr_t static do_ui_update;
  uintptr_t static secondary_breaker;
  uintptr_t static switcher_mode;
  uint32_t static breakaway_type;
  uintptr_t static breakaway_button;
  static bool force_primary_input;

  static bool fasterbreaker;
  static bool instantkeyboardbreakers;
  static bool force_keyboard_input;
  static bool cheaton;
  static bool use_secondary;
  static bool infinite_breakers;
  static uint32_t breakers[8];
  static uint32_t nextbreaker;
  void static breakerpress_detour(); 
  // Override this things if you want to store values in the config file
  void on_config_load(const utility::Config& cfg) override;
  void on_config_save(utility::Config& cfg) override;

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

  std::unique_ptr<FunctionHook> m_breakersize_hook;
  std::unique_ptr<FunctionHook> m_nextbreaker_hook;
  std::unique_ptr<FunctionHook> m_breakerinputcontrol_hook;
  std::unique_ptr<FunctionHook> m_NeroUIOverride_hook;
  std::unique_ptr<FunctionHook> m_bringerinputcontroller_hook;
};