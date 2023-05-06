#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class VergilAirTrickNoSS : public Mod {
public:
	VergilAirTrickNoSS()
	{
		_mod = this;
	}
	// mod name string for config
	std::string_view get_name() const override { return "VergilAirTrickNoSS"; }
	std::string get_checkbox_name() override { return m_check_box_name; };
	std::string get_hotkey_name() override { return m_hot_key_name; };
	// called by m_mods->init() you'd want to override this
	std::optional<std::string> on_initialize() override;
	static inline bool cheaton = true;

private:
	static inline VergilAirTrickNoSS* _mod = nullptr;

	sdk::REMethodDefinition* _trickTargetCntrStartAirTrickMethod;
	sdk::REMethodDefinition* _pl0800SetAirTrickActionMethod;

	void init_check_box_info() override;

	static bool check_air_trick_hook(uintptr_t threadCntxt, uintptr_t pl0800);

	void on_sdk_init() override
	{
		_trickTargetCntrStartAirTrickMethod = sdk::find_method_definition("app.PlayerVergilPL.TrickTargetController", "startAirTrick(app.PlayerVergilPL)");
		_pl0800SetAirTrickActionMethod = sdk::find_method_definition("app.PlayerVergilPL", "setAirTrickAction(via.GameObject)");
	}

	std::shared_ptr<Detour_t> _checkAirTrickHook;
};
