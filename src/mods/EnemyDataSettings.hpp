#pragma once
#include "Mod.hpp"
#include "mods/EnemySwapper.hpp"
class EnemyDataSettings : public Mod
{
public:
	static inline bool cheaton = false;
	static inline bool showReadme = false;
	static inline bool shareSettings = true;
	static inline float waitTimeMin = 0.0f;
	static inline float waitTimeMax = 0.0f;
	static inline int enemyNum = 4;
	static inline float odds = 100.0f;
	//static inline bool isBp = false;

	static inline void set_enemy_num(int num) { enemyNum = num; }
	//static inline std::array<int, EnemySwapper::enemyListCount> enemiesNum;

	 std::string_view get_name() const override { return "EnemyDataSetting"; }
     std::string get_checkbox_name() override { return m_check_box_name; };
     std::string get_hotkey_name() override { return m_hot_key_name; };

	inline std::optional<std::string> on_initialize() override {
		init_check_box_info();
		auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
		ischecked = &cheaton;
		onpage = balance;
		full_name_string = "Enemy data settings (+)";
		author_string = "VPZadov";
		description_string = "Change enemy num in waves, delay for spawn and spawn odds.";

		for (int i = 0; i < EnemySwapper::enemySettings.size(); i++) {
			EnemySwapper::enemySettings[i].emId.set_current_id(i);
		}

		return Mod::on_initialize();
	}

