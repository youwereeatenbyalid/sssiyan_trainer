#pragma once
#include "Mod.hpp"

//clang-format off

class NoRoundtripCallback : public Mod
{
private:

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	std::unique_ptr<FunctionHook> m_vergil_rt_hook;
	std::unique_ptr<FunctionHook> m_dante_rb_rt_hook;
	std::unique_ptr<FunctionHook> m_dante_sd_rt_hook;

public:
	NoRoundtripCallback() = default;

	static inline bool cheaton = false;
	static inline bool isFeSkipEnabled = false;
	static inline bool isRbSkipEnabled = false;
	static inline bool isSdSkipEnabled = false;

	static inline uintptr_t FeRet = 0;
	static inline uintptr_t FeSkip = 0;
	static inline uintptr_t RbRet = 0;
	static inline uintptr_t RbSkip = 0;
	static inline uintptr_t SdRet = 0;
	static inline uintptr_t SdSkip = 0;

	static naked void fe_back_detour()
	{
		__asm {
			movss[rsp + 0x20], xmm1
			cmp byte ptr [NoRoundtripCallback::cheaton], 0
			je originalcode
			cmp byte ptr [NoRoundtripCallback::isFeSkipEnabled], 0
			je originalcode
			jmp qword ptr[NoRoundtripCallback::FeSkip]

			originalcode:
			jmp qword ptr [NoRoundtripCallback::FeRet]
		}
	}

	static naked void rb_back_detour()
	{
		__asm {
			movss [rsp + 0x20], xmm1
			cmp byte ptr [NoRoundtripCallback::cheaton], 0
			je originalcode
			cmp byte ptr [NoRoundtripCallback::isRbSkipEnabled], 0
			je originalcode
			jmp qword ptr [NoRoundtripCallback::RbSkip]

			originalcode:
			jmp qword ptr [NoRoundtripCallback::RbRet]
		}
	}

	static naked void sd_back_detour()
	{
		__asm {
			movss [rsp + 0x20], xmm1
			cmp byte ptr [NoRoundtripCallback::cheaton], 0
			je originalcode
			cmp byte ptr [NoRoundtripCallback::isSdSkipEnabled], 0
			je originalcode
			jmp qword ptr [NoRoundtripCallback::SdSkip]

			originalcode:
			jmp qword ptr [NoRoundtripCallback::SdRet]
		}
	}


	std::string_view get_name() const override
	{
		return "NoRoundtripCallback";
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
		m_on_page = Page_Animation;
		m_full_name_string = "Disable Roundtrip Callback Animation";
		m_author_string = "V.P.Zadov";
		m_description_string = "Disable hand callback animation for Dante's/Vergil's roundtrip move.";

		auto FEBackAddr = m_patterns_cache->find_addr(base, "F3 0F 11 4C 24 20 E8 AF F8");//DevilMayCry5.exe+C85DE6
		if (!FEBackAddr)
		{
			return "Unable to find NoRoundtripCallback.FEBackAddr pattern.";
		}

		auto RbBackAddr = m_patterns_cache->find_addr(base, "F3 0F 11 4C 24 20 E8 29 4A");//DevilMayCry5.exe+1261CFC
		if (!RbBackAddr)
		{
			return "Unable to find NoRoundtripCallback.RbBackAddr pattern.";
		}

		auto SdBackAddr = m_patterns_cache->find_addr(base, "F3 0F 11 4C 24 20 E8 99 FF");//DevilMayCry5.exe+16E678C
		if (!SdBackAddr)
		{
			return "Unable to find NoRoundtripCallback.SdBackAddr pattern.";
		}

		FeSkip = FEBackAddr.value() + 0xB;
		RbSkip = RbBackAddr.value() + 0xB;
		SdSkip = SdBackAddr.value() + 0xB;


		if (!install_hook_absolute(FEBackAddr.value(), m_vergil_rt_hook, &fe_back_detour, &FeRet, 0x6))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize NoRoundtripCallback.FEBack";
		}

		if (!install_hook_absolute(RbBackAddr.value(), m_dante_rb_rt_hook, &rb_back_detour, &RbRet, 0x6))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize NoRoundtripCallback.RbBack";
		}

		if (!install_hook_absolute(SdBackAddr.value(), m_dante_sd_rt_hook, &sd_back_detour, &SdRet, 0x6))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize NoRoundtripCallback.SdBack";
		}

		set_up_hotkey();

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override
	{
		isFeSkipEnabled = cfg.get<bool>("NoRoundtripCallback.isFeSkipEnabled").value_or(true);
		isRbSkipEnabled = cfg.get<bool>("NoRoundtripCallback.isRbSkipEnabled").value_or(true);
		isSdSkipEnabled = cfg.get<bool>("NoRoundtripCallback.isSdSkipEnabled").value_or(true);

	}
	void on_config_save(utility::Config& cfg) override
	{
		cfg.set<bool>("NoRoundtripCallback.isFeSkipEnabled", isFeSkipEnabled);
		cfg.set<bool>("NoRoundtripCallback.isEb", isRbSkipEnabled);
		cfg.set<bool>("NoRoundtripCallback.isSdSkipEnabled", isSdSkipEnabled);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override
	{
		ImGui::Checkbox("Skip FE Round Trip callback animation", &isFeSkipEnabled);
		ImGui::Checkbox("Skip Rebellion Round Trip callback animation", &isRbSkipEnabled);
		ImGui::Checkbox("Skip Sparda Round Trip callback animation", &isSdSkipEnabled);
	}
};
//clang-format on