#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class DoppelWeaponSwitcher : public Mod {
public:
  DoppelWeaponSwitcher() = default;
  // mod name string for config
  std::string_view get_name() const override { return "DoppelWeaponSwitcher"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  static uintptr_t updateweapon_jmp_ret;
  static uintptr_t resetweapon_jmp_ret;
  static uintptr_t fixparameter_jmp_ret;
  static uintptr_t fixjdc_jmp_ret;

  static uintptr_t jjdcaltcheck_jmp_ret;
  static uintptr_t doppeljjdc_jmp_ret;
  static uintptr_t doppeljjdc_jmp_jne;
  static uintptr_t doppelonlyjjdc_jmp_ret;
  static uintptr_t doppelonlyjjdc_jmp_call;
  static uintptr_t doppelonlyjjdcteleport_jmp_ret;
  static uintptr_t doppelonlyjjdcteleport_jmp_call;

  static uintptr_t doppelidle1_jmp_ret;
  static uintptr_t doppelidle2_jmp_ret;
  static uintptr_t doppelidle3_jmp_ret;

  static uintptr_t doppelbeowulfcharge_jmp_ret;
  static uintptr_t soundchargestart_jmp_ret;
  static uintptr_t soundchargeend_jmp_ret;
  static uintptr_t soundchargelevel1_jmp_ret;
  static uintptr_t soundchargelevel2_jmp_ret;

  static uintptr_t yamatotype;
  static uintptr_t beowulftype;
  static uintptr_t forceedgetype;
  static uintptr_t weaponresetparameter;
  static uint32_t doppelweaponid;
  static uintptr_t doppelweaponparameter;
  static bool cheaton;
  // Override this things if you want to store values in the config file
  // void on_config_load(const utility::Config& cfg) override;
  // void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  // void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  // void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  void on_draw_debug_ui() override; // used
private:

  // function hook instance for our detour, convinient wrapper 
  // around minhook
  void init_check_box_info() override;

  std::shared_ptr<Detour_t> m_updateweapon_detour;
  std::shared_ptr<Detour_t> m_resetweapon_detour;
  std::shared_ptr<Detour_t> m_fixparameter_detour;
  std::shared_ptr<Detour_t> m_fixjdc_detour;

  std::shared_ptr<Detour_t> m_jjdcaltcheck_detour;
  std::shared_ptr<Detour_t> m_doppeljjdc_detour;
  std::shared_ptr<Detour_t> m_doppelonlyjjdc_detour;
  std::shared_ptr<Detour_t> m_doppelonlyjjdcteleport_detour;

  std::shared_ptr<Detour_t> m_doppelidle1_detour;
  std::shared_ptr<Detour_t> m_doppelidle2_detour;
  std::shared_ptr<Detour_t> m_doppelidle3_detour;

  std::shared_ptr<Detour_t> m_doppelbeowulfcharge_detour;
  std::shared_ptr<Detour_t> m_soundchargestart_detour;
  std::shared_ptr<Detour_t> m_soundchargeend_detour;
  std::shared_ptr<Detour_t> m_soundchargelevel1_detour;
  std::shared_ptr<Detour_t> m_soundchargelevel2_detour;
};
