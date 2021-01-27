#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class LDK : public Mod {
public:
  LDK() = default;
  // mod name string for config
  std::string_view get_name() const override { return "LDK"; }
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  static uintptr_t enemynumber_jmp_ret;
  static uintptr_t capbypass_jmp_ret1;
  static uintptr_t capbypass_jmp_ret2;
  static uintptr_t capbypass_jmp_jnl;
  static uintptr_t capbypass_jmp_jle;

  static uintptr_t gethpoflasthitobject_jmp_ret;
  static uintptr_t multipledeathoptimize_jmp_ret;
  static uintptr_t multipledeathoptimize_jmp_jle;
  static uintptr_t canlasthitkill_jmp_ret;
  static uintptr_t nopfunction_jmp_ret1;
  static uintptr_t nopfunction_jmp_ret2;
  static uintptr_t nopfunction_1_call;

  static uintptr_t vergildivebomb_jmp_ret;
  static uintptr_t missionmanager;
  static bool cheaton;

  static uint32_t number;
  static uint32_t hardlimit;
  static uint32_t softlimit;
  static uint32_t limittype;

  static float hpoflasthitobj;

  // Override this things if you want to store values in the config file
  void on_config_load(const utility::Config& cfg) override;
  void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  //void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  void on_draw_debug_ui() override;

  // function hook instance for our detour, convinient wrapper 
  // around minhook
  std::unique_ptr<FunctionHook> m_enemynumber_hook;
  std::unique_ptr<FunctionHook> m_capbypass_hook1;
  std::unique_ptr<FunctionHook> m_capbypass_hook2;

  /// <summary>
  /// Optimize
  /// </summary>
  std::unique_ptr<FunctionHook> m_gethpoflasthitobject_hook;
  std::unique_ptr<FunctionHook> m_multipledeathoptimize_hook;
  std::unique_ptr<FunctionHook> m_canlasthitkill_hook;
  std::unique_ptr<FunctionHook> m_nopfunction_hook1;
  std::unique_ptr<FunctionHook> m_nopfunction_hook2;
  /// <summary>
  /// Vergil miniboss
  /// </summary>
  std::unique_ptr<FunctionHook> m_vergildivebomb_hook;
};
