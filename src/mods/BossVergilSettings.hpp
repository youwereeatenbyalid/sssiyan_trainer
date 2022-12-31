#pragma once
#include "Mod.hpp"
#include "Pl0300Controller.hpp"

//clang-format off

class BossVergilSettings : public Mod
{
private:

	enum TeleportCheatState
	{
		Fast,
		Instant,
		Custom
	};

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	static inline Pl0300Controller::Pl0300Controller::TeleportTimingParams _teleportParams[3];//Fast, instant, custom
	
	static inline bool _useTeleportCheats = false;
	static inline bool _useDoppelCheats = false;
	static inline bool _isDoppelParryAttacks = false;//0x2D
	static inline bool _isDoppelParryShells = false;//0x2E
	static inline bool _isDoppelInSDT = false;
	static inline bool _useAirRaidCheats = false;
	static inline bool _useJcCheats = false;
	//static inline bool _useJCECheats = false;
	static inline bool _useStabCheats = false;
	static inline bool _useDTCheats = false;


	static inline float _doppelAttackRate = 0.35f;//0x14
	static inline float _doppelHP = 500.0f;//0x10

	static inline float _airRaidSpeedInArea = 100.0f;//0x20
	static inline float _airRaidSpeed = 50.0f;//0x24
	static inline float _airRaidHomingSpeedInArea = 0.15f;//0x48
	static inline float _airRaidHomingXSpeed = 3.0f;//0x58

	static inline float _jcWaitSec = 0.1f;//0x18
	static inline float _jcMaxHeight = 2.75f;//0x3C

	//static inline float _jceSecWaitAttack = 0.1f;//0x3C

	static inline float _stabSpeed = 45.0f;//0x1C
	static inline float _stabDistanceMove = 15.0f;//0x20
	static inline float _stabDistanceFromTargetAfterWarp = 12.0f;//0x10

	static inline float _dtHpRecoveryVal = 0.3f;//0x10
	static inline float _dtActionSpeed = 1.2f;//0x14
	static inline float _dtReactionDamageRate = 0.1f;//0x1C
	static inline float _dtReactionDamageRateFromMajin = 0.1f;//0x20

	static inline int  _airRaidAttackNum = 3;//0x2C


	static inline TeleportCheatState _teleportsState = Fast;

public:
	BossVergilSettings()
	{
		_teleportParams[0].set_all(0.123f);
		_teleportParams[1].set_all(0);
	}

	static inline bool cheaton = true;

	std::string_view get_name() const override
	{
		return "BossVergilSettings";
	}
	std::string get_checkbox_name() override
	{
		return m_check_box_name;
	};
	std::string get_hotkey_name() override
	{
		return m_hot_key_name;
	};

	//setup all this stuff in Pl0300ControllerManager.cpp
	static void after_pl0300_start(uintptr_t threadCtx, uintptr_t pl0300)
	{
		if (!cheaton || pl0300 == 0)
			return;
		bool isDoppel = *(bool*)(pl0300 + 0x1F60);
		auto emParams = *(uintptr_t*)(pl0300 + 0x1768);
		if (emParams == 0)
			return;
		auto teleportParams = *(uintptr_t*)(emParams + 0xE0);
		auto doppelParams = *(uintptr_t*)(emParams + 0x128);
		auto airRaidParams = *(uintptr_t*)(emParams + 0x108);
		auto stabParams = *(uintptr_t*)(emParams + 0x110);
		auto dtParams = *(uintptr_t*)(emParams + 0x118);
		auto weaponYamato = *(uintptr_t*)(pl0300 + 0x2018);

		if (!isDoppel)
		{
			if (_useTeleportCheats)
				*(Pl0300Controller::Pl0300Controller::TeleportTimingParams*)(teleportParams + 0x60) = _teleportParams[_teleportsState];

			if (_useDoppelCheats && doppelParams != 0)
			{
				*(float*)(doppelParams + 0x10) = _doppelHP;
				*(float*)(doppelParams + 0x14) = _doppelAttackRate;
				*(bool*)(doppelParams + 0x2D) = _isDoppelParryAttacks;
				*(bool*)(doppelParams + 0x2E) = _isDoppelParryShells;
			}

			if (_useAirRaidCheats && airRaidParams != 0)
			{
				*(float*)(airRaidParams + 0x20) = _airRaidSpeedInArea;
				*(float*)(airRaidParams + 0x24) = _airRaidSpeed;
				*(float*)(airRaidParams + 0x48) = _airRaidHomingSpeedInArea;
				*(float*)(airRaidParams + 0x58) = _airRaidHomingXSpeed;
				*(int*)(airRaidParams + 0x2C) = _airRaidAttackNum;
			}

			if (_useStabCheats && stabParams != 0)
			{
				*(float*)(stabParams + 0x10) = _stabDistanceFromTargetAfterWarp;
				*(float*)(stabParams + 0x20) = _stabDistanceMove;
				*(float*)(stabParams + 0x1C) = _stabSpeed;
			}

			if (_useDTCheats && dtParams != 0)
			{
				*(float*)(dtParams + 0x10) = _dtHpRecoveryVal;
				*(float*)(dtParams + 0x14) = _dtActionSpeed;
				*(float*)(dtParams + 0x1C) = _dtReactionDamageRate;
				*(float*)(dtParams + 0x20) = _dtReactionDamageRateFromMajin;
			}

			if (_useJcCheats && weaponYamato != 0)
			{
				auto param = *(uintptr_t*)(weaponYamato + 0x300);
				if (param != 0)
				{
					auto jcParam = *(uintptr_t*)(param + 0x38);
					if (jcParam != 0)
					{
						*(float*)(jcParam + 0x18) = _jcWaitSec;
						*(float*)(jcParam + 0x3C) = _jcMaxHeight;
					}
				}
			}
		}
		else
		{
			if (_useDoppelCheats && _isDoppelInSDT)
				*(int*)(pl0300 + 0x9B0) = 2;
		}
	}

