#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include "mods/GameFunctions/GameFunc.hpp"
class PlayerTracker : public Mod {
public:
  PlayerTracker() = default;
  // mod name string for config
  std::string_view get_name() const override { return "PlayerTracker"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  static uintptr_t player_jmp_ret;
  static uintptr_t summon_jmp_ret;
  static uintptr_t summon_jmp_je;
  static uintptr_t incombat_jmp_ret;
  static uintptr_t sin_jmp_ret;
  static uintptr_t cos_jmp_ret;
  static uintptr_t threshhold_jmp_ret;
  static uintptr_t threshhold_jmp_jb;
  static uintptr_t vergildata_jmp_ret;

  static uintptr_t playerentity;
  static uint32_t playerid;
  static uintptr_t groundedmem;
  static uint32_t isgrounded;
  static uintptr_t playertransform;
  static uint32_t playermoveid;

  static uintptr_t playerinertiax;
  static uintptr_t playerinertiay;
  static uintptr_t playerinertiaz;

  static uintptr_t neroentity;
  static uintptr_t nerotransform;
  
  static uintptr_t danteentity;
  static uintptr_t dantetransform;
  static uintptr_t danteweapon;
  
  static uintptr_t ventity;
  static uintptr_t vtransform;
  
  static uintptr_t shadowcontroller;
  static uintptr_t shadowentity;
  static uintptr_t shadowtransform;

  static uintptr_t griffoncontroller;
  static uintptr_t griffonentity;
  static uintptr_t griffontransform;

  static uintptr_t nightmarecontroller;
  static uintptr_t nightmareentity;
  static uintptr_t nightmaretransform;

  static uintptr_t vergilentity;
  static uintptr_t vergiltransform;

  static uintptr_t yamatomodel;
  static uintptr_t yamatocommonparameter;
  static uintptr_t forceedgemodel;
  static uintptr_t beowulfmodel;

  static uintptr_t doppelentity;
  static uintptr_t doppeltransform;
  static uintptr_t doppelweaponmodel;

  static uintptr_t playermanager;

  static uint32_t incombat;
  static uint32_t motionID;
  static uint32_t motionBank;
  static float motionFrame;
  static bool ingameplay;
  static float sinvalue;
  static float cosvalue;
  static bool redirect;
  // Override this things if you want to store values in the config file
  // void on_config_load(const utility::Config& cfg) override;
  // void on_config_save(utility::Config& cfg) override;

  //on_frame() is called every frame regardless whether the gui shows up.
  void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  // void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  void on_draw_debug_ui() override; // used
private:

  // function hook instance for our detour, convinient wrapper 
  // around minhook
  void init_check_box_info() override;

  std::unique_ptr<FunctionHook> m_player_hook;
  std::unique_ptr<FunctionHook> m_summon_hook;
  std::unique_ptr<FunctionHook> m_incombat_hook;
  std::unique_ptr<FunctionHook> m_sin_hook;
  std::unique_ptr<FunctionHook> m_cos_hook;
  std::unique_ptr<FunctionHook> m_threshhold_hook;
  std::unique_ptr<FunctionHook> m_vergildata_hook;
};