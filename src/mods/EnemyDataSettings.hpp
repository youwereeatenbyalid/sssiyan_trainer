#pragma once
#include "Mod.hpp"
#include "mods/EnemySwapper.hpp"

#define SELECTABLE_STYLE_ACT	ImVec4(0.26f, 0.39f, 0.58f, 0.41f)
#define SELECTABLE_STYLE_HVR	ImVec4(0.26f, 0.59f, 0.98f, 0.61f)
#define SELECTABLE_STYLE_ACT_HVR	ImVec4(0.26f, 0.59f, 0.98f, 0.91f) 
class EnemyDataSettings : public Mod
{
public:

	struct EnemySetting 
	{
		EnemyData::EnemyId emId;
		float waitTimeMin;
		float waitTimeMax;
		float odds;
		int enemyNum;
		bool useDefault;
	};

	static inline bool cheaton = false;
	static inline bool showReadme = false;
	static inline bool shareSettings = true;
	static inline float waitTimeMin = 0.0f;
	static inline float waitTimeMax = 0.0f;
	static inline int enemyNum = 4;
	static inline float odds = 100.0f;
	int index = 0;
	//static inline bool isBp = false;

	static inline void set_enemy_num(int num) { enemyNum = num; }
	//static inline std::array<int, EnemySwapper::enemyListCount> enemiesNum;

	 std::string_view get_name() const override { return "EnemyDataSetting"; }
     std::string get_checkbox_name() override { return m_check_box_name; };
     std::string get_hotkey_name() override { return m_hot_key_name; };

