#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"

enum HitVfxState { DrawAll, DamageOnly, Nothing };

class LDK : public Mod {
#define CHAR_HITS 0xD400
#define CHAR_DAMAGE 0x7798
#define SAFE_NUMBER 6

private:
  struct RegAddrBackup {
    uintptr_t rax;
    uintptr_t rbx;
    uintptr_t rcx;
    uintptr_t rdx;
    uintptr_t rsi;
    uintptr_t rdi;
    uintptr_t rbp;
    uintptr_t rsp;
    uintptr_t r8;
    uintptr_t r9;
    uintptr_t r10;
    uintptr_t r11;
    uintptr_t r12;
    uintptr_t r13;
    uintptr_t r14;
    uintptr_t r15;
  };

  void set_container_limit_all(uint32_t num);
  void set_container_limit_blood_only(uint32_t num);

public:
  LDK() = default;
  // mod name string for config
  std::string_view get_name() const override { return "LDK"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };
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
  static uintptr_t cavforcevalid_jmp_ret;
  static uintptr_t cavforcevalid_jmp_ret2;
  static uintptr_t cavforcevalid_jmp_je;
  static uintptr_t cavforcelightning1_jmp_ret;
  static uintptr_t cavforcelightning2_jmp_ret;
  static uintptr_t cavcoordinatechange_jmp_ret;
  static uintptr_t missionmanager;
  static uintptr_t enemygentype;
  static bool cheaton;

  static uintptr_t hitvfxskip_jmp;
  static uintptr_t hitvfxskip_ret;
  static uintptr_t sswords_restriction_jmp;
  static uintptr_t sswords_restriction_jmp_ret;
  static uintptr_t containernum_addr;
  static uintptr_t nopfunction1_jmp_ret2;
  static uintptr_t waitTimeJmpRet;

  static const uint32_t SPAWN_PAUSE_TIME = 4;

  static bool physics_fix_on;
  static bool hitvfx_fix_on;
  static bool pausespawn_enabled;
  static bool default_redorbsdrop_enabled;
  static bool waitTimeEnabled;

  static uint32_t number;
  static uint32_t hardlimit;
  static uint32_t softlimit;
  static uint32_t limittype;

  static float hpoflasthitobj;

  static uint32_t hardlimit_temp;
  static uint32_t container_num;
  static uint32_t container_limit_all;
  static uint32_t container_limit_damage_only;
  static uint32_t enemydeath_count;
  static float waitTime;//GenerateEnemyManager.GenerateData.WaitTime. Use for coop insted of pause spawn fix.

  static HitVfxState vfx_state;

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
  void init_check_box_info() override;

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
  /// <summary>
  /// Cavaliere miniboss
  /// </summary>
  std::unique_ptr<FunctionHook> m_cavforcevalid_hook;
  std::unique_ptr<FunctionHook> m_cavforcelightning1_hook;
  std::unique_ptr<FunctionHook> m_cavforcelightning2_hook;
  std::unique_ptr<FunctionHook> m_cavcoordinatechange_hook;

  std::unique_ptr<FunctionHook> m_hitvfxskip_hook;
  std::unique_ptr<FunctionHook> m_ssowrds_restriction_hook;
  std::unique_ptr<FunctionHook> m_wait_spawn_time_hook;

  static RegAddrBackup death_func_backup;
  static RegAddrBackup redorbdrop_backup;
};
