#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class BpStageJump : public Mod {
public:
  BpStageJump() = default;
  // mod name string for config

  
  static bool randombosses;
  static bool bossrush;
  static bool endless;
  static enum palace_type_enum { BALANCED, PARTIAL, RANDOM };
  int static palace_type;
  int static random_generator(int low, int high);
  // random_generator(1,7), case switch to return a boss floor
  int static return_boss_floor();
  // return non-boss bp floor
  int static return_normal_floor();
  // Reset counter, palacearray,bossarray
  void static reset_palace();
  // randomize values in an array via swapping
  void static randomize_array(int* array_param, int range_low, int range_high, int rand_low, int rand_high);
  void static randomize_array(int* array_param, int range_low, int range_high);
  // Generate a new palace scenario
  void static generate_palace(int seed);
  // get the next floor
  int static next_floor();

  std::string_view get_name() const override { return "BpStageJump"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  static uintptr_t jmp_ret;
  static uintptr_t jmp_jne;
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
  void on_draw_debug_ui() override;

private:
  // function hook instance for our detour, convinient wrapper
  // around minhook
  void init_check_box_info() override;
  static int palacearray[100];
  static int bossarray[8];
  static int counter;
  std::unique_ptr<FunctionHook> m_function_hook;
};
