#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include <array>
#include <random>
#include "EnemyWaveEditor.hpp"
#include "CheckpointPos.hpp"
#include "ImGuiExtensions/ImGuiExtensions.h"
#include "MissionManager.hpp"
//#include "EnemyDataSettings.hpp"

class EnemySwapper : public Mod {


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
static uintptr_t nowFlowRet;
static uintptr_t gameModeRet;
//static uintptr_t swapIdRet;

static uintptr_t posSpawnRet;
static uintptr_t posSpawnTestJne;
static uintptr_t bossDanteAiRet;
static uintptr_t bossDanteAiJne;
static uintptr_t killShadowRet;
static uintptr_t killGriffonRet;
static uintptr_t nightmareStartingPosRet;
static uintptr_t nightmareArrivalPosRet;
static uintptr_t plPosBase;
static uintptr_t cavFixRet;
static uintptr_t vergilFixRet;
static uintptr_t vergilFixJs;
static uintptr_t airRaidControllerRet;
static uintptr_t goliathSuckJmpRet;
static uintptr_t goliathLeaveJmpRet;
static uintptr_t artemisFixRet;
static uintptr_t urizen3TpRet;
static uintptr_t urizen3TpJne;
static uintptr_t malphasRet;
static uintptr_t cerberusFixRet;
static uintptr_t cerberusThunderWaveRet;
static uintptr_t cerberusThunderBallRet;
static uintptr_t cerberusThunderBallJmp;

static bool isSwapAll;
static bool cheaton;
static bool isCustomRandomSettings;
static bool isCustomSeed;
static bool isCustomSpawnPos;
static bool isBossDanteAiEnabled;
static bool isDanteM20;
static bool canKillShadow;
static bool canKillGriffon;
static bool isNightmareFix;
static bool isInMission;//NowFlow.isOnMission
static bool isCavFixEnabled;
static bool isVergilFixEnabled;
static bool isFastDiveBombAttack;
static bool isGoliathFixEnabled;
static bool isArtemisFixEnabled;
static bool isArtemisPlayersXY;
static bool isUrizen3FixEnabled;
static bool isMalphasFixEnabled;
static bool malphasFixPlPos;
static bool isCerberusFixEnabled;
static bool cerberusFixPlPos;
static bool cerberusThunderWavePlPos;

static uint32_t selectedToSwap[enemyListCount];
static uint32_t selectedSwapAll;
static uint32_t currentEnemyId;
static uint32_t newEnemyId;
static uint32_t newEnemyId1;
static uint32_t newEnemyId2;
static uint32_t newEnemyId3;
static uint32_t newEnemyId5;
static uint32_t newEnemyId6;
static uint32_t nowFlow;//22-game, 17 - start
static uint32_t prevFlow;
static uint32_t gameMode;

static float spawnPosZOffset;
static float spawnPosXOffset;
static float spawnPosYOffset;
static float curSpawnPosZ;
static float curSpawnPosX;
static float curSpawnPosY;
static float waitTimeMin;
static float waitTimeMax;
static float odds;
static float divebombDistanceGroundCheck; // default = 5
static float divebombHeightOfArenaSide; // default = 1.5
static float divebombHeightOfOutside;// default = 8
//static float inline radiusOfArea = 15.0f;//43.0
//static float inline radiusOfRevolution = 35.0f;//80
static float cerberusThunderWaveZ;

static int enemyNum;

static std::array<EnemyId, enemyListCount> swapSettings;
static EnemyId swapForAll;

static Vector3f nightmareStartPosOffs;
static inline Vector3f cavOffset{1.5f, 2.32f, -0.8f};
static inline Vector3f artemisCenterOfFloor{-368.0f, -308.5f, -10.35f};
static inline Vector3f malphasCenterOfFloor{0.0f,0.0f,0.0f};
static inline Vector3f cerberusCenterOfFloor{0.0f, -0.1f, 0.0f};

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
	  "Qliphoth's tentacle",//19 
	  "Goliath", //20
	  "Malphas", //21
	  "Nidhogg", //22
	  "Artemis", //23
	  "Gilgamesh", //24
	  "Elder Geryon Knight",//25
	  "Cavaliere", //26
	  "Qliphoth Root Boss",//27 
	  "King Cerberus", //28
	  "Griffon (NoDie)", //29
	  "Shadow (NoDie)", //30
	  "Nightmare", //31
	  "Urizen 1",//32
      "Urizen 2",//33
      "Urizen 3",//34
      "Vergil", //35
	  "Phantom Goliath",//36
      "Phantom Artemis",//37
      "Phantom Cavaliere",//38
      //"Vergil M20",//39
      "Dante (Ai disabled by default)",//40
      //"Dante M20"//41 No:)
  };

  static std::vector<uintptr_t> setDataAddrs;
  //static std::mutex mtx;


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
  size_t reservedForReswap = 4000;
  void reserveReswapVector(size_t newSize);
  static void btn_set_plpos_to(Vector3f &to, const char* btnContent = "Set center of floor to current player position");


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
  std::unique_ptr<FunctionHook> m_spawn_pos_hook;
  std::unique_ptr<FunctionHook> m_now_flow_hook;
  std::unique_ptr<FunctionHook> m_gamemode_hook;
  std::unique_ptr<FunctionHook> m_m19check_hook;
  std::unique_ptr<FunctionHook> m_griffon_hook;
  std::unique_ptr<FunctionHook> m_shadow_hook;
  std::unique_ptr<FunctionHook> m_nightmire_starting_hook;
  std::unique_ptr<FunctionHook> m_nightmire_arrival_hook;
  std::unique_ptr<FunctionHook> m_cavfix_hook;
  std::unique_ptr<FunctionHook> m_vergilcenterfloor_hook;
  std::unique_ptr<FunctionHook> m_airraid_controller_hook;
  std::unique_ptr<FunctionHook> m_goliath_suctionjmp_hook;
  std::unique_ptr<FunctionHook> m_goliath_leavejmp_hook;
  std::unique_ptr<FunctionHook> m_artemis_centerfloor_hook;
  std::unique_ptr<FunctionHook> m_urizen3_tp_hook;
  std::unique_ptr<FunctionHook> m_malphas_tp_hook;
  std::unique_ptr<FunctionHook> m_cerberus_pos_hook;
  std::unique_ptr<FunctionHook> m_cerberus_thunderwave_hook;
  std::unique_ptr<FunctionHook> m_cerberus_thunderball_hook;
};