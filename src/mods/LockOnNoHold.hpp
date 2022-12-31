#pragma once
#include "Mod.hpp"
#include "Mods.hpp"
#include "PlayerTracker.hpp"
#include "EndLvlHooks.hpp"
#include "InputSystem.hpp"

class LockOnNoHold : public Mod, private EndLvlHooks::IEndLvl
{
private:

	bool _isPlLockedOn = false;

	InputSystem* _inputSystem = nullptr;

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	void on_pl_lock_on_update(uintptr_t threadCtxt, uintptr_t pl)
	{
		if (!cheaton || *(int*)(pl + 0xE64) == 3)
			return;
		if (_inputSystem->is_action_button_down(InputSystem::PadInputGameAction::LockOn))
		{
			if (*(int*)(pl + 0xE64) == 4 && *(bool*)(pl + 0x17F0))//is pl0800 doppel
				return;
			_isPlLockedOn = !_isPlLockedOn;
			auto padInput = *(uintptr_t*)(pl + 0xEF0);
			*(bool*)(padInput + 0x28) = _isPlLockedOn;
		}
	}

	void reset(EndLvlHooks::EndType type) override
	{
		_isPlLockedOn = false;
	}

	void after_all_inits() override
	{
		_inputSystem = static_cast<InputSystem*>(g_framework->get_mods()->get_mod("InputSystem"));
		PlayerTracker::on_pl_lock_on_update_sub(std::make_shared<Events::EventHandler<LockOnNoHold, uintptr_t, uintptr_t>>(this, &LockOnNoHold::on_pl_lock_on_update));
	}

public:
	LockOnNoHold() = default;

	~LockOnNoHold()
	{
		PlayerTracker::on_pl_lock_on_update_unsub(std::make_shared<Events::EventHandler<LockOnNoHold, uintptr_t, uintptr_t>>(this, &LockOnNoHold::on_pl_lock_on_update));
	}

	static inline bool cheaton = true;

	std::string_view get_name() const override
	{
		return "LockOnNoHold";
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
		m_on_page = Page_QOL;
		m_full_name_string = "Toggle Lock On";
		m_author_string = "V.P.Zadov";
		m_description_string = "Switch lock on state by pressing the lock on button without holding it.";

		set_up_hotkey();

		return Mod::on_initialize();
	}

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override {}
	void on_config_save(utility::Config& cfg) override {}

	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override {}
};
//clang-format on