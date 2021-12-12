#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include "VergilInfSDT.hpp"
#include "InfDT.hpp"
#include "DMC3JCE.hpp"
class VergilSDTAccumulateRework : public Mod {
public:
  static bool cheaton;
  //camelCase go brrr
  static float sdtPointsToAdd;
  static float curDtValue;
  static float prevDtValue;
  static constexpr float maxSdt = 10000.0;

  static bool isNeedToAddStdPoints;
  static inline bool isConstInc = false; 

  static uintptr_t dtchange_jmp_ret;
  static uintptr_t sdtchange_jmp_ret;

  VergilSDTAccumulateRework() = default;
  std::string_view get_name() const override { return "VergilSDTAccumulateRework"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };

  // called by m_mods->init() you'd want to override this
  // Override this things if you want to store values in the config file
  void on_config_load(const utility::Config &cfg) override;
  void on_config_save(utility::Config &cfg) override;
  std::optional<std::string> on_initialize() override;

  // on_frame() is called every frame regardless whether the gui shows up.
  // void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  void on_draw_debug_ui() override;

private:
  // function hook instance for our detour, convinient wrapper
  // around minhook
  void init_check_box_info() override;
  std::unique_ptr<FunctionHook> m_dtchange_hook;
};
