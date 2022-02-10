#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class TextEditor : public Mod {
public:
  TextEditor();
  // mod name string for config
  std::string_view get_name() const override { return "TextEditor"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  static uintptr_t jmp_ret;
  static bool cheaton;
  static void* function_hook(int* a1, wchar_t* a2);
  void* string_replace(int* a1, wchar_t* a2);
  static wchar_t ldkcomp[];
  static wchar_t ldkdescriptioncomp[];

  static wchar_t voidcomp[];
  static wchar_t voidpausecomp[];
  static wchar_t voidconfirmcomp[];
  static wchar_t voiddescriptioncomp[];


  static wchar_t ldkstring[];
  static wchar_t sosstring[];

  static wchar_t ldkdescriptionstring[];
  static wchar_t sosdescriptionstring[];

  static wchar_t swstring[];
  static wchar_t voidstring[];

  static wchar_t swpausestring[];
  static wchar_t voidpausestring[];

  static wchar_t swconfirmstring[];
  static wchar_t voidconfirmstring[];

  static wchar_t swdescriptionstring[];
  static wchar_t voiddescriptionstring[];


  // Override this things if you want to store values in the config file
  // void on_config_load(const utility::Config& cfg) override;
  // void on_config_save(utility::Config& cfg) override;
  // on_frame() is called every frame regardless whether the gui shows up.
  // void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  // void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  // void on_draw_debug_ui() override;
private:

  // function hook instance for our detour, convinient wrapper 
  // around minhook
  void init_check_box_info() override;

  std::unique_ptr<FunctionHook> m_sub_function_hook;
};
