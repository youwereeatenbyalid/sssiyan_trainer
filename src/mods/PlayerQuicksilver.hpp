#pragma once
#include "Mod.hpp"
#include "EndLvlHooks.hpp"
#include "mods/QuickSilverController.hpp"
#include "ImGuiExtensions/ImGuiExtensions.h"
//#include "PlayerTracker.hpp"

//clang-format off

class PlayerQuicksilver : public Mod, private EndLvlHooks::IEndLvl
{
public:
	enum class ActivationMode
	{
		DPad,
		DPadLockOn,
		DPadNoLockOn,
		LockOnForward,
		LockOnBack
	};

private:
	bool _isBanTime = false;

	void update_activation_restriction()
	{
		_isBanTime = false;
		_updateBanTimeCoroutine.stop();
	}

	Coroutines::Coroutine<decltype(&PlayerQuicksilver::update_activation_restriction), PlayerQuicksilver*> _updateBanTimeCoroutine{ &PlayerQuicksilver::update_activation_restriction, false, false };

protected:

	ActivationMode _activationMode = ActivationMode::DPad;

	std::unique_ptr<QuickSilverCtrl::QuickSilverSlowWorldController> _quickSilverCtrl = nullptr;

	InputSystem* _inputSystem = nullptr;

	float _dtConsumption = 150.0f;
	float _dtUpdateFrequency = 800.0f;
	float _banTime = 350.0f;

	bool cheaton = false;
	bool _useBanTime = true;
	bool _useTimerEfx = false;

	QuickSilverCtrl::QuickSilverSlowWorldController::SlowWorldType _slowWorldType;

	virtual void reset(EndLvlHooks::EndType type) override
	{
		_quickSilverCtrl = nullptr;
		update_activation_restriction();
	}

	virtual void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	bool check_activation_mode(uintptr_t pl)
	{
		switch (_activationMode)
		{
			case PlayerQuicksilver::ActivationMode::DPad:
				return true;

			case PlayerQuicksilver::ActivationMode::DPadLockOn:
				return *(bool*)(pl + 0xED0);

			case PlayerQuicksilver::ActivationMode::DPadNoLockOn:
				return !*(bool*)(pl + 0xED0);

			case PlayerQuicksilver::ActivationMode::LockOnForward:
				return *(bool*)(pl + 0xED0) && _inputSystem->is_front_input((REManagedObject*)(*(uintptr_t*)(pl + 0xEF0)), 50.0f);

			case PlayerQuicksilver::ActivationMode::LockOnBack:
				return *(bool*)(pl + 0xED0) && _inputSystem->is_back_input((REManagedObject*)(*(uintptr_t*)(pl + 0xEF0)), 50.0f);

			default:
				return false;
			}
	}

	void setup_quicksilver_ctrl(uintptr_t owner, volatile float* dtGauge, float dtMinLimit = 3000.0f)
	{
		_quickSilverCtrl->set_owner(owner);
		_quickSilverCtrl->set_dt_gauge_ptr(dtGauge);
		_quickSilverCtrl->set_dt_min_limit(dtMinLimit);
		_quickSilverCtrl->set_dt_consumption(_dtConsumption);
		_quickSilverCtrl->set_dt_consumption_update_frequency(_dtUpdateFrequency);
		_quickSilverCtrl->set_slow_world_type(_slowWorldType);
	}

	bool is_ban_time() const noexcept { return _isBanTime; }

	void start_ban_time()
	{
		_isBanTime = true;
		_updateBanTimeCoroutine.set_delay(_banTime * 10.0f);
		_updateBanTimeCoroutine.start(this);
	}

	uintptr_t create_clock_efx(uintptr_t qsShell, gf::Vec3 pos, gf::Quaternion rot)
	{
		if (qsShell == 0)
			return 0;
		return QuickSilverCtrl::QuickSilverSlowWorldController::request_slow_world_efx(qsShell, pos, rot);
	}

	void after_all_inits() override
	{
		_inputSystem = static_cast<InputSystem*>(g_framework->get_mods()->get_mod("InputSystem"));
	}

public:
	PlayerQuicksilver() = default;

	std::string_view get_name() const override
	{
		return "";
	}
	std::string get_checkbox_name() override
	{
		return m_check_box_name;
	};
	std::string get_hotkey_name() override
	{
		return m_hot_key_name;
	};

	std::optional<std::string> on_initialize() override
	{
		m_is_enabled = &cheaton;
		m_on_page = Page_VergilCheat;
		m_full_name_string = "Quicksilver style (+)";
		m_author_string = "V.P.Zadov";
		return Mod::on_initialize();
	}

