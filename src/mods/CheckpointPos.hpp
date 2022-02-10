#pragma once
#include "Mod.hpp"
#include <map>
class CheckpointPos : public Mod
{
public:
  static bool cheaton;
  static bool isCustomPos;
  static bool isNoBoss;

  static uintptr_t restartPos_ret;
  static uintptr_t startPos_ret;


  static uint32_t missionN;
  static Vector3f newRestartPos;
  static Vector3f customPos;
  static Vector3f playerPos;

  struct CheckpointPosData {
    int mNumber;
    Vector3f pos;
    CheckpointPosData(int mNum, Vector3f coords) : mNumber(mNum), pos(coords){}
  };

  static Vector3f get_player_coords();


  static inline std::array<CheckpointPosData, 15> mPosData {//x, y, z
      CheckpointPosData(0, Vector3f(0.1704f, -80.5705f, -5.2691f)),
      CheckpointPosData(1, Vector3f(1.63f, -64.902f, - 0.091f)),
      CheckpointPosData(2, Vector3f(-49.079f, -350.258f, 1.764f)),
      CheckpointPosData(3, Vector3f(-307.771f, -307.061f, -10.386f)),
      CheckpointPosData(4, Vector3f(211.002f, -555.957f, - 33.852f)),
      CheckpointPosData(5, Vector3f(282.346f, -65.189f, - 42.138f)),
      CheckpointPosData(7, Vector3f(9.225, -609.819f, 32.107f)),
      CheckpointPosData(8, Vector3f(0.183f, -81.354f, 1774.665f)),
      CheckpointPosData(10, Vector3f(-43.536f, -121.236f, -31.105f)),
      CheckpointPosData(11, Vector3f(281.389f, -3.210f, 4.441f)),
      CheckpointPosData(12, Vector3f(-1233.897f, -59.512f, 65.877f)),
      CheckpointPosData(14, Vector3f(18.932f, 594.113f, 188.030f)),
      CheckpointPosData(15, Vector3f(317.828f, -1243.324f, 31.173f)),
      CheckpointPosData(16, Vector3f(373.906f, -1296.405f, -137.827f)),
      CheckpointPosData(18, Vector3f(242.011f, -164.744f, 325.294))
  };

  CheckpointPos() = default;

  std::string_view get_name() const override { return "CheckpointPos"; }
  std::string get_checkbox_name() override { return m_check_box_name; };
  std::string get_hotkey_name() override { return m_hot_key_name; };

  std::optional<std::string> on_initialize() override;

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
  void init_check_box_info() override;
  static uintptr_t get_player_coords_ptr(uintptr_t addr);
  static bool is_null_ptr(uintptr_t ptr);
  std::unique_ptr<FunctionHook> m_checkpointpos_hook;
  std::unique_ptr<FunctionHook> m_startpos_hook;
};

