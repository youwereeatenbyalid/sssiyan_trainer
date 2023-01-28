#pragma once
#include "Mod.hpp"
#include "Coroutine/Coroutines.hpp"
#include "PrefabFactory/PrefabFactory.hpp"
#include "EndLvlHooks.hpp"
#include "PlayerTracker.hpp"
#include "QuickSilverController.hpp"
#include "PlSetActionData.hpp"

//clang-format off

class WitchTime : public Mod, private EndLvlHooks::IEndLvl
{
private:

	std::shared_ptr<Events::EventHandler<WitchTime, uintptr_t, uintptr_t, uintptr_t>> _plJustEscapeEventHandler;

	std::unique_ptr<QuickSilverCtrl::QuickSilverSlowWorldController> _qsController = nullptr;

	QuickSilverCtrl::QuickSilverSlowWorldController::SlowWorldType _slowWorldType;

	float _neroSlowShellLifeTime = 85.0f, _vSlowShellLifeTime = 85.0f, _danteSlowShellLifeTime = 85.0f, _vergilPlSlowShellLifeTime = 85.0f;
	float _banTime = 220.0f;

	bool _isInited = false;
	bool _useBanTime = false;
	bool _isEfxCreate = false;
	bool _pl0000WTTableHopperOnly = false;

	std::mutex _mtx;

	void reset(EndLvlHooks::EndType type) override
	{
		if (_isInited)
		{
			PlayerTracker::pl_on_just_escape_unsub(_plJustEscapeEventHandler);
			reset_ban_time();
			_slowWorldLifeTimeCoroutine.stop();
			_qsController = nullptr;
		}
		_isInited = false;
	}

	void reset_ban_time()
	{
		_activationDelayCoroutine.stop();
	}

	void destroy_slow_shell()
	{
		if (_qsController != nullptr)
			_qsController->stop_quicksilver();
		_slowWorldLifeTimeCoroutine.stop();
		if (_useBanTime)
		{
			_activationDelayCoroutine.set_delay(_banTime * 10.0f);
			_activationDelayCoroutine.start(this);
		}
	}

	Coroutines::Coroutine<decltype(&WitchTime::reset_ban_time), WitchTime*> _activationDelayCoroutine{ &WitchTime::reset_ban_time };
	Coroutines::Coroutine<decltype(&WitchTime::reset_ban_time), WitchTime*> _slowWorldLifeTimeCoroutine{ &WitchTime::destroy_slow_shell };

	void after_all_inits() override
	{
		PlayerTracker::pl_added_event_sub(std::make_shared<Events::EventHandler<WitchTime, uintptr_t, uintptr_t>>(this, &WitchTime::on_pl_add));
	}

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	void on_pl_just_escape(uintptr_t threadCtxt, uintptr_t pl, uintptr_t hitInfo)
	{
		if (!cheaton || pl == 0 || *(int*)(pl + 0x108) == 1 || !_isInited || _qsController->is_started() || _activationDelayCoroutine.is_started())
			return;
		auto plId = *(int*)(pl + 0xE64);
		if (plId == 0 && _pl0000WTTableHopperOnly && *(int*)(pl + 0x1834) == 0)
			return;
		if (!PlSetActionData::cmp_real_cur_action("AvoidLeft") && !PlSetActionData::cmp_real_cur_action("AvoidRight")
			&& !PlSetActionData::cmp_real_cur_action("AvoidLeftSword") && !PlSetActionData::cmp_real_cur_action("AvoidRightSword"))
		{
			switch (plId)
			{
			case 0:
			{
				if (!PlSetActionData::cmp_real_cur_action("TableHopperRightLv3") && !PlSetActionData::cmp_real_cur_action("TableHopperLeftLv3"))
					return;
				break;
			}

			case 2:
			{
				if (!PlSetActionData::cmp_real_cur_action("ShadowAvoidLeft") && !PlSetActionData::cmp_real_cur_action("ShadowAvoidRight"))
					return;
				break;
			}

			default:
				return;
			}
		}
		switch (plId)
		{
		case 0:
		{
			_slowWorldLifeTimeCoroutine.set_delay(_neroSlowShellLifeTime * 10.0f);
			break;
		}
		case 1:
		{
			_slowWorldLifeTimeCoroutine.set_delay(_danteSlowShellLifeTime * 10.0f);
			break;
		}
		case 2:
		{
			_slowWorldLifeTimeCoroutine.set_delay(_vSlowShellLifeTime * 10.0f);
			break;
		}
		case 4:
		{
			_slowWorldLifeTimeCoroutine.set_delay(_vergilPlSlowShellLifeTime * 10.0f);
			break;
		}
		default:
			break;
		}
		auto gameObj = *(uintptr_t*)(pl + 0x10);
		auto transform = *(uintptr_t*)(gameObj + 0x18);
		auto pos = *(gf::Vec3*)(transform + 0x30);
		auto rot = *(gf::Quaternion*)(transform + 0x40);
		_qsController->set_slow_world_type(_slowWorldType);
		auto shell = static_cast<QuickSilverCtrl::QuickSilverControllerBase*>(_qsController.get())->create_slow_time_shell(pos, rot, pl);
		if (_isEfxCreate)
			QuickSilverCtrl::QuickSilverSlowWorldController::request_slow_world_efx((uintptr_t)shell, pos, rot);
		_slowWorldLifeTimeCoroutine.start(this);
	}

