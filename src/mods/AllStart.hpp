#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class AllStart : public Mod {
public:
  AllStart() = default;
  // mod name string for config
  std::string_view get_name() const override { return "AllStart"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  static uintptr_t jmp_ret;
  static uintptr_t jmp_initial;
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

/*{
// ORIGINAL CODE - INJECTION POINT: "DevilMayCry5.exe"+964B06

"DevilMayCry5.exe"+964AD9: 48 83 78 18 00           -  cmp qword ptr [rax+18],00
"DevilMayCry5.exe"+964ADE: 75 3B                    -  jne DevilMayCry5.exe+964B1B
"DevilMayCry5.exe"+964AE0: 33 C0                    -  xor eax,eax
"DevilMayCry5.exe"+964AE2: 48 C7 47 50 00 00 00 00  -  mov qword ptr [rdi+50],00000000
"DevilMayCry5.exe"+964AEA: 48 89 47 60              -  mov [rdi+60],rax
"DevilMayCry5.exe"+964AEE: 48 89 47 74              -  mov [rdi+74],rax
"DevilMayCry5.exe"+964AF2: 66 89 47 7C              -  mov [rdi+7C],ax
"DevilMayCry5.exe"+964AF6: 88 47 7E                 -  mov [rdi+7E],al
"DevilMayCry5.exe"+964AF9: C7 47 5A 00 00 00 00     -  mov [rdi+5A],00000000		// GunStart
"DevilMayCry5.exe"+964B00: 66 C7 47 58 00 00        -  mov word ptr [rdi+58],0000	// JumpStart
// ---------- INJECTING HERE ----------
"DevilMayCry5.exe"+964B06: 66 C7 47 5E 00 00        -  mov word ptr [rdi+5E],0000	// AllStart
// ---------- DONE INJECTING  ----------
"DevilMayCry5.exe"+964B0C: 48 C7 47 68 FF FF FF FF  -  mov qword ptr [rdi+68],FFFFFFFFFFFFFFFF
"DevilMayCry5.exe"+964B14: C7 47 70 FF FF FF FF     -  mov [rdi+70],FFFFFFFF
"DevilMayCry5.exe"+964B1B: 48 8B 5C 24 30           -  mov rbx,[rsp+30]
"DevilMayCry5.exe"+964B20: 48 83 C4 20              -  add rsp,20
"DevilMayCry5.exe"+964B24: 5F                       -  pop rdi
"DevilMayCry5.exe"+964B25: C3                       -  ret 
"DevilMayCry5.exe"+964B26: CC                       -  int 3 
"DevilMayCry5.exe"+964B27: CC                       -  int 3 
"DevilMayCry5.exe"+964B28: CC                       -  int 3 
"DevilMayCry5.exe"+964B29: CC                       -  int 3 
}*/
