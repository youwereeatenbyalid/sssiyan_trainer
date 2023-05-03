#pragma once
#include "Mod.hpp"
#include "Mods.hpp"
#include "VergilDoppelInitSetup.hpp"
#include "PlayerQuicksilver.hpp"
//#include "InputSystem.hpp"

//clang-format off

class DanteQuickSilver : public PlayerQuicksilver
{
private:

	bool _isOnAnyStyle = false;

	PlayerTracker::PlDanteStyleType _quickSilverBtn = PlayerTracker::PlDanteStyleType::Trickster;

	std::shared_ptr<Events::EventHandler<DanteQuickSilver, uintptr_t, uintptr_t, float*, int, bool>> _plAddDtEventHandler = 
		std::make_shared<Events::EventHandler<DanteQuickSilver, uintptr_t, uintptr_t, float*, int, bool>>(this, &DanteQuickSilver::on_pl_add_dt);

	std::shared_ptr<Events::EventHandler<DanteQuickSilver, uintptr_t, PlayerTracker::PlDanteStyleType>> _pl0100SetStyleRequestEventHandler =
		std::make_shared<Events::EventHandler<DanteQuickSilver, uintptr_t, PlayerTracker::PlDanteStyleType>>(this, &DanteQuickSilver::on_pl0100_set_style_requested);

	static inline const std::array<const char*, 4> _styleNames
	{
		"Trickster",
		"Swordmaster",
		"Gunslinger",
		"Royalguard"
	};

	void reset(EndLvlHooks::EndType type) override
	{
		PlayerQuicksilver::reset(type);
		PlayerTracker::pl0100_style_set_request_unsub(_pl0100SetStyleRequestEventHandler);
		PlayerTracker::pl_add_dt_gauge_unsub(_plAddDtEventHandler);
	}

	void on_pl_add_dt(uintptr_t threadCtxt, uintptr_t pl, float* val, int dtAddType, bool fixedValue)
	{
		if (cheaton && is_quicksilver_active() && *(int*)(pl+0xE64) == 1 && *(int*)(pl + 0x108) == 0 && *val > 0)
			*val = 0;
	}

	void on_pl_added(uintptr_t threadCtxt, uintptr_t plManager, uintptr_t player)
	{
		if (!cheaton || *(int*)(player + 0xE64) != 1 || *(int*)(player + 0x108) != 0 || _quickSilverCtrl != nullptr)
			return;
		_quickSilverCtrl = std::make_unique<QuickSilverCtrl::QuickSilverSlowWorldController>();
		_quickSilverCtrl->set_is_dt_consume(true);
		_quickSilverCtrl->set_dt_min_limit(3000.0f);
		PlayerTracker::pl0100_style_set_request_sub(_pl0100SetStyleRequestEventHandler);
		PlayerTracker::pl_add_dt_gauge_sub(_plAddDtEventHandler);
	}

	void on_pl0100_set_style_requested(uintptr_t plDante, PlayerTracker::PlDanteStyleType style)
	{
		if (!cheaton || *(int*)(plDante + 0x108) != 0 || _quickSilverCtrl == nullptr)
			return;
		if (!check_activation_mode(plDante))
			return;
		if (style == *(PlayerTracker::PlDanteStyleType*)(plDante + 0x188C))
		{
			if (_isOnAnyStyle || _quickSilverBtn == style)
			{
				if (_useBanTime)
				{
					if (is_ban_time())
						return;
					else
						start_ban_time();
				}
				if (!_quickSilverCtrl->is_started())
				{
					auto transform = *(uintptr_t*)((*(uintptr_t*)(plDante + 0x10)) + 0x18);
					auto pos = *(gf::Vec3*)(transform + 0x30);
					auto rot = *(gf::Quaternion*)(transform + 0x40);
					setup_quicksilver_ctrl(plDante, (volatile float*)(plDante + 0x1110));
					_quickSilverCtrl->create_slow_time_shell(pos, rot);
					if (_useTimerEfx)
						create_clock_efx((uintptr_t)(_quickSilverCtrl->get_cur_quicksilver_shell()), pos, rot);
				}
				else
					_quickSilverCtrl->stop_quicksilver();
			}
		}
	}

public:
	DanteQuickSilver() = default;

	~DanteQuickSilver()
	{
		PlayerTracker::on_pl_mng_pl_add_unsub(std::make_shared<Events::EventHandler<DanteQuickSilver, uintptr_t, uintptr_t, uintptr_t>>(this, &DanteQuickSilver::on_pl_added));
	}

	std::string_view get_name() const override
	{
		return "DanteQuickSilver";
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
		init_check_box_info();
		auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
		m_is_enabled = &cheaton;
		m_on_page = Page_DanteCheat;
		m_full_name_string = "Quicksilver style (+)";
		m_author_string = "V.P.Zadov";
		m_description_string = "Adds a quicksilver style for Dante. Press twice on the selected style button to enable/disable quicksilver. This mod must be enabled before loading into a level.";

		set_up_hotkey();

		PlayerTracker::on_pl_mng_pl_add_sub(std::make_shared<Events::EventHandler<DanteQuickSilver, uintptr_t, uintptr_t, uintptr_t>>(this, &DanteQuickSilver::on_pl_added));

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override
	{
		_quickSilverBtn = (PlayerTracker::PlDanteStyleType)cfg.get<int>("DanteQuickSilver._quickSilverBtn").value_or((int)PlayerTracker::PlDanteStyleType::Trickster);
		_isOnAnyStyle = cfg.get<bool>("DanteQuickSilver._isOnAnyStyle").value_or(false);
		PlayerQuicksilver::on_config_load(cfg);
	}
	void on_config_save(utility::Config& cfg) override
	{
		cfg.set<int>("DanteQuickSilver._quickSilverBtn", (int)_quickSilverBtn);
		cfg.set<bool>("DanteQuickSilver._isOnAnyStyle", _isOnAnyStyle);
		PlayerQuicksilver::on_config_save(cfg);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override
	{
		ImGui::Checkbox("Activate Quicksilver with any style button", &_isOnAnyStyle);
		if (!_isOnAnyStyle)
		{
			bool isSelected = false;
			if (ImGui::BeginCombo("##_quickSilverBtn", _styleNames[(int)_quickSilverBtn]))
			{
				for (int i = 0; i < _styleNames.size(); i++)
				{
					isSelected = (_quickSilverBtn == (PlayerTracker::PlDanteStyleType)i);
					if (ImGui::Selectable(_styleNames[i], isSelected))
						_quickSilverBtn = (PlayerTracker::PlDanteStyleType)i;
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		ImGui::Spacing();
		PlayerQuicksilver::on_draw_ui();
	}
};
//clang-format on