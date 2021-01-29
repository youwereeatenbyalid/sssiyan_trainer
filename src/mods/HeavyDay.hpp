#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class HeavyDay : public Mod {
public:
  HeavyDay() = default;
  // mod name string for config
  std::string_view get_name() const override { return "HeavyDay"; }
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  static uintptr_t enemystep_jmp_ret;
  static uintptr_t enemystep_je_ret;
  static uintptr_t lockon_jmp_ret;
  static uintptr_t targetswitch_jmp_ret;
  static uintptr_t damageall_jmp_ret;
  static uintptr_t pvp1_jmp_ret;
  static uintptr_t pvp2_jmp_ret;
  static uintptr_t danteclientside_jmp_ret;
  static uintptr_t dtenable_jmp_ret;
  static uintptr_t dtenable_ja_ret;
  static uintptr_t rgenable_jmp_ret;
  static uintptr_t rgmod_jmp_ret;
  static uintptr_t combatmode_jmp_ret;
  static uintptr_t dantefix_jmp_ret;
  static bool cheaton;

  // Override this things if you want to store values in the config file
  //void on_config_load(const utility::Config& cfg) override;
  //void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  //void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  //void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  //void on_draw_debug_ui() override;
private:

  // function hook instance for our detour, convinient wrapper 
  // around minhook
  std::unique_ptr<FunctionHook> m_enemystep_hook;
  std::unique_ptr<FunctionHook> m_lockon_hook;
  std::unique_ptr<FunctionHook> m_targetswitch_hook;
  std::unique_ptr<FunctionHook> m_damageall_hook;
  std::unique_ptr<FunctionHook> m_pvp1_hook;
  std::unique_ptr<FunctionHook> m_pvp2_hook;
  std::unique_ptr<FunctionHook> m_danteclientside_hook;
  std::unique_ptr<FunctionHook> m_dtenable_hook;
  std::unique_ptr<FunctionHook> m_rgenable_hook;
  std::unique_ptr<FunctionHook> m_rgmod_hook;
  std::unique_ptr<FunctionHook> m_combatmode_hook;
  std::unique_ptr<FunctionHook> m_dantefix_hook;

};