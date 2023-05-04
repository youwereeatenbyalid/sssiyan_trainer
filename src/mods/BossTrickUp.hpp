#pragma once
#include <algorithm>
#include "Mod.hpp"
#include "sdk/DMC5.hpp"
#include "GameFunctions/PositionController.hpp"
#include "PlSetActionData.hpp"
#include "VergilTrickUpLockedOn.hpp"
#include "InputSystem.hpp"
#include "Mods.hpp"
#include "mods/EndLvlHooks.hpp"
#include "EnemyData.hpp"
#include "PlayerTracker.hpp"
#include "InfiniteTrickUp.hpp"

//clang-format off
namespace gf = GameFunctions;

class BossTrickUp : public Mod
{
private:
	float _zOffs = 5.0f;
	float _distanceOffs = 1.8f;
	float _angleForwardThreshold = 8.0f;

	bool _isPadInputTrickUp = false;
	bool _isDoppelComeBack = false;

	InputSystem *_inputSys = nullptr;

	static inline BossTrickUp *_mod = nullptr;

	const sdk::REMethodDefinition *_doppelComeBackMethod = nullptr;
	const sdk::REMethodDefinition* _pl0800SetActionMethod = nullptr;
	const sdk::REMethodDefinition* _getLockOnPosMethod;


	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	std::shared_ptr<Detour_t> m_trickup_action_hook;

	void on_pl0800_set_air_trick_action(uintptr_t threadCntxt, uintptr_t pl0800, uintptr_t targetGameObj, bool* skipCall)
	{
		if (!cheaton || *(bool*)(pl0800 + 0x17F0))
			return;
		bool isTrickUpAllowed = InfiniteTrickUp::cheaton ? true : *(bool*)(pl0800 + 0x19BC);//isTrickUp;
		auto padInput = *(uintptr_t*)(pl0800 + 0xEF0);
		if (!isTrickUpAllowed || !_mod->_inputSys->is_front_input((REManagedObject*)padInput, _mod->_angleForwardThreshold))
		{
			_isPadInputTrickUp = false;
			return;
		}
		else
		{
			_isPadInputTrickUp = true;
			_pl0800SetActionMethod->call(threadCntxt, pl0800, 0x7D5, 0, 0, 0, 0, 0, false, false);
			*(bool*)(pl0800 + 0x19BC) = false;
			*skipCall = true;
		}
	}

	void on_sdk_init() override
	{
		_doppelComeBackMethod = sdk::find_method_definition("app.PlayerVergilPL", "comeBackDoppelGanger()");
		_pl0800SetActionMethod = sdk::find_method_definition("app.PlayerVergilPL", "setAction(app.PlayerVergilPL.Actions, System.UInt32, System.Single, System.Single, "
			"via.motion.InterpolationMode, via.motion.InterpolationCurve, System.Boolean, System.Boolean)");
		_getLockOnPosMethod = sdk::find_method_definition("app.LockOnObject", "get_lockOnPosition()");
	}

public:
	BossTrickUp()
	{
		_mod = this;
	}

	~BossTrickUp()
	{
		PlayerTracker::on_pl0800_set_air_trick_action_unsub(std::make_shared<Events::EventHandler<BossTrickUp, uintptr_t, uintptr_t, uintptr_t, bool*>>(this, &BossTrickUp::on_pl0800_set_air_trick_action));
	}

	static inline bool cheaton = true;

	static inline uintptr_t ret = 0;
	static inline uintptr_t skipTrickUpRet = 0;

