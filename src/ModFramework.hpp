#pragma once

#include <spdlog/spdlog.h>

class Mods;

#include "D3D11Hook.hpp"
#include "D3D12Hook.hpp"
#include "DInputHook.hpp"
#include "WindowsMessageHook.hpp"
#include "fw-imgui/ui.hpp"

// Global facilitator
class ModFramework {
public:
  ModFramework();
  virtual ~ModFramework();

  bool is_valid() const { return m_valid; }

  bool is_dx11() const { return m_is_d3d11; }

  bool is_dx12() const { return m_is_d3d12; }

  const auto& get_mods() const { return m_mods; }

  const auto& get_keyboard_state() const { return m_last_keys; }

  Address get_module() const { return m_game_module; }

  bool is_ready() const { return m_game_data_initialized; }

  uint8_t get_menu_key() const { return m_menu_key; }

  void on_frame_d3d11();
  void on_frame_d3d12();
  void on_reset();
  void consume_input();
  bool on_message(HWND wnd, UINT message, WPARAM w_param, LPARAM l_param);
  void on_direct_input_keys(const std::array<uint8_t, 256>& keys);

  void save_config();

private:
  enum PanelID : uint8_t;

  void draw_ui();
  void draw_panel(PanelID focusedPanel);

  bool hook_d3d11();
  bool hook_d3d12();

  void set_style() noexcept;

  bool initialize();

  // UI
  bool m_draw_ui{false};
  bool m_last_draw_ui{m_draw_ui};
  bool m_is_ui_focused{false};
  bool m_cursor_state{true};
  bool m_is_internal_message{false};

  // Really shitty stuff
  enum PanelID : uint8_t {
    PanelID_Gameplay = 0,
    PanelID_Scenario,
    PanelID_System,
    PanelID_Nero,
    PanelID_Dante,
    PanelID_Gilver,
    PanelID_Vergil
  };

  PanelID m_focused_panel{ PanelID_Gameplay };
  PanelID m_last_focused_panel{ m_focused_panel };
  uint8_t m_frame_counter{0};
  bool m_is_second_frame{false};

  bool m_first_frame{true};
  bool m_is_d3d12{false};
  bool m_is_d3d11{false};
  bool m_valid{false};
  bool m_initialized{false};
  ImVec2 m_window_pos{50, 50};
  ImVec2 m_window_size{1400, 750};
  std::atomic<bool> m_game_data_initialized{false};

  std::mutex m_input_mutex{};

  HWND m_wnd{0};
  HMODULE m_game_module{0};
  uint8_t m_menu_key{DIK_DELETE};
  uint8_t m_close_menu_key{DIK_ESCAPE};

  float m_accumulated_mouse_delta[2]{};
  float m_mouse_delta[2]{};
  std::array<uint8_t, 256> m_last_keys{0};
  std::unique_ptr<D3D11Hook> m_d3d11_hook{};
  std::unique_ptr<D3D12Hook> m_d3d12_hook{};
  std::unique_ptr<WindowsMessageHook> m_windows_message_hook;
  std::unique_ptr<DInputHook> m_dinput_hook;
  std::shared_ptr<spdlog::logger> m_logger;

  std::string m_error{""};

  // Game-specific stuff
  std::unique_ptr<Mods> m_mods;

private: // D3D11 Init
  void create_render_target_d3d11();
  void cleanup_render_target_d3d11();


private: // D3D12 Init
  bool create_rtv_descriptor_heap_d3d12();
  bool create_srv_descriptor_heap_d3d12();
  bool create_command_allocator_d3d12();
  bool create_command_list_d3d12();
  void cleanup_render_target_d3d12();
  void create_render_target_d3d12();

private: // D3D11 members
  ID3D11RenderTargetView* m_main_render_target_view_d3d11{nullptr};
  UI::Texture2DDX11 m_logo_dx11;

private: // D3D12 members
  struct FrameContext_D3D12 {
    ID3D12CommandAllocator* CommandAllocator;
    UINT64 FenceValue;
  };

  static int const m_NUM_FRAMES_IN_FLIGHT_D3D12{3};
  static int const m_NUM_BACK_BUFFERS_D3D12{3};
  FrameContext_D3D12 m_frame_context_d3d12[m_NUM_FRAMES_IN_FLIGHT_D3D12]{};
  ID3D12DescriptorHeap* m_pd3d_rtv_desc_heap_d3d12{NULL};
  ID3D12DescriptorHeap* m_pd3d_srv_desc_heap_d3d12{NULL};
  ID3D12CommandQueue* m_pd3d_command_queue_d3d12{NULL};
  ID3D12GraphicsCommandList* m_pd3d_command_list_d3d12{NULL};
  ID3D12Resource* m_main_render_target_resource_d3d12[m_NUM_BACK_BUFFERS_D3D12]{
      NULL};
  D3D12_CPU_DESCRIPTOR_HANDLE m_main_render_target_descriptor_d3d12[m_NUM_BACK_BUFFERS_D3D12]{};

  UI::Texture2DDX12 m_logo_dx12;

  //DXGI members
  DXGI_SWAP_CHAIN_DESC m_swap_desc;
};

extern std::unique_ptr<ModFramework> g_framework;