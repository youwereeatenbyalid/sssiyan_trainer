#pragma once
#include "Mod.hpp"
#include "GameInput.hpp"
#include "AirMoves.hpp"
#include "VergilTrickTrailsEfx.hpp"
#include "events/Events.hpp"
#include "events/EventArgs.hpp"

namespace gf = GameFunctions;

class ActionEventArgs : Events::EventArgs
{
	const char* currentAction;
};

class PlSetActionData : public Mod
{
public:
	static inline const unsigned int ACTION_STR_LENGTH = 65;

private:
	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	};

	static inline PlSetActionData* _mod = nullptr;

	static inline std::array<char, ACTION_STR_LENGTH> actionStr;
	static inline std::array<char, ACTION_STR_LENGTH> realActionStr;
	static inline std::array<char, ACTION_STR_LENGTH> doppelActionStr;
	static inline std::array<char, ACTION_STR_LENGTH> doppelRealActionStr;
	static inline std::array<char, ACTION_STR_LENGTH> plBossActionStr;
	static inline std::array<char, ACTION_STR_LENGTH> realPlBossActionStr;

	static inline unsigned int actualLength = 1;
	static inline unsigned int doppelActualLength = 1;
	static inline unsigned int plBossActualLength = 1;

	static inline unsigned int realActionLength = 1;
	static inline unsigned int doppelRealActionLength = 1;
	static inline unsigned int realPlBossActionLength = 1;


	static inline bool isDoppelNow = false;

	std::unique_ptr<FunctionHook> cur_action_hook;

	static bool cmp_action_str(const std::array<char, ACTION_STR_LENGTH>& srcStr, int srcRealLength, const char* str)
	{
		size_t strLen = strlen(str);
		if (strLen != srcRealLength)
			return false;
		for (int i = 0; i < srcRealLength; i++)
		{
			if (srcStr[i] != str[i])
				return false;
		}
		return true;
	}

	static inline Events::Event<const std::array<char, ACTION_STR_LENGTH>*, uintptr_t, uintptr_t, uintptr_t> newAction{ true };
	static inline Events::Event<const std::array<char, ACTION_STR_LENGTH>*, uintptr_t, uintptr_t, uintptr_t> newPlBossAction{ true };

	//static inline Events::Event<const std::array<char, ACTION_STR_LENGTH>&, uintptr_t> doppelNewAction {};

