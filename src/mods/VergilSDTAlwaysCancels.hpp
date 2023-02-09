#pragma once
#include "Mod.hpp"

//clang-format off

class VergilSDTAlwaysCancels : public Mod
{
private:

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	std::unique_ptr<FunctionHook> m_full_sdt_trans_hook;

public:
	VergilSDTAlwaysCancels() = default;

	static inline bool cheaton = true;

	static inline uintptr_t ret = 0;

	static naked void detour()
	{
		__asm {
			cmp byte ptr [VergilSDTAlwaysCancels::cheaton], 1
			je cheat

			originalcode:
			movzx ecx ,al
			mov rax, [rbx+0x50]
			jmp qword ptr [VergilSDTAlwaysCancels::ret]

			cheat:
			mov al, 1
			jmp originalcode
		}
	}

	std::string_view get_name() const override
	{
		return "VergilSDTAlwaysCancels";
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
		m_on_page = Page_VergilSDT;
		m_full_name_string = "Force SDT activation animation";
		m_author_string = "V.P.Zadov";
		m_description_string = "Forces the SDT activation animation to play even if Vergil is mid-move.";

		set_up_hotkey();

		auto shortSdtTransAddr = m_patterns_cache->find_addr(base, "E6 FE FF 0F B6 C8 48 8B 43 50"); //DevilMayCry5.exe+52BF79 (-0x3)
		if (!shortSdtTransAddr)
		{
			return "Unable to find VergilSDTAlwaysCancels.shortSdtTransAddr pattern.";
		}

		if (!install_hook_absolute(shortSdtTransAddr.value() + 0x3, m_full_sdt_trans_hook, &detour, &ret, 0x7))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize VergilSDTAlwaysCancels.shortSdtTrans";
		}

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override
	{
	}
	void on_config_save(utility::Config& cfg) override
	{
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override
	{
	}
};
//clang-format on