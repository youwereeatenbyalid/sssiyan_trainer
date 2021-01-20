#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class PlayerTracker : public Mod {
public:
  PlayerTracker() = default;
  // mod name string for config
  std::string_view get_name() const override { return "PlayerTracker"; }
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  uintptr_t static player_jmp_ret;
  uintptr_t static summon_jmp_ret;
  uintptr_t static summon_jmp_je;
  uintptr_t static incombat_jmp_ret;
  uintptr_t static sin_jmp_ret;
  uintptr_t static cos_jmp_ret;
  uintptr_t static threshhold_jmp_ret;
  uintptr_t static threshhold_jmp_jb;

  uintptr_t static playerentity;
  uint32_t static playerid;
  uintptr_t static groundedmem;
  uint32_t static isgrounded;
  uintptr_t static playertransform;
  uint32_t static playermoveid;

  uintptr_t static playerinertiax;
  uintptr_t static playerinertiay;
  uintptr_t static playerinertiaz;

  uintptr_t static neroentity;
  uintptr_t static nerotransform;
  
  uintptr_t static danteentity;
  uintptr_t static dantetransform;
  uintptr_t static danteweapon;
  
  uintptr_t static ventity;
  uintptr_t static vtransform;
  
  uintptr_t static shadowcontroller;
  uintptr_t static shadowentity;
  uintptr_t static shadowtransform;

  uintptr_t static griffoncontroller;
  uintptr_t static griffonentity;
  uintptr_t static griffontransform;

  uintptr_t static nightmarecontroller;
  uintptr_t static nightmareentity;
  uintptr_t static nightmaretransform;

  uintptr_t static vergilentity;
  uintptr_t static vergiltransform;
  
  uint32_t static incombat;
  
  float static sinvalue;
  float static cosvalue;
  bool static redirect;
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
  std::unique_ptr<FunctionHook> m_player_hook;
  std::unique_ptr<FunctionHook> m_summon_hook;
  std::unique_ptr<FunctionHook> m_incombat_hook;
  std::unique_ptr<FunctionHook> m_sin_hook;
  std::unique_ptr<FunctionHook> m_cos_hook;
  std::unique_ptr<FunctionHook> m_threshhold_hook;
};