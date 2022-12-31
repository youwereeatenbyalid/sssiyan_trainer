#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class VergilInfSDT : public Mod {
public:
  VergilInfSDT()
  {
	  _mod = this;
  }
  // mod name string for config
  std::string_view get_name() const override { return "VergilInfSDT"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  static inline bool cheaton = false;

private:

  static inline VergilInfSDT* _mod = nullptr;
  // function hook instance for our detour, convinient wrapper
  // around minhook
  void init_check_box_info() override;

  static void dt_update_hook(uintptr_t threadCntx, uintptr_t pl0800);

  std::unique_ptr<FunctionHook> m_function_hook1;
  std::unique_ptr<FunctionHook> m_function_hook2;
  std::unique_ptr<FunctionHook> _pl0800DtUpdateHook;
};