	std::optional<std::string> on_initialize() override
	{
		init_check_box_info();
		auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
		m_is_enabled = &cheaton;
		m_on_page = Page_Balance;
		m_full_name_string = "Boss Vergil Settings (+)";
		m_author_string = "V.P. Zadov";
		m_description_string = "Make boss Vergil suck a little less. Settings can't be changed after boss was spawned. I recommend to use it with akasha's \"Motivated boss Vergil\" file mod.";

		set_up_hotkey();

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override
	{
		_useTeleportCheats = cfg.get<bool>("BossVergilSettings._useTeleportCheats").value_or(true);
		_useDoppelCheats = cfg.get<bool>("BossVergilSettings._useDoppelCheats").value_or(true);
		_isDoppelParryAttacks = cfg.get<bool>("BossVergilSettings._isDoppelParryAttacks").value_or(true);
		_isDoppelParryShells = cfg.get<bool>("BossVergilSettings._isDoppelParryShells").value_or(true);
		_isDoppelInSDT = cfg.get<bool>("BossVergilSettings._isDoppelInSDT").value_or(true);
		_useAirRaidCheats = cfg.get<bool>("BossVergilSettings._useAirRaidCheats").value_or(true);
		_useJcCheats = cfg.get<bool>("BossVergilSettings._useJcCheats").value_or(true);
		_useStabCheats = cfg.get<bool>("BossVergilSettings._useStabCheats").value_or(true);
		_useStabCheats = cfg.get<bool>("BossVergilSettings._useDTCheats").value_or(true);

		_doppelAttackRate = cfg.get<float>("BossVergilSettings._doppelAttackRate").value_or(0.35f);
		_doppelHP = cfg.get<float>("BossVergilSettings._doppelHP").value_or(500.0f);
		_airRaidSpeedInArea = cfg.get<float>("BossVergilSettings._airRaidSpeedInArea").value_or(100.0f);
		_airRaidSpeed = cfg.get<float>("BossVergilSettings._airRaidSpeed").value_or(50.0f);
		_airRaidHomingSpeedInArea = cfg.get<float>("BossVergilSettings._airRaidHomingSpeedInArea").value_or(0.15f);
		_airRaidHomingXSpeed = cfg.get<float>("BossVergilSettings._airRaidHomingXSpeed").value_or(3.0f);
		_jcWaitSec = cfg.get<float>("BossVergilSettings._jcWaitSec").value_or(0);
		_jcMaxHeight = cfg.get<float>("BossVergilSettings._jcMaxHeight").value_or(40.0f);
		_stabSpeed = cfg.get<float>("BossVergilSettings._stabSpeed").value_or(60.0f);
		_stabDistanceMove = cfg.get<float>("BossVergilSettings._stabDistanceMove").value_or(25.0f);
		_stabDistanceFromTargetAfterWarp = cfg.get<float>("BossVergilSettings._stabDistanceFromTargetAfterWarp").value_or(12.0f);
		_dtHpRecoveryVal = cfg.get<float>("BossVergilSettings._dtHpRecoveryVal").value_or(0.42f);
		_dtActionSpeed = cfg.get<float>("BossVergilSettings._dtActionSpeed").value_or(1.35f);
		_dtReactionDamageRate = cfg.get<float>("BossVergilSettings._dtReactionDamageRate").value_or(0.1f);
		_dtReactionDamageRateFromMajin = cfg.get<float>("BossVergilSettings._dtReactionDamageRateFromMajin").value_or(0.1f);

		_airRaidAttackNum = cfg.get<int>("BossVergilSettings._airRaidAttackNum").value_or(5);
		_teleportsState = (TeleportCheatState)cfg.get<int>("BossVergilSettings._teleportsState").value_or((int)TeleportCheatState::Fast);

		_teleportParams[2].appearsSec = cfg.get<float>("BossVergilSettings._teleportParams[2].appearsSec").value_or(1.0f);
		_teleportParams[2].appearsSecLong = cfg.get<float>("BossVergilSettings._teleportParams[2].appearsSecLong").value_or(4.0f);
		_teleportParams[2].appearsSecOnBack = cfg.get<float>("BossVergilSettings._teleportParams[2].appearsSecOnBack").value_or(0.1f);
		_teleportParams[2].appearsSecOfOverhead = cfg.get<float>("BossVergilSettings._teleportParams[2].appearsSecOfOverhead").value_or(0.5f);
		_teleportParams[2].appearsSecOnStab = cfg.get<float>("BossVergilSettings._teleportParams[2].appearsSecOnStab").value_or(0.25f);
		_teleportParams[2].appearsSecOnCenter = cfg.get<float>("BossVergilSettings._teleportParams[2].appearsSecOnCenter").value_or(0.1f);
		_teleportParams[2].apeearsSec2Owner = cfg.get<float>("BossVergilSettings._teleportParams[2].apeearsSec2Owner").value_or(1.0f);
		_teleportParams[2].appearsSecOnCommandCombo = cfg.get<float>("BossVergilSettings._teleportParams[2].appearsSecOnCommandCombo").value_or(0.1f);
		_teleportParams[2].appearsSecOnAirRaid = cfg.get<float>("BossVergilSettings._teleportParams[2].appearsSecOnAirRaid").value_or(0.5f);
	}

	void on_config_save(utility::Config& cfg) override
	{
		cfg.set<bool>("BossVergilSettings._useTeleportCheats", _useTeleportCheats);
		cfg.set<bool>("BossVergilSettings._useDoppelCheats", _useDoppelCheats);
		cfg.set<bool>("BossVergilSettings._isDoppelParryAttacks", _isDoppelParryAttacks);
		cfg.set<bool>("BossVergilSettings._isDoppelParryShells", _isDoppelParryShells);
		cfg.set<bool>("BossVergilSettings._isDoppelInSDT", _isDoppelInSDT);
		cfg.set<bool>("BossVergilSettings._useAirRaidCheats", _useAirRaidCheats);
		cfg.set<bool>("BossVergilSettings._useJcCheats", _useJcCheats);
		cfg.set<bool>("BossVergilSettings._useStabCheats", _useStabCheats);
		cfg.set<bool>("BossVergilSettings._useStabCheats", _useDTCheats);

		cfg.set<float>("BossVergilSettings._doppelAttackRate", _doppelAttackRate);
		cfg.set<float>("BossVergilSettings._doppelHP", _doppelHP);
		cfg.set<float>("BossVergilSettings._airRaidSpeedInArea", _airRaidSpeedInArea);
		cfg.set<float>("BossVergilSettings._airRaidSpeed", _airRaidSpeed);
		cfg.set<float>("BossVergilSettings._airRaidHomingSpeedInArea", _airRaidHomingSpeedInArea);
		cfg.set<float>("BossVergilSettings._airRaidHomingXSpeed", _airRaidHomingXSpeed);
		cfg.set<float>("BossVergilSettings._jcWaitSec", _jcWaitSec);
		cfg.set<float>("BossVergilSettings._jcMaxHeight", _jcMaxHeight);
		cfg.set<float>("BossVergilSettings._stabSpeed", _stabSpeed);
		cfg.set<float>("BossVergilSettings._stabDistanceMove", _stabDistanceMove);
		cfg.set<float>("BossVergilSettings._stabDistanceFromTargetAfterWarp", _stabDistanceFromTargetAfterWarp);
		cfg.set<float>("BossVergilSettings._dtHpRecoveryVal", _dtHpRecoveryVal);
		cfg.set<float>("BossVergilSettings._dtActionSpeed", _dtActionSpeed);
		cfg.set<float>("BossVergilSettings._dtReactionDamageRate", _dtReactionDamageRate);
		cfg.set<float>("BossVergilSettings._dtReactionDamageRateFromMajin", _dtReactionDamageRateFromMajin);

		cfg.set<int>("BossVergilSettings._airRaidAttackNum", _airRaidAttackNum);
		cfg.set<int>("BossVergilSettings._teleportsState", (int)_teleportsState);

		cfg.set<float>("BossVergilSettings._teleportParams[2].appearsSec", _teleportParams[2].appearsSec);
		cfg.set<float>("BossVergilSettings._teleportParams[2].appearsSecLong", _teleportParams[2].appearsSecLong);
		cfg.set<float>("BossVergilSettings._teleportParams[2].appearsSecOnBack", _teleportParams[2].appearsSecOnBack);
		cfg.set<float>("BossVergilSettings._teleportParams[2].appearsSecOfOverhead", _teleportParams[2].appearsSecOfOverhead);
		cfg.set<float>("BossVergilSettings._teleportParams[2].appearsSecOnStab", _teleportParams[2].appearsSecOnStab);
		cfg.set<float>("BossVergilSettings._teleportParams[2].appearsSecOnCenter", _teleportParams[2].appearsSecOnCenter);
		cfg.set<float>("BossVergilSettings._teleportParams[2].apeearsSec2Owner", _teleportParams[2].apeearsSec2Owner);
		cfg.set<float>("BossVergilSettings._teleportParams[2].appearsSecOnCommandCombo", _teleportParams[2].appearsSecOnCommandCombo);
		cfg.set<float>("BossVergilSettings._teleportParams[2].appearsSecOnAirRaid", _teleportParams[2].appearsSecOnAirRaid);


	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override
	{

		if (ImGui::CollapsingHeader("Trick settings"))
		{
			ImGui::Checkbox("Use trick settings", &_useTeleportCheats);
			ImGui::TextWrapped("Trick speed:");
			ImGui::RadioButton("Fast", (int*)&_teleportsState, 0); ImGui::SameLine(); ImGui::Spacing();
			ImGui::RadioButton("Instant", (int*)&_teleportsState, 1); ImGui::SameLine(); ImGui::Spacing();
			ImGui::RadioButton("Custom", (int*)&_teleportsState, 2);
			if (ImGui::CollapsingHeader("Trick custom settings:"))
			{
				ImGui::TextWrapped("Appears time:");
				UI::SliderFloat("##appearsSec", &(_teleportParams[2].appearsSec), 0, 4.0f, "%.3f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
				ImGui::TextWrapped("Appears time long:");
				UI::SliderFloat("##appearsSecLong", &(_teleportParams[2].appearsSecLong), 0, 4.0f, "%.3f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
				ImGui::TextWrapped("Appears time back:");
				UI::SliderFloat("##appearsSecOnBack", &(_teleportParams[2].appearsSecOnBack), 0, 4.0f, "%.3f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
				ImGui::TextWrapped("Appears time up:");
				UI::SliderFloat("##appearsSecOfOverhead", &(_teleportParams[2].appearsSecOfOverhead), 0, 4.0f, "%.3f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
				ImGui::TextWrapped("Appears time on center of arena:");
				UI::SliderFloat("##appearsSecOnCenter", &(_teleportParams[2].appearsSecOnCenter), 0, 4.0f, "%.3f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
				ImGui::TextWrapped("Appears time on air raid teleport:");
				UI::SliderFloat("##appearsSecOnAirRaid", &(_teleportParams[2].appearsSecOnAirRaid), 0, 4.0f, "%.3f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
				ImGui::TextWrapped("Appears time on yamato stab:");
				UI::SliderFloat("##appearsSecOnStab", &(_teleportParams[2].appearsSecOnStab), 0, 4.0f, "%.3f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
				ImGui::TextWrapped("Appears time on phase 3 combo(?):");
				UI::SliderFloat("##appearsSecOnCommandCombo", &(_teleportParams[2].appearsSecOnCommandCombo), 0, 4.0f, "%.3f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
				ImGui::TextWrapped("Appears time on doppel returns to owner:");
				UI::SliderFloat("##apeearsSec2Owner", &(_teleportParams[2].apeearsSec2Owner), 0, 4.0f, "%.3f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
			}
		}
		
		ImGui::Separator();

		if (ImGui::CollapsingHeader("Doppelganger settings"))
		{
			ImGui::Checkbox("Use doppel settings", &_useDoppelCheats);
			ImGui::Checkbox("Doppel will parry attacks", &_isDoppelParryAttacks);
			ImGui::Checkbox("Doppel will parry shells", &_isDoppelParryShells);
			ImGui::Checkbox("Doppel uses SDT", &_isDoppelInSDT);
			ImGui::TextWrapped("Doppel's HP:");
			UI::SliderFloat("##DoppelHP", &_doppelHP, 100.0f, 10000.0f, "%.2f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
			ImGui::TextWrapped("Doppel's attack rate:");
			UI::SliderFloat("##DoppelAttackRate", &_doppelAttackRate, 0.01f, 1.0f, "%.2f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		}

		ImGui::Separator();

		if (ImGui::CollapsingHeader("Air Raid settings"))
		{
			ImGui::Checkbox("Use Air Raid settings", &_useAirRaidCheats);
			ImGui::ShowHelpMarker("\"Dive bomb\" or whatever you call this");
			ImGui::TextWrapped("Attack num:");
			UI::SliderInt("##_airRaidAttackNum", &_airRaidAttackNum, 1, 10, "%d", 1.0F, ImGuiSliderFlags_AlwaysClamp);
			ImGui::TextWrapped("Speed (50 - game default):");
			ImGui::InputFloat("##_airRaidSpeed", &_airRaidSpeed, 1.0f, 5.0f, "%.2f", 1.0f);
			ImGui::TextWrapped("Speed in area (100 - game default):");
			ImGui::InputFloat("##_airRaidSpeedInArea", &_airRaidSpeedInArea, 1.0f, 5.0f, "%.2f", 1.0f);
			ImGui::TextWrapped("Homing speed in area (0.15 - game default):");
			UI::SliderFloat("##_airRaidHomingSpeedInArea", &_airRaidHomingSpeedInArea, 0.01f, 8.0f, "%.3f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
			ImGui::TextWrapped("Speed on homing (3 - game default):");
			UI::SliderFloat("##_airRaidHomingXSpeed", &_airRaidHomingXSpeed, 0.1f, 25.0f, "%.2f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		}

		ImGui::Separator();

		if (ImGui::CollapsingHeader("Judgement cut settings"))
		{
			ImGui::Checkbox("Use Judgement Cut settings", &_useJcCheats);
			ImGui::TextWrapped("Delay before JC:");
			UI::SliderFloat("##_jcWaitSec", &_jcWaitSec, 0, 2.0f, "%.3f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
			ImGui::TextWrapped("JC max height:");
			UI::SliderFloat("##_jcMaxHeight", &_jcMaxHeight, 2.75f, 228.0f, "%.2f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
		}

		ImGui::Separator();

		if (ImGui::CollapsingHeader("Trick stab settings"))
		{
			ImGui::Checkbox("Use Yamato stab settings", &_useStabCheats);
			ImGui::TextWrapped("Stab speed (45 - game default):");
			UI::SliderFloat("##_stabSpeed", &_stabSpeed, 35.0f, 150.0f, "%.2f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
			ImGui::TextWrapped("Stab distance (15 - game default):");
			UI::SliderFloat("##_stabDistanceMove", &_stabDistanceMove, 12.0f, 50.0f, "%.2f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
			ImGui::TextWrapped("Distance from target after trick (12 - game default):");
			UI::SliderFloat("##_stabDistanceFromTargetAfterWarp", &_stabDistanceFromTargetAfterWarp, 5.0f, 30.0f, "%.2f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
		}

		ImGui::Separator();

		if (ImGui::CollapsingHeader("SDT settings"))
		{
			ImGui::Checkbox("Use SDT settings", &_useDTCheats);
			ImGui::TextWrapped("Hp recovery:");
			UI::SliderFloat("##_dtHpRecoveryVal", &_dtHpRecoveryVal, 0.0f, 1.0f, "%.3f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
			ImGui::TextWrapped("Actions speed:");
			UI::SliderFloat("##_dtActionSpeed", &_dtActionSpeed, 0.1f, 2.5f, "%.2f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
			ImGui::TextWrapped("Damage reaction (e.g. hyperarmor):");
			UI::SliderFloat("##_dtReactionDamageRate", &_dtReactionDamageRate, 0.0f, 1.0f, "%.2f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
			ImGui::TextWrapped("Damage reaction from SDT attacks:");
			UI::SliderFloat("##_dtReactionDamageRateFromMajin", &_dtReactionDamageRateFromMajin, 0.0f, 1.0f, "%.2f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
		}

		ImGui::Separator();
	}
};
//clang-format on