#pragma once
#include "Mod.hpp"
#include "PlayerTracker.hpp"

//clang-format off

class Pl0000SlowWorldStop : public Mod
{
private:

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	void after_pl0000_slow_world_start(uintptr_t threadCtxt, uintptr_t fsmSlowWorld, uintptr_t behaviorTreeArg)
	{
		if (!cheaton || *(uintptr_t*)(fsmSlowWorld + 0x38) == 0)
			return;
		auto pl = *(uintptr_t*)(fsmSlowWorld + 0x38);
		if (*(int*)(pl + 0xE64) != 0)
			return;
		auto qsRoomShel = *(uintptr_t*)(fsmSlowWorld + 0x20);
		if (qsRoomShel == 0)
			return;
		auto qsController = *(uintptr_t*)(qsRoomShel + 0x448);
		if (qsController == 0)
			return;
		*(int*)(qsController + 0x60) = 2;
	}

	void after_all_inits() override
	{
		PlayerTracker::after_pl0000_quicksilver_slow_world_action_start_sub(std::make_shared<Events::EventHandler<Pl0000SlowWorldStop, uintptr_t, uintptr_t, uintptr_t>>(this,
			&Pl0000SlowWorldStop::after_pl0000_slow_world_start));
	}

public:
	Pl0000SlowWorldStop() = default;

	~Pl0000SlowWorldStop()
	{
		PlayerTracker::after_pl0000_quicksilver_slow_world_action_start_unsub(std::make_shared<Events::EventHandler<Pl0000SlowWorldStop, uintptr_t, uintptr_t, uintptr_t>>(this,
			&Pl0000SlowWorldStop::after_pl0000_slow_world_start));
	}

	static inline bool cheaton = true;

	std::string_view get_name() const override
	{
		return "Pl0000SlowWorldStop";
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
		m_on_page = Page_Breaker;
		m_full_name_string = "Slow World Works Like Bangle Of Time";
		m_author_string = "V.P.Zadov";
		m_description_string = "Ragtime's \"Slow World\" Brealage completely freezes time for enemies like in DMC1.";

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