	inline std::optional<std::string> on_initialize() override {
		init_check_box_info();
		m_is_enabled = &cheaton;
		m_on_page = Page_Encounters;
		m_full_name_string = "Increased Enemy Spawns (+)";
		m_author_string = "V.P.Zadov";
		m_description_string = "Increase the number of enemies that spawn and adjust the time between spawns.";

		for (int i = 0; i < enemySettings.size(); i++) {
			enemySettings[i].emId = EnemyData::indx_to_id(i);
		}

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	inline void on_config_load(const utility::Config& cfg) override {
		std::string key;
        float oddsLd, timeMinLd, timeMaxLd;
		int emNumLd;
		oddsLd = cfg.get<float>("EnemyDataSettings_OddsAll").value_or(100.0f);
        timeMinLd = cfg.get<float>("EnemyDataSettings_waitTimeMinAll").value_or(0.0f);
        timeMaxLd = cfg.get<float>("EnemyDataSettings_waitTimeMaxAll").value_or(0.0f);
        emNumLd = cfg.get<int>("EnemyDataSettings_enemyNumAll").value_or(1);
        shareSettings = cfg.get<bool>("EnemyDataSettings_shareSettings").value_or(true);
        set_all_data_settings(timeMinLd, timeMaxLd, emNumLd, oddsLd);
		for (int i = 0; i <EnemyData::EnemyNames.size(); i++) {
			key = std::string(EnemyData::EnemyNames[i]) + "_" + "waitTimeMin";
			enemySettings[i].waitTimeMin = cfg.get<float>(key).value_or(0.0f);
			key = std::string(EnemyData::EnemyNames[i]) + "_" + "waitTimeMax";
			enemySettings[i].waitTimeMax = cfg.get<float>(key).value_or(0.0f);
			key = std::string(EnemyData::EnemyNames[i]) + "_" + "enemyNum";
			enemySettings[i].enemyNum = cfg.get<int>(key).value_or(1);
			key = std::string(EnemyData::EnemyNames[i]) + "_" + "useDefault";
			enemySettings[i].useDefault = cfg.get<bool>(key).value_or(true);
			key = std::string(EnemyData::EnemyNames[i]) + "_" + "odds";
			enemySettings[i].odds = cfg.get<float>(key).value_or(100.0f);
		}
	}

	inline void on_config_save(utility::Config& cfg) override {
		std::string key;
		cfg.set<float>("EnemyDataSettings_OddsAll", odds);
		cfg.set<float>("EnemyDataSettings_waitTimeMinAll", waitTimeMin);
		cfg.set<float>("EnemyDataSettings_waitTimeMaxAll", waitTimeMax);
		cfg.set<int>("EnemyDataSettings_enemyNumAll", enemyNum);
		cfg.set<bool>("EnemyDataSettings_shareSettings", shareSettings);

		for (int i = 0; i <EnemyData::EnemyNames.size(); i++) {
			if(i == 32 || i == 33)
				continue;
			key = std::string(EnemyData::EnemyNames[i]) + "_" + "waitTimeMin";
			cfg.set<float>(key, enemySettings[i].waitTimeMin);
			key = std::string(EnemyData::EnemyNames[i]) + "_" + "waitTimeMax";
			cfg.set<float>(key, enemySettings[i].waitTimeMax);
			key = std::string(EnemyData::EnemyNames[i]) + "_" + "enemyNum";
			cfg.set<int>(key, enemySettings[i].enemyNum);
			key = std::string(EnemyData::EnemyNames[i]) + "_" + "useDefault";
			cfg.set<bool>(key, enemySettings[i].useDefault);
			key = std::string(EnemyData::EnemyNames[i]) + "_" + "odds";
			cfg.set<float>(key, enemySettings[i].odds);

		}
	}

	// on_frame() is called every frame regardless whether the gui shows up.
	inline void on_frame() override{}
	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	inline void on_draw_ui() override {
		if (ImGui::CollapsingHeader("Current Issues")) {
			ImGui::TextWrapped("The game has a hard limit on the number of enemies that can be present at the same time, which can be removed by enabling LDK.\n"
			"This mod can cause problems when editing the data for bosses& when setting the enemy multiplier to 0 on certain encounters throughout the campaign & bloody palace.");
		}
        ImGui::Checkbox("Share settings for all enemies", &shareSettings);
		if (shareSettings) {
		  ImGui::TextWrapped("Enemy number: changes the amount of enemies produced at each spawn point. Setting it to 0 removes all enemies at that spawn point.");
		  ImGui::InputInt("##enemyNumShared", &enemyNum, 1);
		  ImGui::Spacing();
		  ImGui::TextWrapped("Minimum spawn time & Maximum spawn time: Adjust in-engine settings controlling the delay between enemy spawns.");
		  ImGui::Columns(2, NULL, false);
          ImGui::TextWrapped("Wait time mininum");
		  ImGui::Spacing();
          ImGui::InputFloat("##WaitTimeMinShared", &waitTimeMin);
		  ImGui::NextColumn();
		  ImGui::TextWrapped("Wait time maximum");
		  ImGui::Spacing();
          ImGui::InputFloat("##WaitTimeMaxShared", &waitTimeMax);
		  ImGui::Columns(1);
		  //ImGui::TextWrapped("Spawn chance (%%): Spawn chance affects the odds an enemy spawns from a spawn point.This is 100 % by default in - game, and can lead to softlocks if used.");
          //ImGui::SliderFloat("##enemyOddsShared", &odds, 0.0f, 100.0f, "%.1f");
          validate_values(shareSettings);
          set_all_data_settings(waitTimeMin, waitTimeMax, enemyNum, odds);
		}
		else {
			/*ImGui::Text("Check if you want use custom settings for each enemies in BP (same shit like with swapper)");
			ImGui::Checkbox("Tick this if you go to BP", &isBp);
			ImGui::Separator();*/
			//ImGui::Text("index %d", index);
			ImGui::Columns(4, NULL, false);
			for (int i = 0; i <EnemyData::EnemyNames.size(); i++) {
				if (i == 32 || i == 33)//Urizens
					continue;

				bool state = !enemySettings[i].useDefault;
				ImVec4 backgroundcolor = state ? ImGui::GetStyleColorVec4(ImGuiCol_HeaderHovered): SELECTABLE_STYLE_ACT;
				if (i == index)
					backgroundcolor = SELECTABLE_STYLE_HVR;
				ImGui::PushStyleColor(ImGuiCol_Header, backgroundcolor);
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, SELECTABLE_STYLE_ACT_HVR);
				
				
					if (state) {
						uniqStr = std::string(EnemyData::EnemyNames[i] + std::string(" x") + std::to_string(enemySettings[i].enemyNum));
					}
					else {
						uniqStr = EnemyData::EnemyNames[i];
					}

				if (ImGui::Selectable(uniqStr.c_str(), state, ImGuiSelectableFlags_AllowDoubleClick)) {
					if (ImGui::IsMouseDoubleClicked(0)) {
						enemySettings[i].useDefault = !enemySettings[i].useDefault;
					}
					else {
						index = i;
					}
				}
				ImGui::PopStyleColor(2);
				ImGui::NextColumn();
			}
			if (ImGui::Button("Restore default settings")) {
				for (int i = 0; i <EnemyData::EnemyNames.size(); i++) {
					set_enemy_data_settings(i, 0.0f, 0.0f, 1, true);
				}
			}
			ImGui::Columns(1);
			ImGui::Spacing();
			ImGui::Separator();
			uniqStr = std::string("Use default game settings ##" + std::to_string(index));
			ImGui::TextWrapped(EnemyData::EnemyNames[index]);
			ImGui::Checkbox(uniqStr.c_str(),&enemySettings[index].useDefault);
			if (!enemySettings[index].useDefault) {
				ImGui::TextWrapped("Enemy multiplier: multiplies the number of enemies produced at each spawn point. Setting it to 0 prevents enemies from spawning.");
				uniqStr = std::string("##EnemyNum" + std::to_string(index));
				ImGui::InputInt(uniqStr.c_str(), &enemySettings[index].enemyNum);
				ImGui::TextWrapped("Minimum spawn & Maximum spawn times adjust in-engine settings controlling the delay between enemy spawns.");
				ImGui::Columns(2, NULL, false);
				ImGui::TextWrapped("Wait time min: ");

				uniqStr = std::string("##WaitTimeMin" + std::to_string(index));
				ImGui::InputFloat(uniqStr.c_str(), &enemySettings[index].waitTimeMin, 0.1F);
				ImGui::NextColumn();
				ImGui::TextWrapped("Wait time max: ");
				uniqStr = std::string("##WaitTimeMax" + std::to_string(index));
				ImGui::InputFloat(uniqStr.c_str(), &enemySettings[index].waitTimeMax, 0.1f);
				ImGui::Columns(1);
				//ImGui::TextWrapped("Spawn chance (%): Spawn chance affects the odds an enemy spawns from a spawn point.This is 100 % by default in - game, and can lead to softlocks if used.");
				//uniqStr = std::string("##Odds" + std::to_string(index));
				//ImGui::SliderFloat(uniqStr.c_str(), &enemySettings[index].odds, 0.1f, 100.0f, "%.01f");
				validate_values(shareSettings, &enemySettings[index]);
			}
		}
	}