	bool is_quicksilver_active() const noexcept
	{
		if (_quickSilverCtrl == nullptr)
			return false;
		return _quickSilverCtrl->is_started();
	}

	void request_end_quicksilver()
	{
		if (_quickSilverCtrl == nullptr)
			return;
		_quickSilverCtrl->stop_quicksilver();
	}

	ActivationMode get_activation_mode() const noexcept { return _activationMode; }

	// Override this things if you want to store values in the config file
	virtual void on_config_load(const utility::Config& cfg) override
	{
		auto modName = std::string(get_name()) + std::string(".");
		_slowWorldType = (QuickSilverCtrl::QuickSilverSlowWorldController::SlowWorldType)cfg.get<int>
			(modName + std::string("_slowWorldType")).value_or((int)QuickSilverCtrl::QuickSilverSlowWorldController::SlowWorldType::Slow);
		_activationMode = (ActivationMode)cfg.get<int>(modName + std::string("_activationMode")).value_or((int)ActivationMode::DPad);

		_dtConsumption = cfg.get<float>(modName + std::string("_dtConsumption")).value_or(100.0f);
		_dtUpdateFrequency = cfg.get<float>(modName + std::string("_dtUpdateFrequency")).value_or(693.0f);
		_dtUpdateFrequency = cfg.get<float>(modName + std::string("_banTime")).value_or(350.0f);
		
		_useBanTime = cfg.get<bool>(modName + std::string("_useBanTime")).value_or(true);
		_useTimerEfx = cfg.get<bool>(modName + std::string("_useTimerEfx")).value_or(true);
	}

	virtual void on_config_save(utility::Config& cfg) override
	{
		auto modName = std::string(get_name()) + std::string(".");
		cfg.set<int>(modName + std::string("_slowWorldType"), (int)_slowWorldType);
		cfg.set<int>(modName + std::string("_activationMode"), (int)_activationMode);

		cfg.set<float>(modName + std::string("_dtConsumption"), (float)_dtConsumption);
		cfg.set<float>(modName + std::string("_dtUpdateFrequency"), (float)_dtUpdateFrequency);
		cfg.set<float>(modName + std::string("_banTime"), (float)_banTime);

		cfg.set<bool>(modName + std::string("_useBanTime"), _useBanTime);
		cfg.set<bool>(modName + std::string("_useTimerEfx"), _useTimerEfx);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	virtual void on_draw_ui() override
	{
		bool isSelected = false;
		
		ImGui::TextWrapped("Select quicksilver type:");
		ImGui::RadioButton("DMC3/5 (Slow world)", (int*)&_slowWorldType, (int)QuickSilverCtrl::QuickSilverSlowWorldController::SlowWorldType::Slow); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
		ImGui::RadioButton("DMC1 (Frozen world)", (int*)&_slowWorldType, (int)QuickSilverCtrl::QuickSilverSlowWorldController::SlowWorldType::StopOnSlowPfb);

		ImGui::Spacing();

		ImGui::TextWrapped("Activation command:");
		ImGui::RadioButton("Default", (int*)&_activationMode, (int)ActivationMode::DPad); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
		ImGui::RadioButton("Default + Lock On", (int*)&_activationMode, (int)ActivationMode::DPadLockOn); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
		ImGui::RadioButton("Default + no Lock On", (int*)&_activationMode, (int)ActivationMode::DPadNoLockOn);
		ImGui::RadioButton("Default + Lock On + Forward", (int*)&_activationMode, (int)ActivationMode::LockOnForward); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
		ImGui::RadioButton("Default + Lock On + Back", (int*)&_activationMode, (int)ActivationMode::LockOnBack);
		ImGui::Spacing();

		/* I think we can just lock this
		ImGui::TextWrapped("DT consumption update frequency:");
		UI::SliderFloat("##_dtUpdateFrequency", &_dtUpdateFrequency, 100.0f, 1500.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		*/
		ImGui::TextWrapped("DT cost per tick:");
		UI::SliderFloat("##_dtConsumption", &_dtConsumption, 5.0f, 200.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);

		ImGui::Spacing();

		ImGui::Checkbox("Enable clock-wave efx on qs startup", &_useTimerEfx);
		ImGui::Checkbox("Use activation/deactivation cooldown", &_useBanTime);
		ImGui::ShowHelpMarker("Creates a delay after activating/deactivating quicksilver during which you can't toggle it.");
		UI::SliderFloat("##_banTime", &_banTime, 100.0f, 850.0f, "%.1f", 1.0f);
	}
};
//clang-format on