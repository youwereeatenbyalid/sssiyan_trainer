#pragma once
#include "Mod.hpp"
class VergilNoRoyalForkDelay : public Mod
{
public:
	static inline uintptr_t ret = 0;

	static inline float delay = 0.0f;

	static inline bool cheaton = false;

	VergilNoRoyalForkDelay() = default;

	static naked void detour()
	{
		__asm {
			cmp byte ptr [VergilNoRoyalForkDelay::cheaton], 1
			je cheat
			movss xmm0, [rdi + 0x68]
			jmp qword ptr [VergilNoRoyalForkDelay::ret]

			cheat:
			movss xmm0, [VergilNoRoyalForkDelay::delay]
			jmp qword ptr [VergilNoRoyalForkDelay::ret]
		}
	}

	std::string_view get_name() const override
	{
		return "VergilNoRoyalForkDelay";
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
		m_on_page = Page_VergilCheat;
		m_full_name_string = "No delay after V's royal fork";
		m_author_string = "V.P.Zadov";
		m_description_string = "Remove the delay between royal fork and returning to normal state while performing \"World of V\".";

		set_up_hotkey();

		auto frameTimerAddr = m_patterns_cache->find_addr(base, "F3 0F 10 47 68 0F 5A C0 66 0F 2F C1 77 3E"); //DevilMayCry5.exe+14CC373
		if (!frameTimerAddr.has_value())
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize VergilNoRoyalForkDelay.frameTimerAddr";
		}

		if (!install_hook_absolute(frameTimerAddr.value(), m_frame_timer_hook, &detour, &ret, 0x5))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize VergilNoRoyalForkDelay.frameTimer";
		}

		return Mod::on_initialize();
	};

private:
	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	};
	std::unique_ptr<FunctionHook> m_frame_timer_hook;

};