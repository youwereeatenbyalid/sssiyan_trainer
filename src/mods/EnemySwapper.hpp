#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include <array>
#include <random>

class EnemySwapper : public Mod {

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

public:

  static inline RegAddrBackup enemySwapBackup1;
  static inline RegAddrBackup enemySwapBackup2;
  static inline RegAddrBackup enemySwapBackup3;
  static inline RegAddrBackup enemySwapBackup5;
  static inline RegAddrBackup enemySwapBackup6;
  static inline RegAddrBackup enemySwapBackup7;
  static inline RegAddrBackup swapIdBackup;
  static inline RegAddrBackup spawnPosBackup;

static const int enemyListCount = 41;

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
          if (curListIndx == 40) // Dante, but his AI is disabled
            id = 55;
	}
  };

static uintptr_t setEnemyDataRet1;
static uintptr_t setEnemyDataRet2;
static uintptr_t setEnemyDataRet3;
static uintptr_t setEnemyDataRet4;
static uintptr_t setEnemyDataRet5;
static uintptr_t setEnemyData4Jmp;
static uintptr_t setEnemyDataRet6;
static uintptr_t swapIdRet;

static uintptr_t posSpawnRet;
static uintptr_t posSpawnTestJne;

static bool isSwapAll;
static bool cheaton;
static bool isCustomRandomSettings;
static bool isCustomSeed;
static bool isCustomSpawnPos;

static uint32_t selectedToSwap[enemyListCount];
static uint32_t selectedSwapAll;
static uint32_t currentEnemyId;
static uint32_t newEnemyId;

static float spawnPosZOffset;
static float curSpawnPosZ;
static float waitTimeMin;
static float waitTimeMax;
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
  void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  void on_draw_debug_ui() override;
  // function hook instance for our detour, convinient wrapper
  // around minhook
  void init_check_box_info() override;

  inline static void set_enemy_num(int num) { EnemySwapper::enemyNum = num; }
  inline static void set_wait_time(float waitTimeMin, float waitTimeMax) {
    EnemySwapper::waitTimeMin = waitTimeMin;
    EnemySwapper::waitTimeMax = waitTimeMax;
  }
  inline static void set_spawn_pos_offset(float zOffset) {
    spawnPosZOffset = zOffset;
  }

  static void random_em_swap(uint32_t min, uint32_t max);
  static void seed_rnd_gen(int seed = -1);

  static void set_swapper_settings(std::array<int, enemyListCount> &settingsList);

  static inline std::array<const char*, enemyListCount> emNames{
	  "Hell Caina", //0
	  "Hell Antenora",//1
	  "Hell Judecca", //2
	  "Empusa",//3
	  "Green Empusa",//4
	  "Red Empusa", //5
	  "Empusa Queen",//6
	  "Riot", //7
	  "Chaos", //8
	  "Fury", //9
	  "Baphomet",//10 
	  "Lusachia", //11
	  "Behemoth", //12
	  "Nobody", //13
	  "Scudo Angelo",//14, 
	  "Proto Angelo", //15
	  "Death Scrissors",//16
	  "Hellbat", //17
	  "Pyrobat", //18
	  "Qliphot's tentacle",//19 
	  "Goliath", //20
	  "Malphas", //21
	  "Nidhogg", //22
	  "Artemis", //23
	  "Gilgamesh", //24
	  "Elder Geryon Knight",//25
	  "Cavaliere", //26
	  "Qliphot Root Boss",//27 
	  "King Cerberus", //28
	  "Griffon (NoDie)", //29
	  "Shadow (NoDie)", //30
	  "Nightmire", //31
	  "Urizen 1",//32
      "Urizen 2",//33
      "Urizen 3",//34
      "Vergil", //35
	  "Phantom Goliath",//36
      "Phantom Artemis",//37
      "Phantom Cavaliere",//38
      "Vergil M20",//39
      "Dante (AiDisabled)",//40
      //"Dante M20"//41 No:)
  };

private:
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

  
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook1;
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook2;
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook3;
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook4;
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook5;
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook6;
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook7;
  std::unique_ptr<FunctionHook> m_spawn_pos_hook;
};