	/*struct EnemySetting {
          int listIndx = 0;
          float waitTimeMin = 0;
          float waitTimeMax = 0;
          int enemyNum = 1;
          bool useDefault   = true;
	};

	static inline std::array<EnemySetting, EnemySwapper::enemyListCount> enemySettings; */

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
		for (int i = 0; i < EnemySwapper::emNames.size(); i++) {
			if(i == 32 || i == 33 || i == 35 || i == 39 || i == 40)//Vergils and Dante
				continue;
			key = std::string(EnemySwapper::emNames[i]) + "_" + "waitTimeMin";
			EnemySwapper::enemySettings[i].waitTimeMin = cfg.get<float>(key).value_or(0.0f);
			key = std::string(EnemySwapper::emNames[i]) + "_" + "waitTimeMax";
			EnemySwapper::enemySettings[i].waitTimeMax = cfg.get<float>(key).value_or(0.0f);
			key = std::string(EnemySwapper::emNames[i]) + "_" + "enemyNum";
			EnemySwapper::enemySettings[i].enemyNum = cfg.get<int>(key).value_or(1);
			key = std::string(EnemySwapper::emNames[i]) + "_" + "useDefault";
			EnemySwapper::enemySettings[i].useDefault = cfg.get<bool>(key).value_or(true);
			key = std::string(EnemySwapper::emNames[i]) + "_" + "odds";
			EnemySwapper::enemySettings[i].odds = cfg.get<float>(key).value_or(100.0f);
		}
	}

	inline void on_config_save(utility::Config& cfg) override {
		std::string key;
		cfg.set<float>("EnemyDataSettings_OddsAll", odds);
		cfg.set<float>("EnemyDataSettings_waitTimeMinAll", waitTimeMin);
		cfg.set<float>("EnemyDataSettings_waitTimeMaxAll", waitTimeMax);
		cfg.set<int>("EnemyDataSettings_enemyNumAll", enemyNum);
		cfg.set<bool>("EnemyDataSettings_shareSettings", shareSettings);

		for (int i = 0; i < EnemySwapper::emNames.size(); i++) {
			if(i == 32 || i == 33 || i == 35 || i == 39 || i == 40)
				continue;
			key = std::string(EnemySwapper::emNames[i]) + "_" + "waitTimeMin";
			cfg.set<float>(key, EnemySwapper::enemySettings[i].waitTimeMin);
			key = std::string(EnemySwapper::emNames[i]) + "_" + "waitTimeMax";
			cfg.set<float>(key, EnemySwapper::enemySettings[i].waitTimeMax);
			key = std::string(EnemySwapper::emNames[i]) + "_" + "enemyNum";
			cfg.set<int>(key, EnemySwapper::enemySettings[i].enemyNum);
			key = std::string(EnemySwapper::emNames[i]) + "_" + "useDefault";
			cfg.set<bool>(key, EnemySwapper::enemySettings[i].useDefault);
			key = std::string(EnemySwapper::emNames[i]) + "_" + "odds";
			cfg.set<float>(key, EnemySwapper::enemySettings[i].odds);

		}
	}

	// on_frame() is called every frame regardless whether the gui shows up.
	inline void on_frame() override{}
	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	inline void on_draw_ui() override {
		ImGui::Checkbox("Readme.txt", &showReadme);
		if (showReadme) {
			for (const char* ch : README) {
				ImGui::TextWrapped(ch);
				ImGui::Spacing();
			}
		}
		ImGui::Separator();
        ImGui::Checkbox("Share settings for all enemies", &shareSettings);
		if (shareSettings) {
          ImGui::TextWrapped("Wait time min: ");
          ImGui::InputFloat("##WaitTimeMinShared", &waitTimeMin);
          ImGui::Spacing();
          ImGui::TextWrapped("Wait time max: ");
          ImGui::InputFloat("##WaitTimeMaxShared", &waitTimeMax);
          ImGui::Spacing();
          ImGui::TextWrapped("Enemy num: ");
          ImGui::InputInt("##enemyNumShared", &enemyNum, 1);
          ImGui::Spacing();
          ImGui::TextWrapped("Odds for spawn (%%): ");
          ImGui::SliderFloat("##enemyOddsShared", &odds, 0.1f, 100.0f, "%.01f");
          validate_values(shareSettings);
          set_all_data_settings(waitTimeMin, waitTimeMax, enemyNum, odds);
		}
		else {
			/*ImGui::Text("Check if you want use custom settings for each enemies in BP (same shit like with swapper)");
			ImGui::Checkbox("Tick this if you go to BP", &isBp);
			ImGui::Separator();*/
			if (ImGui::Button("Restore default mod settings")) {
				for (int i = 0; i < EnemySwapper::emNames.size(); i++) {
					set_enemy_data_settings(i, 0.0f, 0.0f, 1, true);
				}
			}
			ImGui::Separator();
			for (int i = 0; i < EnemySwapper::emNames.size(); i++) {
				if(i == 32 || i == 33 || i == 35 || i == 39 || i == 40)//Vergils and Dante
					continue;
				uniqStr = std::string("Use default game settings ##" + std::to_string(i));
				ImGui::TextWrapped(EnemySwapper::emNames[i]);
				ImGui::Checkbox(uniqStr.c_str(), &EnemySwapper::enemySettings[i].useDefault);
				if (!EnemySwapper::enemySettings[i].useDefault) {
                    ImGui::TextWrapped("Wait time min: ");
				    uniqStr = std::string("##WaitTimeMin" + std::to_string(i));
				    ImGui::InputFloat(uniqStr.c_str() , &EnemySwapper::enemySettings[i].waitTimeMin, 0.1F);
                    ImGui::TextWrapped("Wait time max: ");
				    uniqStr = std::string("##WaitTimeMax" + std::to_string(i));
				    ImGui::InputFloat(uniqStr.c_str() , &EnemySwapper::enemySettings[i].waitTimeMax, 0.1f);
                    ImGui::TextWrapped("Enemy num: ");
					uniqStr = std::string("##EnemyNum" + std::to_string(i));
				    ImGui::InputInt(uniqStr.c_str() , &EnemySwapper::enemySettings[i].enemyNum);
					ImGui::TextWrapped("Odds for spawn (%%): ");
					uniqStr = std::string("##Odds" + std::to_string(i));
				    ImGui::SliderFloat(uniqStr.c_str(), &EnemySwapper::enemySettings[i].odds, 0.1f, 100.0f, "%.01f");
					validate_values(shareSettings, &EnemySwapper::enemySettings[i]);
				}
                ImGui::Separator();
			}
		}
	}
	// on_draw_debug_ui() is called when debug window shows up
	inline void on_draw_debug_ui() override{}

	void set_enemy_data_settings(int emIndx, float waitTimeMin, float waitTimeMax, int enemyNum, bool useDefault = false) {
		EnemySwapper::enemySettings[emIndx].waitTimeMin = waitTimeMin;
		EnemySwapper::enemySettings[emIndx].waitTimeMax = waitTimeMax;
		EnemySwapper::enemySettings[emIndx].enemyNum = enemyNum;
		EnemySwapper::enemySettings[emIndx].useDefault = useDefault;

	}

	inline static void set_all_data_settings(float waitTimeMin, float waitTimeMax, int enemyNum, float odds = 100.0f) {
		EnemyDataSettings::odds = odds;
		EnemyDataSettings::waitTimeMin = waitTimeMin;
		EnemyDataSettings::waitTimeMax = waitTimeMax;
		EnemyDataSettings::enemyNum = enemyNum;
        EnemySwapper::set_wait_time(waitTimeMin, waitTimeMax);
        EnemySwapper::set_odds(odds);
        EnemySwapper::set_enemy_num(enemyNum);
	}

	private:
	// function hook instance for our detour, convinient wrapper
	// around minhook
	inline void init_check_box_info() override {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
  }
	static inline std::string uniqStr = "";
	inline void validate_values(bool isSharedSettings, EnemySwapper::EnemySetting *emSetting = nullptr) {
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
	const std::array<const char*, 10> README{ 
			"This mod change enemy num property. You can't spawn less enemies, that placed on level by Capcom as enemy objects, only increase/decrease num property for each enemy"
		"or completely remove enemy from wave (set \"Enemy num\" to 0). For example, DMD m10 has 3 empusas in first encounter. It placed on level like: empusa with num = 1 "
		"and empusa with num = 2. So if you set \"Enemy num\" = 3, you will get 6 empusas, if you set \"Enemy num\" = 1, you will get 2 empusas. Same with BP lvl 30: "
		"it has 9 cainas, plased manually. If you set \"Enemy num\" to 4, you will get  4*9=36 cainas.",
            "This mod doesn't change enemy limit in encounters, => game will use default limits even if you set enemy num to >, that this setting. "
		"To change it, enable LDK and set \"hardlimit\" to whatever you need.",
			"Many BP stages doesn't have enemy limit in encounters, so make a lot of enemies on stage without control enemy limit with LDK option can crash the game.",
			"Game may be softlocked, if you set \"Enemy num\" to 0 for enemies, that should appears after a cutscene or a blackscreen. Same with low odd chance.",
			"Using this mod on boss Dante will not let you finish a mission (same with BP Vergil).",
			"Mod doesn't affect Dante and all Vergil bosses num.",
			"Game will crash, if you using this mod on Urizen1.",
			"Mod can be used with coop trainer's boss swap option to both bosses on M20 to prevent end the mission when you kill Dante or Vergil.",
			"Mod may softlock M19 after killing Vergil.",
			"If enemy swapper enabled, set some enemy num to 0 may also affect wrong enemy. To prevent that, use wave settings mod and set generate type to \"order\"."
	};
};

