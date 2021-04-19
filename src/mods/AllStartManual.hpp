#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class AllStartManual : public Mod {
public:
  AllStartManual() = default;
  // mod name string for config
  std::string_view get_name() const override { return "AllStartManual"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  static uintptr_t jmp_ret;
  static bool cheaton;

  // Override this things if you want to store values in the config file
  void on_config_load(const utility::Config& cfg) override;
  void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  // void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  // void on_draw_debug_ui() override;

private:
  // function hook instance for our detour, convinient wrapper
  // around minhook
  void init_check_box_info() override;

  std::unique_ptr<FunctionHook> m_function_hook;
};
/*
// ORIGINAL CODE - INJECTION POINT: "DevilMayCry5.exe"+2C723DF

"DevilMayCry5.exe"+2C723C2: 0F 2F D3              -  comiss xmm2,xmm3
"DevilMayCry5.exe"+2C723C5: 72 18                 -  jb DevilMayCry5.exe+2C723DF
"DevilMayCry5.exe"+2C723C7: 0F 2F 5A 04           -  comiss xmm3,[rdx+04]
"DevilMayCry5.exe"+2C723CB: 72 0A                 -  jb DevilMayCry5.exe+2C723D7
"DevilMayCry5.exe"+2C723CD: F3 0F 10 42 08        -  movss xmm0,[rdx+08]
"DevilMayCry5.exe"+2C723D2: 0F 2F C3              -  comiss xmm0,xmm3
"DevilMayCry5.exe"+2C723D5: 73 35                 -  jae DevilMayCry5.exe+2C7240C
"DevilMayCry5.exe"+2C723D7: 32 C0                 -  xor al,al
"DevilMayCry5.exe"+2C723D9: 48 8B 5C 24 08        -  mov rbx,[rsp+08]
"DevilMayCry5.exe"+2C723DE: C3                    -  ret 
// ---------- INJECTING HERE ----------
"DevilMayCry5.exe"+2C723DF: F3 0F 10 42 04        -  movss xmm0,[rdx+04]
// ---------- DONE INJECTING  ----------
"DevilMayCry5.exe"+2C723E4: 0F 2F C2              -  comiss xmm0,xmm2
"DevilMayCry5.exe"+2C723E7: 76 05                 -  jna DevilMayCry5.exe+2C723EE
"DevilMayCry5.exe"+2C723E9: 0F 2F C3              -  comiss xmm0,xmm3
"DevilMayCry5.exe"+2C723EC: 77 E9                 -  ja DevilMayCry5.exe+2C723D7
"DevilMayCry5.exe"+2C723EE: F3 0F 10 42 08        -  movss xmm0,[rdx+08]
"DevilMayCry5.exe"+2C723F3: 0F 2F D0              -  comiss xmm2,xmm0
"DevilMayCry5.exe"+2C723F6: 76 14                 -  jna DevilMayCry5.exe+2C7240C
"DevilMayCry5.exe"+2C723F8: 0F 2F D8              -  comiss xmm3,xmm0
"DevilMayCry5.exe"+2C723FB: EB 0D                 -  jmp DevilMayCry5.exe+2C7240A
"DevilMayCry5.exe"+2C723FD: F3 0F 10 42 04        -  movss xmm0,[rdx+04]
*/
