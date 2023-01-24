#pragma once
#include "Mod.hpp"
#include "Mods.hpp"
#include "VergilDoppelInitSetup.hpp"
#include "PlayerQuicksilver.hpp"
#include "InputSystem.hpp"

//clang-format off

class VergilQuickSilver : public PlayerQuicksilver
{
private:

	std::shared_ptr<Events::EventHandler<VergilQuickSilver, uintptr_t, VergilDoppelInitSetup::DoppelDelayState, bool*>> _doppelSummonEventHandler =
		std::make_shared<Events::EventHandler<VergilQuickSilver, uintptr_t, VergilDoppelInitSetup::DoppelDelayState, bool*>>(this, &VergilQuickSilver::on_doppel_summon);

	std::shared_ptr<Events::EventHandler<VergilQuickSilver, uintptr_t, uintptr_t, float*, int, bool>> _plAddDtEventHandler = 
		std::make_shared<Events::EventHandler<VergilQuickSilver, uintptr_t, uintptr_t, float*, int, bool>>(this, &VergilQuickSilver::on_pl_add_dt);

	const std::array<char*, 4> _delayNames{ "Doppel banish (dpad down)", "Increased doppel speed", "Normal doppel speed", "Slow doppel speed"};

	VergilDoppelInitSetup::DoppelDelayState _quickSilverBtn = VergilDoppelInitSetup::DoppelDelayState::None;

	bool _isSDTConsume = false;

	void reset(EndLvlHooks::EndType type) override
	{
		PlayerQuicksilver::reset(type);
		VergilDoppelInitSetup::on_doppel_summon_unsub(_doppelSummonEventHandler);
		PlayerTracker::pl_add_dt_gauge_unsub(_plAddDtEventHandler);
	}

	void on_pl_add_dt(uintptr_t threadCtxt, uintptr_t pl, float* val, int dtAddType, bool fixedValue)
	{
		if (cheaton && is_quicksilver_active() && !_isSDTConsume && *(int*)(pl + 0xE64) == 4 && *val > 0)
			*val = 0;
	}

	void on_pl_added(uintptr_t threadCtxt, uintptr_t player)
	{
		if (!cheaton || *(int*)(player + 0xE64) != 4 || _quickSilverCtrl != nullptr)
			return;
		_quickSilverCtrl = std::make_unique<QuickSilverCtrl::QuickSilverSlowWorldController>();
		_quickSilverCtrl->set_is_dt_consume(true);
		VergilDoppelInitSetup::on_doppel_summon_sub(_doppelSummonEventHandler);
		PlayerTracker::pl_add_dt_gauge_sub(_plAddDtEventHandler);
	}

	void on_doppel_summon(uintptr_t pl0800, VergilDoppelInitSetup::DoppelDelayState doppelSpeed, bool* skipSummon)
	{
		if (!cheaton || _quickSilverCtrl == nullptr)
			return;
		if (!check_activation_mode(pl0800))
			return;
		if (doppelSpeed == VergilDoppelInitSetup::DoppelDelayState::Default )//special case cause doppel banish treated as default
		{
			bool isTriggeredByDPadDown = _inputSystem->is_action_button_pressed(*(uintptr_t*)(pl0800 + 0xEF0), InputSystem::PadInputGameAction::ChgStyleRG, true);
			switch (_quickSilverBtn)
			{
				case VergilDoppelInitSetup::None:
				{
					if (!isTriggeredByDPadDown)
						return;
					break;
				}
				case VergilDoppelInitSetup::Default:
				{
					if (isTriggeredByDPadDown)
						return;
					break;
				}
				default:
					return;
				}
		}
		else if(doppelSpeed != _quickSilverBtn)
			return;
		*skipSummon = true;
		if (_useBanTime)
		{
			if (is_ban_time())
				return;
			else
				start_ban_time();
		}
		if (!_quickSilverCtrl->is_started())
		{
			volatile float* gauge;
			float reqForActivation;
			if (_isSDTConsume)
			{
				gauge = (volatile float*)(pl0800 + 0x1B20);
				reqForActivation = 0.1f;
			}
			else
			{
				gauge = (volatile float*)(pl0800 + 0x1110);
				reqForActivation = 3000.0f;
			}
			setup_quicksilver_ctrl(pl0800, gauge, reqForActivation);
			auto transform = *(uintptr_t*)((*(uintptr_t*)(pl0800 + 0x10)) + 0x18);
			auto pos = *(gf::Vec3*)(transform + 0x30);
			auto rot = *(gf::Quaternion*)(transform + 0x40);
			_quickSilverCtrl->create_slow_time_shell(pos, rot);
			if (_useTimerEfx)
				create_clock_efx((uintptr_t)(_quickSilverCtrl->get_cur_quicksilver_shell()), pos, rot);
		}
		else
			_quickSilverCtrl->stop_quicksilver();
	}

public:
	VergilQuickSilver() = default;