public:

	PlSetActionData() = default;

	template<typename T>
	static void new_action_event_sub(std::shared_ptr<Events::EventHandler<T, const std::array<char, ACTION_STR_LENGTH>*, uintptr_t, uintptr_t, uintptr_t>> handler)
	{
		if (handler != nullptr)
			newAction.subscribe(handler);
	}

	template<typename T>
	static void new_action_event_unsub(std::shared_ptr<Events::EventHandler<T, const std::array<char, ACTION_STR_LENGTH>*, uintptr_t, uintptr_t, uintptr_t>> handler)
	{
		if (handler != nullptr)
			newAction.unsubscribe(handler);
	}

	template<typename T>
	static void new_pl_boss_action_event_sub(std::shared_ptr<Events::EventHandler<T, const std::array<char, ACTION_STR_LENGTH>*, uintptr_t, uintptr_t, uintptr_t>> handler)
	{
		if (handler != nullptr)
			newPlBossAction.subscribe(handler);
	}

	template<typename T>
	static void new_pl_boss_action_event_unsub(std::shared_ptr<Events::EventHandler<T, const std::array<char, ACTION_STR_LENGTH>*, uintptr_t, uintptr_t, uintptr_t>> handler)
	{
		if (handler != nullptr)
			newPlBossAction.unsubscribe(handler);
	}

	std::array<char, ACTION_STR_LENGTH>& get_cur_action_str() const noexcept { return actionStr; }
	/// <summary>
	/// Action string that doesn't update when action is "None"
	/// </summary>
	/// <returns></returns>
	std::array<char, ACTION_STR_LENGTH>& get_cur_real_action_str() const noexcept { return realActionStr; }

	std::array<char, ACTION_STR_LENGTH>& get_cur_real_pl_boss_action_str() const noexcept { return realPlBossActionStr; }


	inline unsigned int get_actual_str_length() const noexcept { return actualLength; }

	inline unsigned int get_doppelactual_str_length() const noexcept { return doppelActualLength; }

	inline unsigned int get_pl_boss_actual_str_length() const noexcept { return plBossActualLength; }

	inline unsigned int get_real_actual_str_length() const noexcept { return realActionLength; }

	inline unsigned int get_doppel_real_actual_str_length() const noexcept { return doppelRealActionLength; }

	inline unsigned int get_pl_boss_real_actual_str_length() const noexcept { return realPlBossActionLength; }


	static inline uintptr_t curMoveStrRet = 0;

	static inline bool cmp_cur_action(const char* str)
	{
		return cmp_action_str(actionStr, actualLength, str);
	}

	static inline bool cmp_doppel_cur_action(const char* str)
	{
		return cmp_action_str(doppelActionStr, doppelActualLength, str);
	}

	static inline bool cmp_real_cur_action(const char* str)
	{
		return cmp_action_str(realActionStr, realActionLength, str);
	}

	static inline bool cmp_real_pl_boss_cur_action(const char* str)
	{
		return cmp_action_str(realPlBossActionStr, realPlBossActionLength, str);
	}

	static inline bool cmp_doppel_real_cur_action(const char* str)
	{
		return cmp_action_str(doppelRealActionStr, doppelRealActionLength, str);
	}

	static void set_action_hook(uintptr_t vm, uintptr_t curPl, uintptr_t dotNetString, uint32_t layerNo, float startFrame, float interpolationFrame, int interpolationMode, int interpolationCurve,
		bool isImmediate, bool passSelect, bool isPuppetTransition, int actionPriority)
	{
		_mod->cur_action_hook->get_original<decltype(set_action_hook)>()(vm, curPl, dotNetString, layerNo, startFrame, interpolationFrame, interpolationMode, interpolationCurve, isImmediate,
			passSelect, isPuppetTransition, actionPriority);
		if (*(int*)(curPl + 0xE64) == 3 || *(int*)(curPl + 0x108) == 1)
		{
			_strset(plBossActionStr.data(), 0);
			gf::StringController::get_str(dotNetString, &plBossActionStr, PlSetActionData::plBossActualLength);
			if (strcmp(plBossActionStr.data(), "None") != 0 && strcmp(plBossActionStr.data(), "Wait") != 0 && strcmp(plBossActionStr.data(), "LongWait") != 0 && strcmp(plBossActionStr.data(), "PutOut") != 0)
			{
				_strset(realPlBossActionStr.data(), 0);
				gf::StringController::get_str(dotNetString, &realPlBossActionStr, PlSetActionData::realPlBossActionLength);
				newPlBossAction.invoke(&realPlBossActionStr, vm, dotNetString, curPl);
			}
		}
		else
		{
			bool isPl800Doppel = *(int*)(curPl + 0xE64) == 4 && *(bool*)(curPl + 0x17F0);
			if (!isPl800Doppel)
			{
				_strset(actionStr.data(), 0);
				gf::StringController::get_str(dotNetString, &actionStr, PlSetActionData::actualLength);
			}
			else
			{
				_strset(doppelActionStr.data(), 0);
				gf::StringController::get_str(dotNetString, &doppelActionStr, PlSetActionData::doppelActualLength);
			}

			if (strcmp(actionStr.data(), "None") != 0 && strcmp(actionStr.data(), "Wait") != 0 && strcmp(actionStr.data(), "LongWait") != 0 && strcmp(actionStr.data(), "PutOut") != 0
				&& strcmp(actionStr.data(), "PutAwayYamato") && strcmp(actionStr.data(), "PutAwayForceedge"))
			{
				_strset(realActionStr.data(), 0);
				strcpy(realActionStr.data(), actionStr.data());
				realActionLength = actualLength;
				newAction.invoke(&realActionStr, vm, dotNetString, curPl);
			}
		}


		//-------Place moves cheats here-------//

		if (AirMoves::cheaton)
			AirMoves::str_cur_action_asm(curPl, dotNetString);

		if (VergilTrickTrailsEfx::cheaton)
			VergilTrickTrailsEfx::set_trick_efx(dotNetString, curPl);

		//-------------------------------------//

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

		_mod = this;

		auto plSetActionAddr = m_patterns_cache->find_addr(base, "CC CC CC CC 48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 41 56 48 83 EC 60 80 BC");//DevilMayCry5.app_Player__setAction171195 (-0x4)
		if (!plSetActionAddr)
		{
			return "Unable to find PlSetActionData.plSetActionAddr pattern.";
		}

		cur_action_hook = std::make_unique<FunctionHook>(plSetActionAddr.value() + 0x4, &set_action_hook);
		cur_action_hook->create();

		return Mod::on_initialize();
	};
};