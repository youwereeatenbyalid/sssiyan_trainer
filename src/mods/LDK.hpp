#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include "Coroutine/Coroutines.hpp"

enum HitVfxState { DrawAll, DamageOnly, Nothing };

class LDK : public Mod {
#define CHAR_HITS 0xD400
#define CHAR_DAMAGE 0x7798
#define EM_DT 0xE358
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

  void update_em_limit();

  Coroutines::Coroutine<decltype(&LDK::update_em_limit), LDK*> _spawnPauseCoroutine{&LDK::update_em_limit};

  static inline LDK* _mod = nullptr;

  int _emLimitTmp = 0;

  std::mutex _pauseSpawnMtx;

public:
    LDK()
    {
        _mod = this;
        _spawnPauseCoroutine.set_delay(SPAWN_PAUSE_TIME);
    }
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
  static uintptr_t containernum_addr;
  static uintptr_t nopfunction1_jmp_ret2;
  static uintptr_t waitTimeJmpRet;
  static uintptr_t nohitlns_ret;
  static uintptr_t nohitlns_ret_je;

  static inline const float SPAWN_PAUSE_TIME = 4000.0f;

  static bool hitvfx_fix_on;
  static bool pausespawn_enabled;
  static bool waitTimeEnabled;
  static bool nohitlines_enabled;
  static bool emDtVfxSkipOn;
  static bool showOldFixes;

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
  static constexpr float waitTime = 1.0f;//GenerateEnemyManager.GenerateData.WaitTime. Use for coop insted of pause spawn fix.

  static HitVfxState vfx_state;

  // Override this things if you want to store values in the config file
  void on_config_load(const utility::Config& cfg) override;
  void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  // void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  void on_draw_debug_ui() override; // used

  // function hook instance for our detour, convinient wrapper 
  // around minhook
  void init_check_box_info() override;

  static void pause_spawn_asm();

  std::shared_ptr<Detour_t> m_enemynumber_detour;
  std::shared_ptr<Detour_t> m_capbypass_detour1;
  std::shared_ptr<Detour_t> m_capbypass_detour2;

  /// <summary>
  /// Optimize
  /// </summary>
  std::shared_ptr<Detour_t> m_gethpoflasthitobject_detour;
  std::shared_ptr<Detour_t> m_multipledeathoptimize_detour;
  std::shared_ptr<Detour_t> m_canlasthitkill_detour;
  std::shared_ptr<Detour_t> m_nopfunction_detour1;
  std::shared_ptr<Detour_t> m_nopfunction_detour2;
  /// <summary>
  /// Vergil miniboss
  /// </summary>
  std::shared_ptr<Detour_t> m_vergildivebomb_detour;
  /// <summary>
  /// Cavaliere miniboss
  /// </summary>
  std::shared_ptr<Detour_t> m_cavforcevalid_detour;
  std::shared_ptr<Detour_t> m_cavforcelightning1_detour;
  std::shared_ptr<Detour_t> m_cavforcelightning2_detour;
  std::shared_ptr<Detour_t> m_cavcoordinatechange_detour;

  std::shared_ptr<Detour_t> m_hitvfxskip_detour;
  std::shared_ptr<Detour_t> m_ssowrds_restriction_detour;
  std::shared_ptr<Detour_t> m_wait_spawn_time_detour;
  std::shared_ptr<Detour_t> m_hitvfx_dontdraw_hitlines_detour;

  static RegAddrBackup hitvfx_backup;
};
