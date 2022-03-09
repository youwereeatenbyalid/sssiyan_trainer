#pragma once
#include "Mod.hpp"
class JudgementCustomCost : public Mod
{
public:
	static inline uintptr_t ret = 0;

	static inline bool cheaton = false;

	static inline float customCost = 3000.0f;

	JudgementCustomCost() = default;

	static naked void detour()
	{
		__asm {
			cmp byte ptr [JudgementCustomCost::cheaton], 0
			je originalcode

			cheat:
			movss xmm0, [JudgementCustomCost::customCost]
			jmp qword ptr [JudgementCustomCost::ret]

			originalcode:
			movss xmm0, [rdi + 0x68]
			jmp qword ptr [JudgementCustomCost::ret]
		}
	}

	std::string_view get_name() const override { return "JudgementCustomCost"; }
	std::string get_checkbox_name() override { return m_check_box_name; };
	std::string get_hotkey_name() override { return m_hot_key_name; };

	std::optional<std::string> on_initialize() override
	{
		init_check_box_info();
		auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
		m_is_enabled = &cheaton;
		m_on_page = dantesdt;
		m_full_name_string = "Custom Judgement Cost (+)";
		m_author_string = "VPZadov";
		m_description_string = "Set cost for SDT's \"Judgement\" move.";

		set_up_hotkey();

		auto judgementCostAddr = patterns->find_addr(base, "1A 00 00 F3 0F 10 47 68"); //DevilMayCry5.exe+1388CFA(-0x3);
		if (!judgementCostAddr.has_value())
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize JudgementCustomCost.judgementCostAddr";
		}

		if (!install_hook_absolute(judgementCostAddr.value() + 0x3, m_cost_hook, &detour, &ret, 0x5))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize JudgementCustomCost.judgementCost";
		}

		return Mod::on_initialize();
	};

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override
	{
		customCost = cfg.get<float>("JudgementCustomCost.customCost").value_or(3000.0F);
	}
	void on_config_save(utility::Config& cfg) override
	{
		cfg.set<float>("JudgementCustomCost.customCost", customCost);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override
	{
		ImGui::TextWrapped("Judgement cost:");
		UI::SliderFloat("##CustomCost", &customCost, 0, 10000.0f, "%.1f", 1.0F, ImGuiSliderFlags_AlwaysClamp);
	};

private:
	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	};
	std::unique_ptr<FunctionHook> m_cost_hook;

};