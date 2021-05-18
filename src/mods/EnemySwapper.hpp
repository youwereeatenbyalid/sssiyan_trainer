#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include <array>

class EnemySwapper : public Mod {
public:

static const int enemyNum = 41;

	struct EnemyId {
  public:
    uint32_t listIndx;
    uint32_t id;
    EnemyId() {}

	  EnemyId(uint32_t selectedIndx) { 
		  set_id(selectedIndx);
	}

    uint32_t get_id() const { return id; }
	void set_id(uint32_t selectedIndx) {
      listIndx = selectedIndx;
          if (selectedIndx >= 20)
        id = selectedIndx + 3;
          else
            id = selectedIndx;
          if (selectedIndx == 40)//Dante, but his AI is disabled
            id = 55;
	}
    uint32_t get_selectedIndx() const { return listIndx; }
  };

static uintptr_t setEnemyDataRet1;
static uintptr_t setEnemyDataRet2;

static bool swapAll;
static bool cheaton;

static uint32_t selectedToSwap[enemyNum];
static uint32_t selectedSwapAll;

static std::array<EnemyId, enemyNum> swapSettings;
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
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook1;
  std::unique_ptr<FunctionHook> m_enemy_swapper_hook2;

  static inline std::array<const char*, enemyNum> emNames{
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
	  "Griffon", //29
	  "Shadow", //30
	  "Nightmire", //31
	  "Urizen 1",//32
      "Urizen 2",//33
      "Urizen 3",//34
      "Vergil", //35
	  "Phantom Goliath",//36
      "Phantom Artemis",//37
      "Phantom Cavaliere",//38
      "Vergil M20",//39
      "Dante",//40
      //"Dente M20"//41 No:)
  };
};