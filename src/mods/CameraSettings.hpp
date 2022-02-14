#pragma once
#include "Mod.hpp"
#include "ImGuiExtensions/ImGuiExtensions.h"
#include "sdk/ReClass.hpp"
class CameraSettings : public Mod {
public:
  CameraSettings() = default;
  // mod name string for config
  std::string_view get_name() const override { return "CameraSettings"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  static uintptr_t jmp_retFoV;
  static uintptr_t jmp_retHorizontalSensClockwise;
  static uintptr_t jmp_retHorizontalSensAntiClockwise;

  static uintptr_t jmp_retKeyboardHorizontalEnable;
  static uintptr_t jmp_jeKeyboardHorizontalEnable;

  static uintptr_t jmp_retSiyansCamFix1;
  static uintptr_t jmp_jneSiyansCamFix1;

  static uintptr_t jmp_retCloseAutoCorrect;
  static uintptr_t jmp_jneCloseAutoCorrect;

  static uintptr_t jmp_retDistantAutoCorrect;

  static uintptr_t jmp_retMovementAutoCorrect;

  static uintptr_t jmp_retHeightAutoCorrect;

  static uintptr_t jmp_retNoVignette;

  static bool cheaton;

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

  std::unique_ptr<FunctionHook> m_function_hookFoV;
  std::unique_ptr<FunctionHook> m_function_hookHorizontalSensClockwise;
  std::unique_ptr<FunctionHook> m_function_hookHorizontalSensAntiClockwise;
  std::unique_ptr<FunctionHook> m_function_hookKeyboardHorizontalEnable;
  std::unique_ptr<FunctionHook> m_function_hookSiyansCamFix1;
  std::unique_ptr<FunctionHook> m_function_hookCloseAutoCorrect;
  std::unique_ptr<FunctionHook> m_function_hookDistantAutoCorrect;
  std::unique_ptr<FunctionHook> m_function_hookMovementAutoCorrect;
  std::unique_ptr<FunctionHook> m_function_hookHeightAutoCorrect;
  std::unique_ptr<FunctionHook> m_function_hookNoVignette;
};