	~VergilQuickSilver()
	{
		PlayerTracker::pl_added_event_unsub(std::make_shared<Events::EventHandler<VergilQuickSilver, uintptr_t, uintptr_t>>(this, &VergilQuickSilver::on_pl_added));
	}

	std::string_view get_name() const override
	{
		return "VergilQuickSilver";
	}
	std::string get_checkbox_name() override
	{
		return m_check_box_name;
	};
	std::string get_hotkey_name() override
	{
		return m_hot_key_name;
	};

	bool is_sdt_consume() const noexcept { return _isSDTConsume; }

	std::optional<std::string> on_initialize() override
	{
		init_check_box_info();
		auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
		m_is_enabled = &cheaton;
		m_on_page = Page_VergilCheat;
		m_full_name_string = "Quicksilver style (+)";
		m_author_string = "V.P.Zadov";
		m_description_string = "Add's quicksilver style to Vergil. Use specified doppelganger command to enable/disable quicksilver. This mod must be enabled before loading into a level."
			"You can't on/off QS while doppel is active.";

		set_up_hotkey();

		PlayerTracker::pl_added_event_sub(std::make_shared<Events::EventHandler<VergilQuickSilver, uintptr_t, uintptr_t>>(this, &VergilQuickSilver::on_pl_added));

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override
	{
		_quickSilverBtn = (VergilDoppelInitSetup::DoppelDelayState)cfg.get<int>("VergilQuickSilver._quickSilverBtn").value_or(VergilDoppelInitSetup::DoppelDelayState::Default);

		_isSDTConsume = cfg.get<bool>("VergilQuickSilver._isSDTConsume").value_or(false);

		PlayerQuicksilver::on_config_load(cfg);
	}
	void on_config_save(utility::Config& cfg) override
	{
		cfg.set<int>("VergilQuickSilver._quickSilverBtn", _quickSilverBtn);

		PlayerQuicksilver::on_config_save(cfg);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override
	{
		bool isSelected = false;
		if (ImGui::BeginCombo("##_quickSilverBtn", _delayNames[_quickSilverBtn]))
		{
			for (int i = 0; i < _delayNames.size(); i++)
			{
				isSelected = (_quickSilverBtn == (VergilDoppelInitSetup::DoppelDelayState)i);
				if (ImGui::Selectable(_delayNames[i], isSelected))
					_quickSilverBtn = (VergilDoppelInitSetup::DoppelDelayState)i;
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::ShowHelpMarker("Selected command will activate quicksilver instead of summoning the doppelganger. Make sure that selected input doesn't conflict with the \"Summon Doppelganger Instantly\" mod or "
			"\Boss Vergil Moves\" if you use the no Lock-On activation mode.");
		ImGui::Checkbox("Consume SDT gauge instead of DT gauge while Quicksilver is active;", &_isSDTConsume);
		ImGui::Spacing();

		PlayerQuicksilver::on_draw_ui();
	}
};
//clang-format on