//#pragma once
#include "Mod.hpp"
#include "GameFunctions/PlVergilDoppel.hpp"
#include "PlayerTracker.hpp"
//clang-format off
namespace f = GameFunctions;
class InstantDoppel : public Mod
{
public:
	InstantDoppel() = default;

	enum DoppelSpeed
	{
		None,
		Fast,
		Default,
		Slow
	};

	static inline DoppelSpeed delay = DoppelSpeed::Fast;

	static inline bool cheaton = true;
	static inline bool isControlledBySpeedState = false;

	static inline uintptr_t ret = 0;
	static inline uintptr_t fVergilSetActionAddr = 0;

	static bool setup_doppel_asm()
	{
		auto vergil = PlayerTracker::vergilentity;
		if (isControlledBySpeedState)
		{
			if(*(DoppelSpeed*)(vergil + 0x18CC) != delay)
				return false;
		}
		f::SetDoppelMode setDoppel{vergil};
		setDoppel(true);
		f::GenerateDoppel genDoppel{vergil};
		genDoppel(f::Vec3(0, 1.2f, 0), false);
		return true;
	}

	static naked void detour()
	{
		__asm {
			cmp byte ptr [InstantDoppel::cheaton], 1
			je cheat

			originalcode:
			call qword ptr [InstantDoppel::fVergilSetActionAddr]
			jmp qword ptr[InstantDoppel::ret]

			cheat:
			cmp r8d, 1
			jne originalcode
			push rax
			push rdx
			push rcx
			push rsi
			push r8
			push r9
			push r10
			push r12
			sub rsp, 32
			call qword ptr [InstantDoppel::setup_doppel_asm]
			add rsp, 32
			cmp al, 0
			pop r12
			pop r10
			pop r9
			pop r8
			pop rsi
			pop rcx
			pop rdx
			pop rax
			je originalcode
			jmp qword ptr[InstantDoppel::ret]
		}
	}

	std::string_view get_name() const override
	{
		return "InstantDoppel";
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
		m_on_page = vergildoppel;
		m_full_name_string = "Instant doppelganger summon";
		m_author_string = "VPZadov";
		m_description_string = "Remove i-frame animation when summoning doppel.";

		set_up_hotkey();


		auto setActionDoppelAddr = patterns->find_addr(base, "E8 33 18 01 00");//DevilMayCry5.exe+533E68
		if (!setActionDoppelAddr)
		{
			return "Unable to find InstantDoppel.setActionDoppelAddr pattern.";
		}

		fVergilSetActionAddr = patterns->find_addr(base, "48 89 5C 24 20 56 57 41 57 48 83 EC 60").value_or(g_framework->get_module().as<uintptr_t>() + 0x5456A0); //DevilMayCry5.exe+5456A0

		if (!install_hook_absolute(setActionDoppelAddr.value(), m_doppel_hook, &detour, &ret, 0x5))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize InstantDoppel.setActionDoppel";
		}

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override 
	{
		isControlledBySpeedState = cfg.get<bool>("InstantDoppel.isControlledBySpeedState").value_or(false);
		delay = (DoppelSpeed)cfg.get<int>("InstantDoppel.Delay").value_or(1);
	}
	void on_config_save(utility::Config& cfg) override 
	{
		cfg.set<bool>("InstantDoppel.isControlledBySpeedState", isControlledBySpeedState);
		cfg.set<int>("InstantDoppel.Delay", delay);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override 
	{
		ImGui::TextWrapped("Select this if you want use this mod only when you activate dt with special delay state:");
		ImGui::Checkbox("Depend on doppel delay", &isControlledBySpeedState);
		bool isSelected = false;
		if(ImGui::BeginCombo("##doppelDelay", delayNames[delay - 1]))
		{
			for (int i = 1; i <= delayNames.size(); i++)
			{
				isSelected = (delay == (DoppelSpeed)i);
				if(ImGui::Selectable(delayNames[i-1], isSelected))
					delay = (DoppelSpeed)i;
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

private:
	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}
	std::unique_ptr<FunctionHook> m_doppel_hook;

	const std::array<char*, 3> delayNames {"Fast", "Default", "Slow"};

};
//clang-format on