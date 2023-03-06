#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include <array>
#include <random>
#include "ImGuiExtensions/ImGuiExtensions.h"
#include "EndLvlHooks.hpp"
#include "GameFunctions/GameFunc.hpp"
#include "EnemyData.hpp"

namespace gf = GameFunctions;

class EnemySwapper : public Mod
{
public:

    static inline bool cheaton = false;

	struct EnemyId 
    {
        const EnemyData::EnemyId currentId;//id of what swap;
        EnemyData::EnemyId swapId;//id of to swap;
    
        EnemyId(EnemyData::EnemyId thisId, EnemyData::EnemyId swapToId) : currentId(thisId), swapId(swapToId) {}
        
        EnemyId(const EnemyId& other) : currentId(other.currentId), swapId(other.swapId) {}
    };


  EnemySwapper()
  {
      _mod = this;
      for (int i = 0; i < _swapSettings.size(); i++)
      {
          _swapSettings[i] = std::make_unique<EnemyId>(EnemyData::indx_to_id(i), EnemyData::HellCaina);
          _swapSettings[i]->swapId = EnemyData::indx_to_id(i);
      }
  }

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

  void random_em_swap(uint32_t min, uint32_t max);
  void seed_rnd_gen(int seed = -1);

  static void em_swap_asm(uintptr_t emDataList)//Called in WaveEditor
  {
      std::lock_guard<std::mutex> lck(_swapMtx);
      for (int i = 0; i < gf::ListController::get_list_count(emDataList); i++)
      {
          int *emId = (int*)(gf::ListController::get_item<uintptr_t>(emDataList, i) + 0x10);
          if (!_mod->_isAllSwap)
          {
              for (const auto& i : _mod->_swapSettings)
              {
                  if (i->currentId == *emId)
                  {
                      *emId = i->swapId;
                      break;
                  }
              }
          }
          else
              *emId = EnemyData::indx_to_id(_mod->_swapAllIndx);
      }
  }

private:

    bool _isAllSwap = false;

    static constexpr inline int _enemyListCount = EnemyData::EnemyNames.size();

    std::array<std::unique_ptr<EnemyId>, _enemyListCount> _swapSettings;

    void set_swapper_settings(std::array<int, _enemyListCount>& settingsList);

    int _swapAllIndx = 0;

    bool _isCustomRandomSettings = false;
    bool _isCustomSeed = false;

    static inline std::mutex _swapMtx{};

  void restore_default_settings();
  void set_swapper_setting(int emListIndx, int swapToIndx);
  static inline std::random_device rd;
  static inline std::mt19937 gen;
  int seed    = 0;
  int prev_seed = 0;
  int curMinIndx = 0;
  int curMaxIndx = 19;
  const int minIndx = 0;
  const int maxIndx = 19;

  static inline EnemySwapper *_mod = nullptr;

  // function hook instance for our detour, convinient wrapper
  // around minhook
  void init_check_box_info() override;
};