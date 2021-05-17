#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class DontHideWeaponWheel : public Mod {
public:
  DontHideWeaponWheel() = default;
  // mod name string for config
  std::string_view get_name() const override { return "DontHideWeaponWheel"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  uintptr_t static jmp_wagreturn;

  bool static cheaton;

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
  void init_check_box_info() override;

  //function hooks
  std::unique_ptr<FunctionHook> m_donthideweaponandgun_hook;

};