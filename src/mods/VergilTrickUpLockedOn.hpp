#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class VergilTrickUpLockedOn : public Mod {
public:
  VergilTrickUpLockedOn() = default;
  // mod name string for config
  std::string_view get_name() const override { return "VergilTrickUpLockedOn"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;

  static inline float leftStickAngle = 25.0f;

  static uintptr_t jmp_ret1;
  static uintptr_t jmp_jne1;

  static uintptr_t jmp_ret2;
  static uintptr_t jmp_jne2;

  static uintptr_t jmp_ret3;
  static uintptr_t jmp_je3;

  static bool cheaton;

  // Override this things if you want to store values in the config file
  
  // on_draw_debug_ui() is called when debug window shows up
  // void on_draw_debug_ui() override;

private:
  // function hook instance for our detour, convinient wrapper
  // around minhook
  void init_check_box_info() override;

  std::unique_ptr<FunctionHook> m_function_hook1;
  std::unique_ptr<FunctionHook> m_function_hook2;
  std::unique_ptr<FunctionHook> m_function_hook3;
};
