#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class DoppelWeaponSwitcher : public Mod {
public:
  DoppelWeaponSwitcher() = default;
  // mod name string for config
  std::string_view get_name() const override { return "DoppelWeaponSwitcher"; }
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  uintptr_t static updateweapon_jmp_ret;
  uintptr_t static resetweapon_jmp_ret;
  uintptr_t static fixparameter_jmp_ret;
  uintptr_t static fixjdc_jmp_ret;

  uintptr_t static jjdcaltcheck_jmp_ret;
  uintptr_t static doppeljjdc_jmp_ret;
  uintptr_t static doppeljjdc_jmp_jne;
  uintptr_t static doppelonlyjjdc_jmp_ret;
  uintptr_t static doppelonlyjjdc_jmp_call;
  uintptr_t static doppelonlyjjdcteleport_jmp_ret;
  uintptr_t static doppelonlyjjdcteleport_jmp_call;

  uintptr_t static doppelidle1_jmp_ret;
  uintptr_t static doppelidle2_jmp_ret;
  uintptr_t static doppelidle3_jmp_ret;

  uintptr_t static doppelbeowulfcharge_jmp_ret;
  uintptr_t static soundchargestart_jmp_ret;
  uintptr_t static soundchargeend_jmp_ret;
  uintptr_t static soundchargelevel1_jmp_ret;
  uintptr_t static soundchargelevel2_jmp_ret;

  uintptr_t static yamatotype;
  uintptr_t static beowulftype;
  uintptr_t static forceedgetype;
  uintptr_t static weaponresetparameter;
  uint32_t static doppelweaponid;
  uintptr_t static doppelweaponparameter;
  static bool cheaton;
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
  std::unique_ptr<FunctionHook> m_updateweapon_hook;
  std::unique_ptr<FunctionHook> m_resetweapon_hook;
  std::unique_ptr<FunctionHook> m_fixparameter_hook;
  std::unique_ptr<FunctionHook> m_fixjdc_hook;

  std::unique_ptr<FunctionHook> m_jjdcaltcheck_hook;
  std::unique_ptr<FunctionHook> m_doppeljjdc_hook;
  std::unique_ptr<FunctionHook> m_doppelonlyjjdc_hook;
  std::unique_ptr<FunctionHook> m_doppelonlyjjdcteleport_hook;

  std::unique_ptr<FunctionHook> m_doppelidle1_hook;
  std::unique_ptr<FunctionHook> m_doppelidle2_hook;
  std::unique_ptr<FunctionHook> m_doppelidle3_hook;

  std::unique_ptr<FunctionHook> m_doppelbeowulfcharge_hook;
  std::unique_ptr<FunctionHook> m_soundchargestart_hook;
  std::unique_ptr<FunctionHook> m_soundchargeend_hook;
  std::unique_ptr<FunctionHook> m_soundchargelevel1_hook;
  std::unique_ptr<FunctionHook> m_soundchargelevel2_hook;
};