	void set_enemy_data_settings(int emIndx, float waitTimeMin, float waitTimeMax, int enemyNum, bool useDefault = false) {
		enemySettings[emIndx].waitTimeMin = waitTimeMin;
		enemySettings[emIndx].waitTimeMax = waitTimeMax;
		enemySettings[emIndx].enemyNum = enemyNum;
		enemySettings[emIndx].useDefault = useDefault;

	}

	inline static void set_all_data_settings(float waitTimeMin, float waitTimeMax, int enemyNum, float odds = 100.0f) {
		EnemyDataSettings::odds = odds;
		EnemyDataSettings::waitTimeMin = waitTimeMin;
		EnemyDataSettings::waitTimeMax = waitTimeMax;
		EnemyDataSettings::enemyNum = enemyNum;
	}

	static void change_data_asm(uintptr_t emDataList)//called in WaveEditor
	{
		std::lock_guard<std::mutex> lck(_setupWavesMtx);
		for (int i = 0; i < gf::ListController::get_list_count(emDataList); i++)
		{
			auto item = gf::ListController::get_item<uintptr_t>(emDataList, i);
			if (item == 0)
				return;
			int emId = *(int*)(item + 0x10);
			float* waitTimeMin = (float*)(item + 0x2C);
			float* waitTimeMax = (float*)(item + 0x30);
			float *odds = (float*)(item + 0x24);
			int *emNum = (int*)(item + 0x20);
			if (!shareSettings)
			{
				for (const auto& i : enemySettings)
				{
					if (!i.useDefault && i.emId == emId)
					{
						*waitTimeMin = i.waitTimeMin;
						*waitTimeMax = i.waitTimeMax;
						*emNum = i.enemyNum;
						*odds = i.odds;
						break;
					}
				}
			}
			else
			{
				*waitTimeMin = EnemyDataSettings::waitTimeMin;
				*waitTimeMax = EnemyDataSettings::waitTimeMax;
				*emNum = EnemyDataSettings::enemyNum;
				*odds = EnemyDataSettings::odds;
			}
		}
	}

private:
	static inline std::mutex _setupWavesMtx{};

	static inline std::array<EnemySetting, EnemyData::EnemyNames.size()> enemySettings;

	inline void init_check_box_info() override {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
  }
	static inline std::string uniqStr = "";
	inline void validate_values(bool isSharedSettings, EnemySetting *emSetting = nullptr) {
        if (isSharedSettings) {
            if (waitTimeMin < 0.0f)
              waitTimeMin = 0.0f;
            if (waitTimeMax < 0.0f)
              waitTimeMax = 0.0f;
            if (enemyNum < 0)
              enemyNum = 0;
            if (odds < 0.0f)
              odds = 1.0f;
        } else {
          if (emSetting->waitTimeMin < 0.0f)
            emSetting->waitTimeMin = 0.0f;
          if (emSetting->waitTimeMax < 0.0f)
            emSetting->waitTimeMax = 0.0f;
          if (emSetting->enemyNum < 0)
            emSetting->enemyNum = 0;
          if (emSetting->odds < 0.0f)
            emSetting->odds = 1.0f;
        }
	}
    std::unique_ptr<FunctionHook> m_enemy_data_settings_hook;
};

