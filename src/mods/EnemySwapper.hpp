#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include <array>
#include <random>
#include "EnemyWaveEditor.hpp"
#include "CheckpointPos.hpp"
#include "ImGuiExtensions/ImGuiExtensions.h"
#include "MissionManager.hpp"
#include "PlayerTracker.hpp"
//#include "EnemyDataSettings.hpp"
#include "GameplayStateTracker.hpp"
#include "EndLvlHooks.hpp"
#include "GameFunctions/GameFunc.hpp"
#include "EnemyData.hpp"

namespace gf = GameFunctions;

class EnemySwapper : public Mod, private EndLvlHooks::IEndLvl 
{

public:

static const int enemyListCount = 40;

	struct EnemyId {
  public:
    uint32_t curListIndx;//indx in emNames;
    uint32_t indxToSwap;
    uint32_t currentId;//id of what swap;
    uint32_t swapId;//id of to swap;
    EnemyId() {
	  curListIndx = 0;
      indxToSwap  = 0;
      currentId   = 0;
      swapId      = 0;
	}

	  EnemyId(uint32_t selectedIndx) { 
		  set_current_id(selectedIndx);
	}

    inline uint32_t get_current_id() const { return currentId; }
	inline void set_current_id(uint32_t selectedIndx) {
      set_id(selectedIndx, currentId);
	}

	inline void set_swap_id(uint32_t selectedIndx) {
          set_id(selectedIndx, swapId);
	}

    inline uint32_t get_selectedIndx() const { return curListIndx; }

	inline uint32_t get_swap_id() const { return swapId; }

      private:

		  inline void set_id(uint32_t curListIndx, uint32_t &id) {
          this->curListIndx = curListIndx;
          if (curListIndx >= 20)
            id = curListIndx + 3;
          else
            id = curListIndx;
          if (curListIndx == 39) // Dante, but his AI is disabled
            id = 55;
	}
  };

	struct EnemySetting {
    EnemyId emId;
    float waitTimeMin;
    float waitTimeMax;
    float odds;
    int enemyNum;
    bool useDefault;
  };

	static inline std::array<EnemySetting, EnemySwapper::enemyListCount> enemySettings; 


static uintptr_t setEnemyDataRet1;
static uintptr_t setEnemyDataRet2;
static uintptr_t setEnemyDataRet3;
//static uintptr_t setEnemyDataRet4;
static uintptr_t setEnemyDataRet5;
//static uintptr_t setEnemyData4Jmp;
static uintptr_t setEnemyDataRet6;


static bool isSwapAll;
static bool cheaton;
static bool isCustomRandomSettings;
static bool isCustomSeed;

static uint32_t selectedToSwap[enemyListCount];
static uint32_t selectedSwapAll;
static uint32_t currentEnemyId;
static uint32_t newEnemyId;
static uint32_t newEnemyId1;
static uint32_t newEnemyId2;
static uint32_t newEnemyId3;
static uint32_t newEnemyId5;
static uint32_t newEnemyId6;

static inline constexpr float shadow_warp_offs_z = 0.85f;

static float waitTimeMin;
static float waitTimeMax;
static float odds;

static int enemyNum;

static std::array<EnemyId, enemyListCount> swapSettings;
static EnemyId swapForAll;


  EnemySwapper() = default;

  std::string_view get_name() const override { return "EnemySwapper"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };

  std::optional<std::string> on_initialize() override;

  // Override this things if you want to store values in the config file
  void on_config_load(const utility::Config& cfg) override;
  void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  // void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  void on_draw_debug_ui() override;

  inline static void set_enemy_num(int num) { EnemySwapper::enemyNum = num; }
  inline static void set_wait_time(float waitTimeMin, float waitTimeMax) {
    EnemySwapper::waitTimeMin = waitTimeMin;
    EnemySwapper::waitTimeMax = waitTimeMax;
  }
  inline static void set_odds(float odds) { EnemySwapper::odds = odds; }
  /*inline static void set_spawn_pos_offset(float zOffset) {
    spawnPosZOffset = zOffset;
  }*/

  static void random_em_swap(uint32_t min, uint32_t max);
  static void seed_rnd_gen(int seed = -1);

  static void set_swapper_settings(std::array<int, enemyListCount> &settingsList);

  static void clear_swap_data_asm();

  void reset(EndLvlHooks::EndType end) override
  {
	  if(EnemySwapper::cheaton)
		  clear_swap_data_asm();
  }

  static std::vector<uintptr_t> swapDataAddrs;
  //static std::mutex mtx;


private:

	static inline bool isSpawnOffsForFlyingEnemiesOnly = false;
	static inline bool isForceVerticalSpawnRot = false;
	static inline bool isAlwaysSpawnOnPlPos = false;
	
    static inline const std::array<const char*, 40> *_emNames = EnemyData::get_em_names();

  void restore_default_settings();
  static void set_swapper_setting(int emListIndx, int swapToIndx);
  static inline std::random_device rd;
  static inline std::mt19937 gen;
  int seed    = 0;
  int prev_seed = 0;
  int curMinIndx = 0;
  int curMaxIndx = 19;
  const int minIndx = 0;
  const int maxIndx = 19;
  size_t reservedForReswap = 4000;
  void reserveReswapVector(size_t newSize);

  static inline EnemySwapper *_mod = nullptr;

  // function hook instance for our detour, convinient wrapper
  // around minhook
  void init_check_box_info() override;
  
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook1;
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook2;
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook3;
  //std::unique_ptr<FunctionHook> m_enemy_swapper_hook4;
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook5;
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook6;
  //std::unique_ptr<FunctionHook> m_enemy_swapper_hook7;
};