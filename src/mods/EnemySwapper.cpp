#include "EnemySwapper.hpp"
#include <string>
#include "sdk/ReClass.hpp"
#include "EnemySpawner.hpp"
#include "BossVergilMoves.hpp"

static std::string uniqStr = "";
std::string uniqComboStr = "";//For comboboxes

int index = 0;
int _selectedCbIndex = 0;

void EnemySwapper::set_swapper_setting(int emListIndx, int swapToIndx) {
    _swapSettings[emListIndx]->swapId = EnemyData::indx_to_id(swapToIndx);
}

void EnemySwapper::set_swapper_settings(std::array<int ,_enemyListCount> &swapToIndx)
{
  for (int i = 0; i < swapToIndx.size(); i++)
    set_swapper_setting(i, swapToIndx[i]);
}


void EnemySwapper::on_config_load(const utility::Config& cfg) {
  _isAllSwap          = cfg.get<bool>("EnemySwapper.isSwapAll").value_or(false);
  _swapAllIndx = (EnemyData::EnemyId)cfg.get<int>("SwapAllEnemiesToID").value_or(EnemyData::HellCaina);
  std::string key;
  uint32_t swapTo = 0;
  for (int i = 0; i < EnemyData::EnemyNames.size(); i++) {
    key = std::string(EnemyData::EnemyNames[i]) + "_swapTo";
    swapTo = cfg.get<int>(key).value_or(i);
    auto it = std::find_if(_swapSettings.begin(), _swapSettings.end(), [&](const std::unique_ptr<EnemyId>& obj) { return obj->currentId == EnemyData::indx_to_id(i); });
    if (it != _swapSettings.end())
        it->get()->swapId = (EnemyData::EnemyId)swapTo;
  }
}

void EnemySwapper::on_config_save(utility::Config& cfg) {
  cfg.set<int>("SwapAllEnemiesToID", (int)_swapAllIndx);
  cfg.set<bool>("EnemySwapper.isSwapAll", _isAllSwap);

  for (int i = 0; i < _swapSettings.size(); i++) {
      cfg.set<uint32_t>(std::string(EnemyData::EnemyNames[i]) + "_swapTo", _swapSettings[i]->swapId);
  }
}

// void EnemySwapper::on_frame() {}

void EnemySwapper::restore_default_settings() {
  for (int i = 0; i < EnemySwapper::_enemyListCount; i++)
    set_swapper_setting(i, i);
  EnemySwapper::curMinIndx = minIndx;
  EnemySwapper::curMaxIndx = maxIndx;
  seed       = -1;
  index = 0;
}

void print_issues(const char* str) {
    ImGui::TextWrapped(str);
  ImGui::Spacing();
}

