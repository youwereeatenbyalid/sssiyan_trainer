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
  uintptr_t static jmp_ret;
  bool static cheaton;
  static void* function_hook(int* a1, wchar_t* a2);
  void* string_replace(int* a1, wchar_t* a2);
  wchar_t static ldkcomp[];
  wchar_t static ldkdescriptioncomp[];

  wchar_t static voidcomp[];
  wchar_t static voidpausecomp[];
  wchar_t static voidconfirmcomp[];
  wchar_t static voiddescriptioncomp[];
  

  wchar_t static ldkstring[];
  wchar_t static sosstring[];

  wchar_t static ldkdescriptionstring[];
  wchar_t static sosdescriptionstring[];

  wchar_t static swstring[];
  wchar_t static voidstring[];

  wchar_t static swpausestring[];
  wchar_t static voidpausestring[];
  
  wchar_t static swconfirmstring[];
  wchar_t static voidconfirmstring[];

  wchar_t static swdescriptionstring[];
  wchar_t static voiddescriptionstring[];


  // Override this things if you want to store values in the config file
  void on_config_load(const utility::Config& cfg) override;
  void on_config_save(utility::Config& cfg) override;
  // on_frame() is called every frame regardless whether the gui shows up.
  void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  void on_draw_debug_ui() override;
private:

  // function hook instance for our detour, convinient wrapper 
  // around minhook
  void init_check_box_info() override;

  std::unique_ptr<FunctionHook> m_sub_function_hook;
};
