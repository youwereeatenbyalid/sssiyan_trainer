#pragma once

#include <windows.h>
#include <spdlog/spdlog.h>

class Mods;

#include "D3D11Hook.hpp"
#include "D3D12Hook.hpp"
#include "DInputHook.hpp"
#include "ControllerHook.hpp"
#include "WindowsMessageHook.hpp"
#include "KeyBinder.hpp"
#include "fw-imgui/imgui_notify.h"
#include "fw-imgui/ui.hpp"
#include "sdk/ReClass.hpp"
#include "sdk/REContext.hpp"


using Microsoft::WRL::ComPtr;

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
    
    const auto& get_controller_state() const { return m_last_controller_state; }

    const auto& get_scale() const { return m_scale; }

    const auto& get_icons() const { return m_icons; }
    
    Address get_module() const { return m_game_module; }

    auto get_thread_context() const { sdk::VM::get()->get_thread_context(); }
    
    bool is_ready() const { return m_game_data_initialized; }

    auto get_menu_key() const { return KeyBinder::GetKeyboardKeys("Menu Key"); }

    auto& get_kcw_buffers() { return m_kcw_buffers; }

    bool& get_kcw_show_state_ref() { return m_kcw_buffers.drawWindow; }

	auto get_last_kc_bind_name() const { return m_kcw_buffers.bindName; }

    void set_current_kc_bind_name(const std::string& bindName) { m_kcw_buffers.bindName = bindName; }

    void set_kcw_name(const std::string& newName) { m_kcw_buffers.windowTitle = newName; }

    void clear_kc_bind_name() { m_kcw_buffers.bindName.clear(); }

	bool is_notif_enable() const { return m_is_notif_enabled; }

    void enable_notifications() { m_is_notif_enabled = true; }

    void disable_notifications() { m_is_notif_enabled = false; }

    void queue_notification(const ImGuiToast& notif);
    
    void on_frame_d3d11();
    void on_frame_d3d12();
    void on_reset();
    void consume_input();
    bool on_message(HWND& wnd, UINT& message, WPARAM& w_param, LPARAM& l_param);
    void on_direct_input_keys(const std::array<uint8_t, 256>& keys);
    void on_gamepad_keys(const HIDGamePadDevice& gamePadDevice);

    void save_trainer_settings(utility::Config& cfg) const;
    void load_trainer_settings(utility::Config& cfg);

    void save_config();
    void load_config();
    