	static void set_appear_pos(uintptr_t threadCtxt, uintptr_t vergil)//Calling this in trickTrails mod cause hook already there.
	{
		if (!cheaton || !_mod->_isPadInputTrickUp)
			return;
		if (vergil == 0 || *(int*)(vergil + 0x108) == 1 || *(int*)(vergil + 0xE64) != 4)
			return;
		if ( !gf::StringController::str_cmp(*(uintptr_t*)(vergil + 0x1A0), L"Trick_Up_Start") || !gf::StringController::str_cmp(*(uintptr_t*)(vergil + 0x190), L"Trick_Up_DrawOff"))
			return;
		uintptr_t lockOnObj = *(uintptr_t*)(vergil + 0x428);
		if (lockOnObj == 0)
		{
			_mod->_isPadInputTrickUp = false;
			return;
		}
		auto lockOnTarget = *(uintptr_t*)(lockOnObj + 0x10);
		if (lockOnTarget == 0)
		{
			_mod->_isPadInputTrickUp = false;
			return;
		}
		auto cachedCharacter = *(uintptr_t*)(lockOnTarget + 0x58);
		bool isSafeWarpRequested = false;
		if (auto cachedShell = *(uintptr_t*)(lockOnTarget + 0x68); cachedShell != 0)
			isSafeWarpRequested = true;
		auto emId = EnemyData::get_em_id(cachedCharacter);
		if (emId == EnemyData::Nidhogg)
		{
			_mod->_isPadInputTrickUp = false;
			return;
		}
		float len = sqrt(*(float*)(lockOnTarget + 0x78));//sqLength
		gf::Vec3 targetPos = _mod->_getLockOnPosMethod->call<gf::Vec3>(threadCtxt, lockOnObj);
		auto cachedTargetTransform = *(uintptr_t*)(lockOnObj + 0x50);
		if (cachedTargetTransform == 0)
		{
			_mod->_isPadInputTrickUp = false;
			return;
		}

		if (emId == EnemyData::None)
		{
			isSafeWarpRequested = true;
			targetPos = *(gf::Vec3*)(cachedTargetTransform + 0x30);
		}
		else if (emId == EnemyData::Urizen1)
			isSafeWarpRequested = true;
		else if (emId == EnemyData::Behemoth && gf::StringController::str_cmp(*(uintptr_t*)(cachedCharacter + 0x1A0), L"Em0400HideInGround") &&
			gf::StringController::str_cmp(*(uintptr_t*)(cachedCharacter + 0x190), "HideChase"))
				isSafeWarpRequested = true;
		else if (emId == EnemyData::Vergil)
		{
			auto airRaidCtrl = *(uintptr_t*)(cachedCharacter + 0x1C90);
			if (airRaidCtrl == 0 || *(bool*)(airRaidCtrl + 0x30))//isOutOfArea
				isSafeWarpRequested = true;
		}
		else if (emId == EnemyData::DeathScissors)
		{
			auto terrainChecker = *(uintptr_t*)(cachedCharacter + 0xD80);
			if (terrainChecker == 0 || *(bool*)(terrainChecker + 0x50))//isOutOfArea
				isSafeWarpRequested = true;
		}
		gf::Vec3 newPlPos = targetPos;
		auto vergilTransform = *(uintptr_t*)((*(uintptr_t*)(vergil + 0x10)) + 0x18);
		auto pPos = *(gf::Vec3*)(vergilTransform + 0x30);

		if (!isSafeWarpRequested)
			gf::Transform_SetPosition::set_character_pos(vergil, newPlPos, true);
		newPlPos.x = pPos.x + (len - _mod->_distanceOffs) * (targetPos.x - pPos.x) / (float)len;
		newPlPos.y = pPos.y + (len - _mod->_distanceOffs) * (targetPos.y - pPos.y) / (float)len;
		gf::Transform_SetPosition setPos{ (void*)vergilTransform };
		newPlPos.z += _mod->_zOffs;
		setPos(newPlPos);
		//posCorrector.set_position(newPlPos);
		if (_mod->_isDoppelComeBack && _mod->_doppelComeBackMethod != nullptr)
		{
			if (*(bool*)(vergil + 0x18A8))
				_mod->_doppelComeBackMethod->call(threadCtxt, vergil);
		}
		_mod->_isPadInputTrickUp = false;
	}

	std::string_view get_name() const override
	{
		return "BossTrickUp";
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
		m_on_page = Page_VergilTrick;
		//TrickTrailsEfx needed because set appear pos is called there (see line 89 in this file)
		m_depends_on = { "InputSystem","PlayerTracker","VergilTrickTrailsEfx" };
		m_full_name_string = "Boss's Trick Up (+)";
		m_author_string = "V.P.Zadov, SSSiyan";
		m_description_string = "Lock on + forward + trick will instantly teleport Vergil directly above the enemies head, similarly to how Boss Vergil teleports. Can interrupt any move what air trick can also. "
			"Trick - up without lock on works as normal.";

		set_up_hotkey();

		_inputSys = static_cast<InputSystem*>(g_framework->get_mods()->get_mod("InputSystem"));

		PlayerTracker::on_pl0800_set_air_trick_action_sub(std::make_shared<Events::EventHandler<BossTrickUp, uintptr_t, uintptr_t, uintptr_t, bool*>>(this, &BossTrickUp::on_pl0800_set_air_trick_action));

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override 
	{
		_zOffs = cfg.get<float>("BossTrickUp._zOffs").value_or(4.3F);
		_distanceOffs = cfg.get<float>("BossTrickUp._distanceOffs").value_or(2.0f);
		_angleForwardThreshold = cfg.get<float>("BossTrickUp._angleForwardThreshold").value_or(22.5f);
		_isDoppelComeBack = cfg.get<bool>("BossTrickUp._isDoppelComeBack").value_or(true);
	}
	void on_config_save(utility::Config& cfg) override 
	{
		cfg.set<float>("BossTrickUp._zOffs", _zOffs);
		cfg.set<float>("BossTrickUp._distanceOffs", _distanceOffs);
		cfg.set<float>("BossTrickUp._angleForwardThreshold", _angleForwardThreshold);
		cfg.set<bool>("BossTrickUp._isDoppelComeBack", _isDoppelComeBack);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override 
	{
		if (ImGui::CollapsingHeader("Issues")) {
			ImGui::TextWrapped("This mod can send you out of bounds.\n Sometimes the trick up sound doesn't play after teleporting.\nDoesn't work vs Nidhogg.");
		};
		ImGui::TextWrapped("Height offset:");
		UI::SliderFloat("##_zOffs", &_zOffs, 3.0f, 8.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		ImGui::TextWrapped("Horizontal offset:");
		UI::SliderFloat("##distOffs", &_distanceOffs, 0.2f, 4.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		ImGui::TextWrapped("Left stick forward angle threshold:");
		UI::SliderFloat("##_angleForwardThreshold", &_angleForwardThreshold, 3.5f, 60.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		ImGui::Checkbox("Doppelganger auto-return after boss trick", &_isDoppelComeBack);
	}
};
//clang-format on