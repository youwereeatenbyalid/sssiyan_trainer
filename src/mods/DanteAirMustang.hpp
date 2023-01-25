#pragma once
#include "Mod.hpp"
#include "Mods.hpp"
#include "InputSystem.hpp"

//clang-format off

class DanteAirMustang : public Mod
{
private:

	enum class MustangInput
	{
		Always,
		LockOn,
		LSBack,
		LSForward
	};

	MustangInput _mustangInput = MustangInput::LSBack;

	InputSystem* _inputSys = nullptr;

	static inline DanteAirMustang* _mod = nullptr;

	std::unique_ptr<FunctionHook> _mustangAirActionHook;

	float _backDeg = 45.0f;
	float _frontDeg = 45.0f;

	void after_all_inits()
	{
		_inputSys = static_cast<InputSystem*>(g_framework->get_mods()->get_mod("InputSystem"));
	}

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	static inline naked void detour()
	{
		__asm
		{
			cmp byte ptr [DanteAirMustang::cheaton], 01
			je cheat

			originalcode:
			mov r8d, 0x86F
			jmp qword ptr [DanteAirMustang::ret]

			cheat:
			push rax
			push rcx
			push rdx
			push rsp
			push r8
			push r9
			push r10
			push r11
			mov rcx, rdi
			sub rsp, 32
			call qword ptr [DanteAirMustang::check_input_asm]
			add rsp, 32
			cmp al, 01
			pop r11
			pop r10
			pop r9
			pop r8
			pop rsp
			pop rdx
			pop rcx
			pop rax
			jne originalcode
			mov r8d, 0x86E
			jmp qword ptr [DanteAirMustang::ret]
		}
	}

public:
	DanteAirMustang()
	{
		_mod = this;
	}

	static inline bool cheaton = true;

	static inline uintptr_t ret = 0;

	static bool check_input_asm(uintptr_t plDante)
	{
		if (plDante == 0)
			return false;
		auto padInput = *(uintptr_t*)(plDante + 0xEF0);
		switch (_mod->_mustangInput)
		{
			case MustangInput::Always:
				return true;
			case MustangInput::LockOn:
			{
				return _mod->_inputSys->is_action_button_pressed(padInput, InputSystem::PadInputGameAction::LockOn);
			}
			case MustangInput::LSBack:
			{
				return _mod->_inputSys->is_back_input((REManagedObject*)padInput, _mod->_backDeg);
			}
			case MustangInput::LSForward:
			{
				return _mod->_inputSys->is_front_input((REManagedObject*)padInput, _mod->_frontDeg);
			}
			default:
				return false;
		}
		return false;
	}

	std::string_view get_name() const override
	{
		return "DanteAirMustang";
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
		m_full_name_string = "Grounded Mustang in Air";
		m_author_string = "V.P.Zadov";
		m_description_string = "Perform the grounded version of Mustang while airborne.";

		set_up_hotkey();

		auto airMustangAirSetActionCodeAddr = m_patterns_cache->find_addr(base, "41 B8 6F 08 00 00 C7");//DevilMayCry5.exe+198F3FF
		if (!airMustangAirSetActionCodeAddr)
		{
			return "Unable to find DanteAirMustang.airMustangAirSetActionCodeAddr pattern.";
		}

		if (!install_hook_absolute(airMustangAirSetActionCodeAddr.value(), _mustangAirActionHook, &detour, &ret, 0x6))
		{
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize DanteAirMustang.airMustangSetActionCode";
		}

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override
	{
		_mustangInput = (DanteAirMustang::MustangInput)cfg.get<int>("DanteAirMustang._mustangInput").value_or((int)DanteAirMustang::MustangInput::LSBack);

		_frontDeg = cfg.get<float>("DanteAirMustang._frontDeg").value_or(45.0f);
		_backDeg = cfg.get<float>("DanteAirMustang._backDeg").value_or(45.0f);
	}

	void on_config_save(utility::Config& cfg) override
	{
		cfg.set<int>("DanteAirMustang._mustangInput", (int)_mustangInput);

		cfg.set<float>("DanteAirMustang._backDeg", _backDeg);
		cfg.set<float>("DanteAirMustang._frontDeg", _frontDeg);
	}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override 
	{
		ImGui::TextWrapped("Selected input for alternate Mustang:");
		ImGui::RadioButton("Always Swap", (int*)&_mustangInput, (int)MustangInput::Always); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
		ImGui::RadioButton("Holding Lock On", (int*)&_mustangInput, (int)MustangInput::LockOn); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
		ImGui::RadioButton("Left Stick Back", (int*)&_mustangInput, (int)MustangInput::LSBack); ImGui::SameLine(); ImGui::Spacing(); ImGui::SameLine();
		ImGui::RadioButton("Left Stick Forward", (int*)&_mustangInput, (int)MustangInput::LSForward);

		ImGui::TextWrapped("Back input stick threshold angle:");
		UI::SliderFloat("##_backDeg", &_backDeg, 15.0f, 65.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);

		ImGui::TextWrapped("Forward input stick threshold angle:");
		UI::SliderFloat("##_frontDeg", &_frontDeg, 15.0f, 65.0f, "%.1f", 1.0f, ImGuiSliderFlags_AlwaysClamp);
	}
};