	void on_pl_add(uintptr_t threadCtxt, uintptr_t pl)
	{
		if (!cheaton || _isInited || pl == 0 || *(int*)(pl + 0xE64) == 3)
			return;
		PlayerTracker::pl_on_just_escape_sub(_plJustEscapeEventHandler);
		_qsController = std::make_unique<QuickSilverCtrl::QuickSilverSlowWorldController>();
		_qsController->set_is_dt_consume(false);
		_isInited = true;
	}

public:
	WitchTime()
	{
		_plJustEscapeEventHandler = std::make_shared<Events::EventHandler<WitchTime, uintptr_t, uintptr_t, uintptr_t>>(this, &WitchTime::on_pl_just_escape);
		_activationDelayCoroutine.ignoring_update_on_pause(true);
		_slowWorldLifeTimeCoroutine.ignoring_update_on_pause(true);
	}

	static inline bool cheaton = true;

	void request_stop_witchtime()
	{
		if (!_isInited || !cheaton)
			return;
		_slowWorldLifeTimeCoroutine.stop();
		_activationDelayCoroutine.stop();
		_qsController->stop_quicksilver();
	}

	std::string_view get_name() const override
	{
		return "WitchTime";
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
		m_on_page = Page_CommonCheat;
		m_full_name_string = "Witch time";
		m_author_string = "V.P.Zadov";
		m_description_string = "Successfully dodging with a side roll gives you slow world. This mod must be enabled before loading into a level.";

		set_up_hotkey();

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override
	{
		_neroSlowShellLifeTime = cfg.get<float>("WitchTime._neroSlowShellLifeTime").value_or(825.0f);
		_danteSlowShellLifeTime = cfg.get<float>("WitchTime._danteSlowShellLifeTime").value_or(825.0f);
		_vSlowShellLifeTime = cfg.get<float>("WitchTime._vSlowShellLifeTime").value_or(825.0f);
		_vergilPlSlowShellLifeTime = cfg.get<float>("WitchTime._vergilPlSlowShellLifeTime").value_or(825.0f);
		_banTime = cfg.get<float>("WitchTime._banTime").value_or(220.0f);

		_useBanTime = cfg.get<bool>("WitchTime._useBanTime").value_or(false);
		_isEfxCreate = cfg.get<bool>("WitchTime._isEfxCreate").value_or(true);
		_pl0000WTTableHopperOnly = cfg.get<bool>("WitchTime._pl0000WTTableHopperOnly").value_or(false);

		_slowWorldType = (QuickSilverCtrl::QuickSilverSlowWorldController::SlowWorldType)cfg.get<int>
			("WitchTime._slowWorldType").value_or((int)QuickSilverCtrl::QuickSilverSlowWorldController::SlowWorldType::Slow);
	}

	void on_config_save(utility::Config& cfg) override
	{
		cfg.set<float>("WitchTime._neroSlowShellLifeTime", _neroSlowShellLifeTime);
		cfg.set<float>("WitchTime._danteSlowShellLifeTime", _danteSlowShellLifeTime);
		cfg.set<float>("WitchTime._vSlowShellLifeTime", _vSlowShellLifeTime);
		cfg.set<float>("WitchTime._vergilPlSlowShellLifeTime", _vergilPlSlowShellLifeTime);
		cfg.set<float>("WitchTime._banTime", _banTime);

		cfg.set<bool>("WitchTime._useBanTime", _useBanTime);
		cfg.set<bool>("WitchTime._isEfxCreate", _isEfxCreate);
		cfg.set<bool>("WitchTime._pl0000WTTableHopperOnly", _pl0000WTTableHopperOnly);

		cfg.set<int>("WitchTime._slowWorldType", (int)_slowWorldType);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override
	{
		ImGui::TextWrapped("Select quicksilver type:");
		ImGui::RadioButton("DMC3/5", (int*)&_slowWorldType, (int)QuickSilverCtrl::QuickSilverSlowWorldController::SlowWorldType::Slow); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
		ImGui::RadioButton("DMC1", (int*)&_slowWorldType, (int)QuickSilverCtrl::QuickSilverSlowWorldController::SlowWorldType::StopOnSlowPfb);

		ImGui::Spacing();

		ImGui::TextWrapped("Duration:");

		ImGui::Spacing();

		ImGui::TextWrapped("Nero:");
		UI::SliderFloat("##_neroSlowShellLifeTime", &_neroSlowShellLifeTime, 75.0f, 1200.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		ImGui::Checkbox("Only activate witch time on table hopper", &_pl0000WTTableHopperOnly);

		ImGui::Separator();

		ImGui::TextWrapped("Dante:");
		UI::SliderFloat("##_danteSlowShellLifeTime", &_danteSlowShellLifeTime, 75.0f, 1200.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);

		ImGui::Separator();

		ImGui::TextWrapped("V:");
		UI::SliderFloat("##_vSlowShellLifeTime", &_vSlowShellLifeTime, 75.0f, 1200.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);

		ImGui::Separator();

		ImGui::TextWrapped("Vergil:");
		UI::SliderFloat("##_vergilPlSlowShellLifeTime", &_vergilPlSlowShellLifeTime, 75.0f, 1200.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);

		ImGui::Separator();

		ImGui::Checkbox("Create clock-wave efx", &_isEfxCreate);
		ImGui::Checkbox("Use reactivation delay", &_useBanTime);
		ImGui::TextWrapped("Delay before witch time can be reactivated:");
		UI::SliderFloat("##_banTime", &_banTime, 75.0f, 800.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
	}
};
//clang-format on