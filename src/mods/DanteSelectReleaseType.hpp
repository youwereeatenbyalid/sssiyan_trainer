#pragma once
#include "Mod.hpp"

class DanteSelectReleaseType : public Mod
{
private:

	const std::array<const char*, 3> releaseNames
	{
		"Default",
		"Just Release",
		"Royal Release"
	};

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	std::unique_ptr<FunctionHook> m_release_hook;

public:
	DanteSelectReleaseType() = default;

	static inline bool cheaton = true;

	static inline uintptr_t ret = 0;

	static inline int releaseType = 0;

	static naked void release_detour()
	{
		__asm {
			cmp byte ptr [DanteSelectReleaseType::cheaton], 1
			je cheat

			originalcode:
			mov [rax + 0x18], esi
			ret_jmp:
			mov rax, [rbx + 0x50]
			jmp qword ptr [DanteSelectReleaseType::ret]

			cheat:
			push rcx
			mov ecx, dword ptr [releaseType]
			mov [rax + 0x18], ecx
			pop rcx
			jmp ret_jmp
		}
	}

	std::string_view get_name() const override
	{
		return "DanteSelectReleaseType";
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
		m_on_page = Page_DanteCheat;
		m_full_name_string = "Set Royal Guard Release Type (+)";
		m_author_string = "V.P.Zadov";
		m_description_string = "Always perform just release or royal release.";

		set_up_hotkey();

		auto releaseAddr = m_patterns_cache->find_addr(base, "89 70 18 48 8B 43 50 48 8B 97");//DevilMayCry5.exe+197F967
		if (!releaseAddr)
		{
			return "Unable to find DanteSelectReleaseType.releaseAddr pattern.";
		}

		if (!install_hook_absolute(releaseAddr.value(), m_release_hook, &release_detour, &ret, 0x7))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize DanteSelectReleaseType.release";
		}

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override
	{
		releaseType = cfg.get<int>("DanteSelectReleaseType.releaseType").value_or(2);
	}
	void on_config_save(utility::Config& cfg) override
	{
		cfg.set<int>("DanteSelectReleaseType.releaseType", releaseType);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override 
	{
		ImGui::TextWrapped("Release type:");
		ImGui::SameLine();
		bool isSelected = false;
		if (ImGui::BeginCombo("##RGReleaseType", releaseNames[releaseType]))
		{
			for (int i = 1; i < releaseNames.size(); i++)
			{
				isSelected = releaseType == i;
				if (ImGui::Selectable(releaseNames[i], isSelected))
					releaseType = i;
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}
};
//clang-format on