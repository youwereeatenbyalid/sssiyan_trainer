#pragma once
#include "Mod.hpp"
#include "ImGuiExtensions/ImGuiExtensions.h"
class VergilGuardYamatoBlock : public Mod
{
public:
	static inline bool cheaton = false;
	static inline bool isSelectEfx = false;

	static inline uintptr_t ret = 0;
	static inline uintptr_t efxRet = 0;

	static inline int concEfx = 2;

	VergilGuardYamatoBlock() = default;

	std::string_view get_name() const override
	{
		return "VergilGuardYamatoBlock";
	}
	std::string get_checkbox_name() override
	{
		return m_check_box_name;
	};
	std::string get_hotkey_name() override
	{
		return m_hot_key_name;
	};

	std::optional<std::string> on_initialize() override;
	void on_draw_ui() override;
	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config & cfg) override;
	void on_config_save(utility::Config & cfg) override;

private:
	void init_check_box_info() override;
	std::shared_ptr<Detour_t> m_action_detour;
	std::shared_ptr<Detour_t> m_efx_detour;
};

