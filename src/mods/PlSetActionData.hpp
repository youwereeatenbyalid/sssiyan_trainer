#pragma once
#include "Mod.hpp"
#include "GameInput.hpp"
#include "AirMoves.hpp"
#include "VergilTrickTrailsEfx.hpp"

namespace gf = GameFunctions;

class PlSetActionData : public Mod
{
public:

	static inline const unsigned int ACTION_STR_LENGTH = 50;

	PlSetActionData() = default;

	std::array<char, ACTION_STR_LENGTH> &get_cur_action_str() const noexcept { return actionStr; }
	/// <summary>
	/// Action string that doesn't update when action is "None"
	/// </summary>
	/// <returns></returns>
	std::array<char, ACTION_STR_LENGTH> &get_cur_real_action_str() const noexcept { return realActionStr; }

	inline unsigned int get_actual_str_length() const noexcept { return actualLength; }

	inline unsigned int get_real_actual_str_length() const noexcept { return realActionLength; }

	static inline uintptr_t curMoveStrRet = 0;

	static bool cmp_cur_action(const char* str)
	{
		size_t strLen = strlen(str);
		if(strLen != actualLength)
			return false;
		for (int i = 0; i < actualLength; i++)
		{
			if(actionStr[i] != str[i])
				return false;
		}
		return true;
	}

	static void PlSetActionData::str_cur_action_asm(uintptr_t dotNetString, uintptr_t curPl)
	{
		_strset(actionStr.data(), 0);
		_strset(realActionStr.data(), 0);
		gf::StringController::get_str(dotNetString, &actionStr, PlSetActionData::actualLength);
		if (strcmp(actionStr.data(), "None") != 0)
		{
			strcpy(realActionStr.data(), actionStr.data());
			realActionLength = actualLength;
		}

		//-------Place moves cheats here-------//

		if (AirMoves::cheaton)
			AirMoves::str_cur_action_asm(dotNetString);

		if(VergilTrickTrailsEfx::cheaton)
			VergilTrickTrailsEfx::set_trick_efx(dotNetString, curPl);

		//-------------------------------------//
	}

	static naked void pl_set_action_detour()
	{
		__asm {
			push rax
			push rcx
			push rdx
			push r8
			push r9
			push r10
			push r11
			mov rcx, r8
			sub rsp, 32
			call qword ptr [PlSetActionData::str_cur_action_asm]
			add rsp, 32
			pop r11
			pop r10
			pop r9
			pop r8
			pop rdx
			pop rcx
			pop rax

			originalcode :
			mov [rsp + 0x08], rbx
			jmp qword ptr [PlSetActionData::curMoveStrRet]
		}
	}

	std::string_view get_name() const override
	{
		return "PlSetActionData";
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

		auto plSetActionAddr = m_patterns_cache->find_addr(base, "CC CC CC CC 48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 60 80 BC");//DevilMayCry5.app_Player__setAction171195 (-0x4)
		if (!plSetActionAddr)
		{
			return "Unable to find PlSetActionData.plSetActionAddr pattern.";
		}

		if (!install_hook_absolute(plSetActionAddr.value() + 0x4, cur_action_hook, &pl_set_action_detour, &curMoveStrRet, 0x5))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize PlSetActionData.plSetAction";
		}

		return Mod::on_initialize();
	};

	//// Override this things if you want to store values in the config file
	//void on_config_load(const utility::Config& cfg) override;
	//void on_config_save(utility::Config& cfg) override;

	//// on_frame() is called every frame regardless whether the gui shows up.
	//void on_frame() override {};
	//// on_draw_ui() is called only when the gui shows up
	//// you are in the imgui window here.
	//void on_draw_ui() override {};
	//// on_draw_debug_ui() is called when debug window shows up
	//void on_draw_debug_ui() override {};

private:
	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	};

	static inline std::array<char, ACTION_STR_LENGTH> actionStr;
	static inline std::array<char, ACTION_STR_LENGTH> realActionStr;

	static inline unsigned int actualLength = 1;
	static inline unsigned int realActionLength = 1;

	std::unique_ptr<FunctionHook> cur_action_hook;

};