void EnemySwapper::on_draw_ui() {
  if (ImGui::CollapsingHeader("Current Issues")){
    print_issues("Killing enemies swapped with a boss in mission can cause BGM issues.");
    print_issues("Dante AI fix doesn't work on most of BP stages.");
    print_issues("Bp stages will not end with Dante boss. Same with Griffon and Shadow.");
    print_issues("Some of battle arenas may be softlocked with boss Dante.");
    print_issues("Swapping the Qliphoth root boss can softlock mission 1.");
    print_issues("Swapping Dante can softlock mission 20.");
    print_issues("Wrong swap on some BP stages sometimes.");
    ImGui::Separator();
  }

  ImGui::Checkbox("Swap all enemies", &_isAllSwap);
  if (_isAllSwap) {
    ImGui::TextWrapped("Swap to:");
    ImGui::SameLine();
    ImGui::Combo("##Swap to", (int*)&_swapAllIndx, EnemyData::EnemyNames.data(), EnemyData::EnemyNames.size(),20);
  } 
  else {

    ImGui::Columns(4, NULL, false);
    for (int i = 0; i < EnemyData::EnemyNames.size(); i++) {
        if (i == 32 || i == 33)//Urizens
            continue;

        bool state = EnemyData::id_to_indx(_swapSettings[i]->swapId) != i;
        ImVec4 backgroundcolor = state ? ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered) : SELECTABLE_STYLE_ACT;
        if (i == index)
            backgroundcolor = SELECTABLE_STYLE_HVR;
        ImGui::PushStyleColor(ImGuiCol_Header, backgroundcolor);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, SELECTABLE_STYLE_ACT_HVR);

        
        if (state) {
            uniqStr = std::string(EnemyData::EnemyNames[i] + std::string(" -> ") + EnemyData::EnemyNames[EnemyData::id_to_indx(_swapSettings[i]->swapId)]);
        }
        else {
            uniqStr = EnemyData::EnemyNames[i];
        }

        if (ImGui::Selectable(uniqStr.c_str(), state, ImGuiSelectableFlags_AllowDoubleClick))
            _swapSettings[i]->swapId = EnemyData::indx_to_id(_selectedCbIndex);
        else
            index = i;
        ImGui::PopStyleColor(2);
        ImGui::NextColumn();
    }
    if (ImGui::Button("Restore default settings"))
        restore_default_settings();

    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::Separator();

    ImGui::TextWrapped(EnemyData::EnemyNames[index]);
    uniqComboStr = "##SwapToCustom";
    ImGui::Combo(uniqComboStr.c_str(), &_selectedCbIndex, EnemyData::EnemyNames.data(), EnemyData::EnemyNames.size(), 20);
    ImGui::Spacing();
    ImGui::Separator();

    
    if (ImGui::Button("Randomize enemies", ImVec2(165, 25))) {
        random_em_swap(curMinIndx, curMaxIndx);
    }

    ImGui::Checkbox("Change randomizer settings", &_isCustomRandomSettings);
    if (_isCustomRandomSettings) {
      ImGui::Checkbox("Use custom seed", &_isCustomSeed);
      if (_isCustomSeed) {
        ImGui::Text("Seed:");
        if (ImGui::InputInt("##seedInput", &seed))
          seed_rnd_gen(seed);
      } else
        seed = -1;
      ImGui::TextWrapped("Controls the pool of enemies that can be selected from when randomizing");
      ImGui::Columns(2, NULL, false);
      ImGui::TextWrapped("Min enemy index:");
      UI::SliderInt("##minIndxSlider", &curMinIndx, 0, 39, "%d", 1.0F, ImGuiSliderFlags_AlwaysClamp);
      ImGui::NextColumn();
      ImGui::TextWrapped("Max enemy index:");
      UI::SliderInt("##maxIndxSlider", &curMaxIndx, 1, 40, "%d", 1.0F, ImGuiSliderFlags_AlwaysClamp);
      ImGui::Columns(1);
      if (curMinIndx >= curMaxIndx)
        curMinIndx = curMaxIndx - 1;
    } else {
      curMinIndx = minIndx;
      curMaxIndx = maxIndx;
    }

    
  }
  
}

void EnemySwapper::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

void EnemySwapper::seed_rnd_gen(int seed) {
  if (seed == -1)
    gen.seed(rd());
  else
    gen.seed(seed);
}

void EnemySwapper::random_em_swap(uint32_t min, uint32_t max) {
  std::uniform_int_distribution<> distrib(min, max);
  for (int i = 0; i < 20; i++) {//Rand only for regular enemies
    set_swapper_setting(i, distrib(gen));
  }
}

std::optional<std::string> EnemySwapper::on_initialize() {
  init_check_box_info();
  auto base      = g_framework->get_module().as<HMODULE>(); // note HMODULE

  m_is_enabled = &cheaton;
  m_on_page         = Page_Encounters;
  m_full_name_string   = "Enemy Swapper (+)";
  m_author_string      = "V.P.Zadov";
  m_description_string = "Swap enemy spawns. Affects normal spawns & hell judecca summons.";

  //uintptr_t swapIdAddr = g_framework->get_module().as<uintptr_t>() + 0xF34F6A;

  seed_rnd_gen(-1);
  //EnemySwapper::swapDataAddrs.reserve(reservedForReswap);
  //swapDataAddrs = new std::vector<uintptr_t>();
    return Mod::on_initialize();
}