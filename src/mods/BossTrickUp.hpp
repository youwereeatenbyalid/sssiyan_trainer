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
#include "EnemySpawner.hpp"
#include "PlayerTracker.hpp"

//clang-format off
namespace gf = GameFunctions;

class BossTrickUp : public Mod
{
private:
	float zOffs = 5.0f;
	float distanceOffs = 1.8f;
	float angleForwardThreshold = 8.0f;

	bool isPadInputTrickUp = false;
	bool is2ndAppear = false;
	bool isDoppelComeBack = false;

	InputSystem *_inputSys = nullptr;

	static inline BossTrickUp *_mod = nullptr;

	sdk::REMethodDefinition *_doppelComeBackMethod = nullptr;


	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	std::unique_ptr<FunctionHook> m_trickup_action_hook;

public:
	BossTrickUp()
	{
		_mod = this;
	}

	static inline bool cheaton = true;

	static inline uintptr_t ret = 0;
	static inline uintptr_t skipTrickUpRet = 0;

	static bool check_angle(uintptr_t vergil)
	{
		if (vergil == 0 || *(int*)(vergil + 0x108) == 1 || *(int*)(vergil + 0xE64) != 4)
			return false;
		if (*(bool*)(vergil + 0xED0) == false /*isManualLockOn*/ || _mod->_inputSys->is_action_button_pressed(InputSystem::PadInputGameAction::AttackS) && *(int*)(vergil + 0x1978) != 0)
			return false;
		auto padInput = *(uintptr_t*)(vergil + 0xEF0);
		if (cheaton && !VergilTrickUpLockedOn::cheaton)
			return _mod->_inputSys->is_front_input((REManagedObject*)padInput, _mod->angleForwardThreshold);
		else if (!cheaton && VergilTrickUpLockedOn::cheaton)
			return _mod->_inputSys->is_front_input((REManagedObject*)padInput, VergilTrickUpLockedOn::leftStickAngle);
		else
		{
			auto max = std::max(_mod->angleForwardThreshold, VergilTrickUpLockedOn::leftStickAngle);
			return _mod->_inputSys->is_front_input((REManagedObject*)padInput,max);
		}
		return false;
	}

	static bool check_input(uintptr_t vergil)
	{
		if (vergil == 0 || *(bool*)(vergil + 0xED0) == false /*isManualLockOn*/ || _mod->_inputSys->is_action_button_pressed(InputSystem::PadInputGameAction::AttackS) && *(int*)(vergil + 0x1978) != 0)
		{
			_mod->isPadInputTrickUp = false;
			return true;
		}
		bool res = true;
		auto padInput = *(uintptr_t*)(vergil + 0xEF0);
		if (padInput != 0)
		{
			if(cheaton)
				res = _mod->isPadInputTrickUp = _mod->_inputSys->is_front_input((REManagedObject*)padInput, _mod->angleForwardThreshold);
			if (VergilTrickUpLockedOn::cheaton && cheaton)
			{
				auto max = std::max(_mod->angleForwardThreshold, VergilTrickUpLockedOn::leftStickAngle);
				res = _mod->_inputSys->is_front_input((REManagedObject*)padInput, max);
			}
			else if(VergilTrickUpLockedOn::cheaton)
				res = _mod->_inputSys->is_front_input((REManagedObject*)padInput, VergilTrickUpLockedOn::leftStickAngle);
		}
		return res;
	}

	static void set_appear_pos(uintptr_t threadCtxt, uintptr_t vergil)//Calling this in trickTrails mod cause hook already there.
	{
		if (!cheaton || !_mod->isPadInputTrickUp)
			return;
		if (!_mod->is2ndAppear)
		{
			_mod->is2ndAppear = true;
			return;
		}
		if (!PlSetActionData::cmp_real_cur_action("TrickUp"))
			return;
		if (vergil == 0 || *(int*)(vergil + 0x108) == 1 || *(int*)(vergil + 0xE64) != 4)
			return;
		uintptr_t lockOnObj = *(uintptr_t*)(vergil + 0x428);
		if (lockOnObj == 0)
			return;
		lockOnObj = *(uintptr_t*)(lockOnObj + 0x10);
		if (lockOnObj == 0)
			return;
		auto cachedCharacter = *(uintptr_t*)(lockOnObj + 0x58);
		if (auto cachedShell = *(uintptr_t*)(lockOnObj + 0x68); cachedShell != 0 || cachedCharacter == 0)
			return;
		auto emId = 0;
		auto genIdInfo = *(uintptr_t*)((uintptr_t)cachedCharacter + 0x4C8);
		if (genIdInfo != 0)
			emId = *(int*)(genIdInfo + 0x10);
		else
			emId = *(int*)((uintptr_t)cachedCharacter + 0xB18); //this will suck if dealing with pl characters 'cause it's emId from enemy class
		if (emId == 25)
			return;
		float len = sqrt(*(float*)(lockOnObj + 0x78));//sqLength
		gf::Vec3 emPos;
		if (auto cachedTransform = *(uintptr_t*)(lockOnObj + 0x50); cachedTransform != 0)
			emPos = *(gf::Vec3*)(cachedTransform + 0x30);
		else
			return;
		gf::Vec3 newPlPos = emPos;
		auto lockOnOffs = *(gf::Vec3*)(lockOnObj + 0x20);
		auto vergilTransform = *(uintptr_t*)((*(uintptr_t*)(vergil + 0x10)) + 0x18);
		auto pPos = *(gf::Vec3*)(vergilTransform + 0x30);
		//newPlPos.z += 1.5f;
		if (emId != 35)
			gf::Transform_SetPosition::set_character_pos(vergil, newPlPos, true);
		newPlPos.x = pPos.x + (len - _mod->distanceOffs) * (emPos.x - pPos.x) / (float)len;
		newPlPos.y = pPos.y + (len - _mod->distanceOffs) * (emPos.y - pPos.y) / (float)len;
		newPlPos += lockOnOffs;
		gf::Transform_SetPosition setPos{ (void*)vergilTransform };
		newPlPos.z += _mod->zOffs;
		setPos(newPlPos);
		//posCorrector.set_position(newPlPos);
		if (_mod->isDoppelComeBack && _mod->_doppelComeBackMethod != nullptr)
		{
			if (*(bool*)(vergil + 0x18A8))
				_mod->_doppelComeBackMethod->call(threadCtxt, vergil);
		}
	}

