#pragma once
#include "Mod.hpp"
class DanteDtNoActivationCost : public Mod
{
public:
	static inline uintptr_t ret = 0;
	static inline uintptr_t jmpJe = 0;

	static inline bool cheaton = false;

	DanteDtNoActivationCost() = default;

	static naked void detour()
	{
		__asm {
			cmp byte ptr [DanteDtNoActivationCost::cheaton], 0
			je originalcode
			jmp qword ptr [DanteDtNoActivationCost::jmpJe]

			originalcode:
			test al, al
			je ret_je
			jmp qword ptr [DanteDtNoActivationCost::ret]

			ret_je:
			jmp qword ptr [DanteDtNoActivationCost::jmpJe]
		}
	}

	std::string_view get_name() const override { return "DanteDtNoActivationCost"; }
	std::string get_checkbox_name() override { return m_check_box_name; };
	std::string get_hotkey_name() override { return m_hot_key_name; };

	std::optional<std::string> on_initialize() override
	{
		init_check_box_info();
		auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
		m_is_enabled = &cheaton;
		m_on_page = Page_DanteCheat;
		m_full_name_string = "Remove DT Activation Cost";
		m_author_string = "V.P.Zadov";
		m_description_string = "Remove the activation cost of entering DT.";

		set_up_hotkey();

		auto activationCostAddr = m_patterns_cache->find_addr(base, "84 C0 0F 84 14 01 00 00 4D"); //DevilMayCry5.exe+19725E8
		if (!activationCostAddr.has_value())
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize DanteDtNoActivationCost.activationCostAddr";
		}

		jmpJe = activationCostAddr.value() + 0x11C;

		if (!install_hook_absolute(activationCostAddr.value(), m_activation_cost_hook, &detour, &ret, 0x8))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize DanteDtNoActivationCost.activationCost";
		}

		return Mod::on_initialize();
	};

private:
	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	};
	std::unique_ptr<FunctionHook> m_activation_cost_hook;

};