private:
    enum PanelID_ : uint8_t;
    enum SettingsPanelID_ : uint8_t;

    void draw_ui();
    void draw_panel(PanelID_ panelID);
    void draw_settings(SettingsPanelID_ panelID);
    void draw_notifs();

    bool hook_d3d11();
    bool hook_d3d12();
    
    void set_style(const float& scale = 1.0f) noexcept;
    
    bool initialize();
    
    void prepare_tex();
    void initialize_key_bindings();

    // UI
    bool m_draw_ui{ false };
    bool m_do_once_after_ui{ false };
    bool m_is_ui_focused{ false };
    bool m_main_game_cursor_state_buffer{ true };
    bool m_is_internal_message{ false };
    bool m_close_menu_guard{ false };

    uint8_t m_default_menu_key{ DIK_DELETE };
    uint8_t m_default_close_menu_key{ DIK_ESCAPE };
    
    // Key capture window UI
    UI::KCWBuffers m_kcw_buffers;
    UI::KeyMode_t& m_last_key_mode = m_kcw_buffers.keyMode;

    // Really shitty stuff
    enum PanelID_ : uint8_t {
        PanelID_Gameplay = 0,
        PanelID_Scenario,
        PanelID_System,
        PanelID_Nero,
        PanelID_Dante,
        PanelID_Gilver,
        PanelID_Vergil
    };

    enum SettingsPanelID_ : uint8_t
    {
        SettingsPanelID_FocusedMod = 0,
        SettingsPanelID_Trainer
    };

    PanelID_ m_last_focused_panel{ m_focused_mod_panel };
    SettingsPanelID_ m_last_settings_focused_panel{ m_focused_mod_panel };

    //uint8_t m_frame_counter{0};
    //bool m_is_second_frame{false};
    
    bool m_first_frame{ true };
    bool m_is_d3d12{ false };
    bool m_is_d3d11{ false };
    bool m_valid{ false };
    bool m_initialized{ false };
    ImVec2 m_window_pos{ 50, 50 };
    ImVec2 m_window_size{ 1400, 750 };
    float m_scale{ 1.0f };
    std::atomic<bool> m_game_data_initialized{ false };
    
    std::mutex m_input_mutex{};
    
    HWND m_wnd{ 0 };
    HMODULE m_game_module{ 0 };
    
    float m_accumulated_mouse_delta[2]{};
    float m_mouse_delta[2]{};
    std::array<uint8_t, 256> m_last_keys{ 0 };
    uint32_t m_last_controller_state{ 0 };
    std::unique_ptr<D3D11Hook> m_d3d11_hook{};
    std::unique_ptr<D3D12Hook> m_d3d12_hook{};
    std::unique_ptr<WindowsMessageHook> m_windows_message_hook;
    std::unique_ptr<DInputHook> m_dinput_hook;
    std::unique_ptr<ControllerHook> m_controller_hook;
    std::shared_ptr<spdlog::logger> m_logger;
    
    std::string m_error{ "" };

    // Trainer settings
    bool m_is_notif_enabled{ false };
    bool m_save_after_close_ui{ false };
    bool m_remember_focused_panels{ false };
    PanelID_ m_focused_mod_panel{ PanelID_Gameplay };
    SettingsPanelID_ m_focused_settings_panel{ SettingsPanelID_FocusedMod };

    // Game-specific stuff
    std::unique_ptr<Mods> m_mods;
    
private: // D3D11 Init
	void create_render_target_d3d11();
	void cleanup_render_target_d3d11();

private: // D3D12 Init
    bool create_rtv_descriptor_heap_d3d12();
    bool create_srv_descriptor_heap_d3d12(UINT descriptorCount = 1);
    bool create_command_allocator_d3d12();
    bool create_command_list_d3d12();
    void cleanup_render_target_d3d12();
    void create_render_target_d3d12();

private: //DXGI members
	DXGI_SWAP_CHAIN_DESC m_swap_desc{};

private: // D3D11 members
    ComPtr<ID3D11RenderTargetView> m_main_render_target_view_d3d11{};
    UI::Texture2DDX11 m_logo_dx11;

private: // D3D12 members
    struct FrameContext_D3D12 {
      D3D12_CPU_DESCRIPTOR_HANDLE MainRenderTargetDescriptorHandle{};
      ComPtr<ID3D12Resource> MainRenderTargetResource{};
      ComPtr<ID3D12CommandAllocator> CommandAllocator{};
      UINT64 FenceValue{ 0 };
    };
    
    UINT m_buffer_count_d3d12{ 0 };
    std::vector<FrameContext_D3D12> m_frame_context_d3d12{};
    ComPtr<ID3D12DescriptorHeap> m_pd3d_rtv_desc_heap_d3d12{};
    ComPtr<ID3D12DescriptorHeap> m_pd3d_srv_desc_heap_d3d12{};
    ComPtr<ID3D12CommandQueue> m_pd3d_command_queue_d3d12{};
    ComPtr<ID3D12GraphicsCommandList> m_pd3d_command_list_d3d12{};
    //ID3D12Fence* m_pd3d_fence_d3d12{ NULL };
    
private:
    UI::Texture2DDX12 m_logo_dx12;
    
    // Icons
    struct {
        // D3D11
        UI::Texture2DDX11 kbIconDX11;
        UI::Texture2DDX11 kbIconActiveDX11;
        UI::Texture2DDX11 keyIconsDX11;
        
        // D3D12
        UI::Texture2DDX12 kbIconDX12;
        UI::Texture2DDX12 kbIconActiveDX12;
        UI::Texture2DDX12 keyIconsDX12;
    } m_icons;
};

extern std::unique_ptr<ModFramework> g_framework;

