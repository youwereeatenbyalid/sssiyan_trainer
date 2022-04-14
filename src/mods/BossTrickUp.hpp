#pragma once
#include "Mod.hpp"
#include "sdk/DMC5.hpp"
#include "GameFunctions/PositionController.hpp"
#include "PlSetActionData.hpp"
#include "VergilTrickUpLockedOn.hpp"
#include <algorithm>

//clang-format off
namespace gf = GameFunctions;

class BossTrickUp : public Mod
{
private:
	static inline float zOffs = 5.0f;
	static inline float distanceOffs = 1.8f;
	static inline float angleForwardThreshold = 8.0f;

	static inline bool isPadInputTrickUp = false;
	static inline bool is2ndAppear = false;

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	std::unique_ptr<FunctionHook> m_trickup_action_hook;

public:
	BossTrickUp() = default;

	static inline bool cheaton = true;

	static inline uintptr_t ret = 0;
	static inline uintptr_t skipTrickUpRet = 0;

	static bool check_angle(uintptr_t vergil)
	{
		if (vergil == 0 || *(int*)(vergil + 0x108) == 1 || *(int*)(vergil + 0xE64) != 4)
			return false;
		if (*(bool*)(vergil + 0xED0) == false)//isManualLockOn
			return false;
		auto padInput = *(uintptr_t*)(vergil + 0xEF0);
		if(cheaton && !VergilTrickUpLockedOn::cheaton)
			return sdk::call_object_func_easy<bool>((REManagedObject*)padInput, "isFrontInput(System.Single)", angleForwardThreshold);
		else if (!cheaton && VergilTrickUpLockedOn::cheaton)
			return sdk::call_object_func_easy<bool>((REManagedObject*)padInput, "isFrontInput(System.Single)", VergilTrickUpLockedOn::leftStickAngle);
		else
		{
			auto max = std::max(angleForwardThreshold, VergilTrickUpLockedOn::leftStickAngle);
			return sdk::call_object_func_easy<bool>((REManagedObject*)padInput, "isFrontInput(System.Single)", max);
		}
		return false;
	}

	static bool check_input(uintptr_t vergil)
	{
		if(vergil == 0)
			return false;
		if (*(bool*)(vergil + 0xED0) == false)//isManualLockOn
		{
			isPadInputTrickUp = false;
			return true;
		}
		bool res = true;
		auto padInput = *(uintptr_t*)(vergil + 0xEF0);
		if (padInput != 0)
		{
			if(cheaton)
				res = isPadInputTrickUp = sdk::call_object_func_easy<bool>((REManagedObject*)padInput, "isFrontInput(System.Single)", angleForwardThreshold);
			if (VergilTrickUpLockedOn::cheaton && cheaton)
			{
				auto max = std::max(angleForwardThreshold, VergilTrickUpLockedOn::leftStickAngle);
				res = sdk::call_object_func_easy<bool>((REManagedObject*)padInput, "isFrontInput(System.Single)", max);
			}
			else if(VergilTrickUpLockedOn::cheaton)
				res = sdk::call_object_func_easy<bool>((REManagedObject*)padInput, "isFrontInput(System.Single)", VergilTrickUpLockedOn::leftStickAngle);
		}
		return res;
	}

	static void set_appear_pos(uintptr_t vergil)//Calling this in trickTrails mod cause hook already there.
	{
		if (!is2ndAppear)
		{
			is2ndAppear = true;
			return;
		}
		if(!PlSetActionData::cmp_real_cur_action("TrickUp"))
			return;
		if(!cheaton || !isPadInputTrickUp)
			return;
		if (vergil == 0 || *(int*)(vergil + 0x108) == 1 || *(int*)(vergil + 0xE64) != 4)
			return;
		uintptr_t lockOnObj = *(uintptr_t*)(PlayerTracker::vergilentity + 0x428);
		if(lockOnObj == 0)
			return;
		lockOnObj = *(uintptr_t*)(lockOnObj + 0x10);
		if (lockOnObj == 0)
			return;
		float len = sqrt(*(float*)(lockOnObj + 0x78));//sqLength
		auto emPos = gf::PtrController::get_ptr<gf::Vec3>(std::array<uintptr_t, 2>{0x50, 0x30}, lockOnObj, true);
		auto lockOnOffs = *(gf::Vec3*)(lockOnObj + 0x20);
		auto vergilTransform = gf::PtrController::get_ptr<void>(std::array<uintptr_t, 2>{0x10, 0x18}, vergil, true);
		auto pPos = gf::PtrController::get_ptr<gf::Vec3>(std::array<uintptr_t, 3>{0x10, 0x18, 0x30}, vergil, true);
		gf::Vec3 newPlPos = *emPos;
		newPlPos.x = pPos->x + (len - distanceOffs) * (emPos->x - pPos->x) / (float)len;
		newPlPos.y = pPos->y + (len - distanceOffs) * (emPos->y - pPos->y) / (float)len;
		newPlPos += lockOnOffs;
		//newPlPos.z += 1.5f;
		gf::Transform_SetPosition::set_character_pos(vergil, newPlPos, true);
		gf::PositionErrorCorrector posCorrector{(void*)(*(uintptr_t*)(vergil + 0x8E8))};
		newPlPos.z += zOffs;
		posCorrector.set_position(newPlPos);
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
		m_author_string = "VPZadov, SSSiyan";
		m_description_string = "Lock on + forward + trick will instantly teleport Vergil up to enemy head like boss Vergil trick up works. Trick up without lock on works like it works by default. "
		"Like an instant transmission air trick option, can send you out of bounds.";

		set_up_hotkey();

		auto trickUpSetActionAddr = m_patterns_cache->find_addr(base, "41 B8 D5 07 00 00 C7 44 24 38 00 00 00 00 48");//DevilMayCry5.exe+550BE2
		if (!trickUpSetActionAddr)
		{
			return "Unable to find BossTrickUp.trickUpSetActionAddr pattern.";
		}

		skipTrickUpRet += trickUpSetActionAddr.value();

		if (!install_hook_absolute(trickUpSetActionAddr.value(), m_trickup_action_hook, &detour, &ret, 0x6))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize BossTrickUp.trickUpSetAction";
		}

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override 
	{
		zOffs = cfg.get<float>("BossTrickUp.zOffs").value_or(4.3F);
		distanceOffs = cfg.get<float>("BossTrickUp.distanceOffs").value_or(2.0f);
		angleForwardThreshold = cfg.get<float>("BossTrickUp.angleForwardThreshold").value_or(22.5f);
	}
	void on_config_save(utility::Config& cfg) override 
	{
		cfg.set<float>("BossTrickUp.zOffs", zOffs);
		cfg.set<float>("BossTrickUp.distanceOffs", distanceOffs);
		cfg.set<float>("BossTrickUp.angleForwardThreshold", angleForwardThreshold);
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
	}
};
//clang-format on