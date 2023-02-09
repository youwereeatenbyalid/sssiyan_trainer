#pragma once
#include "Mod.hpp"
#include "PlayerTracker.hpp"

//clang-format off

class VergilGuardSlowMotion : public Mod
{
private:

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	void on_pl0800_guard_check(uintptr_t threadCntxt, uintptr_t vergil, uintptr_t hitCtrilDamageInfo, /*bool* isSkipCall, bool* isUseCallRes,*/ int* res)//Issiue with Dante's and pl0300 shells(
	{
		if (!cheaton || vergil == 0 || *(int*)(vergil + 0x1B58) == 0)
			return;
		if (hitCtrilDamageInfo != 0 && *(int*)(hitCtrilDamageInfo + 0x12C) != 1)
			*res = 6;
		/**isSkipCall = false;
		*isUseCallRes = false;
		*res = 6;*/
	}

public:
	VergilGuardSlowMotion() = default;

	~VergilGuardSlowMotion()
	{
		/*PlayerTracker::on_pl0800_guard_check_unsub(std::make_shared<Events::EventHandler<VergilGuardSlowMotion, uintptr_t,
			uintptr_t, bool*, bool*, int*>>(this, &VergilGuardSlowMotion::on_pl0800_guard_check));*/
		PlayerTracker::after_pl0800_guard_check_unsub(std::make_shared<Events::EventHandler<VergilGuardSlowMotion, uintptr_t, uintptr_t,
			uintptr_t, int*>>(this, &VergilGuardSlowMotion::on_pl0800_guard_check));
	}

	bool cheaton = true;

	std::string_view get_name() const override
	{
		return "VergilGuardSlowMotion";
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
		m_on_page = Page_VergilVFXSettings;
		m_full_name_string = "Slow motion on guard";
		m_author_string = "V.P. Zadov";
		m_description_string = "Enable slow motion when Vergil blocks enemy attack like boss Vergil do on the first block.";

		set_up_hotkey();

		/*PlayerTracker::on_pl0800_guard_check_sub(std::make_shared<Events::EventHandler<VergilGuardSlowMotion, uintptr_t, 
			uintptr_t, bool*, bool*, int*>>(this, &VergilGuardSlowMotion::on_pl0800_guard_check));*/
		PlayerTracker::after_pl0800_guard_check_sub(std::make_shared<Events::EventHandler<VergilGuardSlowMotion, uintptr_t, uintptr_t,
			uintptr_t, int*>>(this, &VergilGuardSlowMotion::on_pl0800_guard_check));

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