	static naked void detour()
	{
		__asm {
			cmp byte ptr [BossTrickUp::cheaton], 1
			je cheat
			cmp byte ptr [VergilTrickUpLockedOn::cheaton], 1
			je cheat

			originalcode:
			mov r8d, 0x7D5
			jmp qword ptr [BossTrickUp::ret]

			cheat:
			push rax
			push rcx
			push rdx
			push rsp
			push r8
			push r9
			push r10
			push r11
			mov rcx, rsi
			sub rsp, 32
			call qword ptr [BossTrickUp::check_input]
			add rsp, 32
			cmp al, 0
			pop r11
			pop r10
			pop r9
			pop r8
			pop rsp
			pop rdx
			pop rcx
			pop rax
			je skip
			jmp originalcode

			skip:
			mov r8d, 0
			jmp qword ptr [BossTrickUp::ret]
		}
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
		m_full_name_string = "Boss's Trick Up (+)";
		m_author_string = "V.P.Zadov, SSSiyan";
		m_description_string = "Lock on + forward + trick will instantly teleport Vergil up to enemy head like boss Vergil trick up works. Trick up without lock on works like it works by default. "
		"Can send you out of bounds. Sometimes trick up sound is missing after teleport. Doesn't work vs Nidhogg.";

		set_up_hotkey();

		auto trickUpSetActionAddr = m_patterns_cache->find_addr(base, "41 B8 D5 07 00 00 C7 44 24 38 00 00 00 00 48");//DevilMayCry5.exe+550BE2
		if (!trickUpSetActionAddr)
		{
			return "Unable to find BossTrickUp.trickUpSetActionAddr pattern.";
		}

		skipTrickUpRet += trickUpSetActionAddr.value();
		_inputSys = static_cast<InputSystem*>(g_framework->get_mods()->get_mod("InputSystem"));

		if (!install_hook_absolute(trickUpSetActionAddr.value(), m_trickup_action_hook, &detour, &ret, 0x6))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize BossTrickUp.trickUpSetAction";
		}

		_doppelComeBackMethod = sdk::find_method_definition("app.PlayerVergilPL", "comeBackDoppelGanger()");

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override 
	{
		zOffs = cfg.get<float>("BossTrickUp.zOffs").value_or(4.3F);
		distanceOffs = cfg.get<float>("BossTrickUp.distanceOffs").value_or(2.0f);
		angleForwardThreshold = cfg.get<float>("BossTrickUp.angleForwardThreshold").value_or(22.5f);
		isDoppelComeBack = cfg.get<bool>("BossTrickUp.isDoppelComeBack").value_or(true);
	}
	void on_config_save(utility::Config& cfg) override 
	{
		cfg.set<float>("BossTrickUp.zOffs", zOffs);
		cfg.set<float>("BossTrickUp.distanceOffs", distanceOffs);
		cfg.set<float>("BossTrickUp.angleForwardThreshold", angleForwardThreshold);
		cfg.set<bool>("BossTrickUp.isDoppelComeBack", isDoppelComeBack);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override 
	{
		ImGui::TextWrapped("Height offset:");
		UI::SliderFloat("##zOffs", &zOffs, 3.0f, 8.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		ImGui::TextWrapped("Front distance offset:");
		UI::SliderFloat("##distOffs", &distanceOffs, 0.2f, 4.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		ImGui::TextWrapped("Left stick forward angle threshold:");
		UI::SliderFloat("##angleForwardThreshold", &angleForwardThreshold, 3.5f, 60.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
		ImGui::Checkbox("Auto come back doppel after trick up teleport", &isDoppelComeBack);
	}
};
//clang-format on