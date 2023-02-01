#include <spdlog/sinks/basic_file_sink.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <sdk/SDK.hpp>

#include <shellapi.h>

// ours
#include "fw-imgui/imgui_impl_win32.h"
#include "fw-imgui/imgui_impl_dx11.h"
#include "fw-imgui/imgui_impl_dx12.h"

#include "fw-imgui/font_robotomedium.hpp"

#include "ImGuiExtensions/ImGuiExtensions.h"
//#include "dmc5font.hpp"
#include "logo.hpp"
#include "icons.hpp"

#include "utility/Module.hpp"

#include "ExceptionHandler.hpp"

#include "Mod.hpp"
#include "Mods.hpp"

#include "LicenseStrings.hpp"
#include "ModFramework.hpp"

#include "Config.hpp"

// clang-format off

static ImVec2 operator*(const ImVec2& lhs, const float rhs) { return {lhs.x * rhs, lhs.y * rhs}; }

std::unique_ptr<ModFramework> g_framework{};

static std::atomic_uint64_t g_currently_hooking = 0;
static HANDLE g_hooking_thread;

static void hook_init_begin_callback(HANDLE hooking_thread)
{
    g_hooking_thread = hooking_thread;
    ++g_currently_hooking;
}

static void hook_init_end_callback(HANDLE hooking_thread)
{
    --g_currently_hooking;
}

ModFramework::ModFramework()
    : m_game_module{ GetModuleHandle(nullptr) }
{
    using std::filesystem::path;

    FunctionHook::get_hook_begin_callback() = &hook_init_begin_callback;
    FunctionHook::get_hook_end_callback() = &hook_init_end_callback;

    // Making sure the log and configs directories are created
    ::CreateDirectory(path(LOG_FILENAME).parent_path().string().c_str(), nullptr);
    ::CreateDirectory(path(CONFIG_FILENAME).parent_path().string().c_str(), nullptr);
    ::CreateDirectory(path(KEYBIND_CONFIG_FILENAME).parent_path().string().c_str(), nullptr);
    ::CreateDirectory(path(AOB_LIST_FILENAME).parent_path().string().c_str(), nullptr);

    m_logger = spdlog::basic_logger_mt("Collab Trainer", LOG_FILENAME, true);

    spdlog::set_default_logger(m_logger);
    spdlog::flush_on(spdlog::level::info);
    spdlog::info(LOG_ENTRY);

    // Initialize the pattern manager
    Mod::m_patterns_cache = std::make_unique<InitPatternsManager>(AOB_LIST_FILENAME, "IsUsingPatternsList");

    // Loading stuff we saved in the config file
    utility::Config cfg(CONFIG_FILENAME);
    load_trainer_settings(cfg);

    // Preparing custom textures before loading them
    prepare_textures();

    // Initializing the key bindings
    initialize_key_bindings();

#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
#endif

    if (!hook_d3d12()) {
        spdlog::error("Failed to hook D3D12 for initial test.");
        return;
    }

    // Setting up the maps for panel ID
    m_mods_panels_map["Gameplay"] = PanelID_Gameplay;
    m_mods_panels_map["Scenario"] = PanelID_Scenario;
    m_mods_panels_map["System"] = PanelID_System;
    m_mods_panels_map["Nero"] = PanelID_Nero;
    m_mods_panels_map["Dante"] = PanelID_Dante;
    m_mods_panels_map["V"] = PanelID_Gilver;
    m_mods_panels_map["Vergil"] = PanelID_Vergil;
    m_mods_panels_map["Trainer"] = PanelID_Trainer;
}

ModFramework::~ModFramework() {
    if (m_is_d3d11) ImGui_ImplDX11_Shutdown();
    if (m_is_d3d12) ImGui_ImplDX12_Shutdown();

    ImGui_ImplWin32_Shutdown();
    if (m_initialized)
    {
        ImGui::DestroyContext();
    }

    // Save config if the game gets closed
    if (m_save_after_close_ui) {
        save_config();
    }

    KeyBinder::ResetInstance();
}

bool ModFramework::hook_d3d11()
{
    m_d3d11_hook.reset();
    m_d3d11_hook = std::make_unique<D3D11Hook>();
    m_d3d11_hook->on_present([this](D3D11Hook& hook) { on_frame_d3d11(); });
    m_d3d11_hook->on_resize_buffers([this](D3D11Hook&, const UINT& width, const UINT& height) { on_reset(width, height); });

    // Making sure D3D12 is not hooked
    if (!m_is_d3d12) {
        if (m_d3d11_hook->hook()) {
            spdlog::info("Hooked DirectX 11");
            m_valid = true;
            m_is_d3d11 = true;
            return true;
        }

        // We make sure to unhook any unwanted hooks if D3D11 didn't get hooked properly
        if (m_d3d11_hook->unhook())
            spdlog::info("D3D11 unhooked!");
        else
            spdlog::info("Cannot unhook D3D11, this might crash.");

        m_valid = false;
        m_is_d3d11 = false;
        return false;
    }

    return false;
}

bool ModFramework::hook_d3d12()
{
    // Making sure we have D3D12 support (win 7 and 8 don't)
    if (LoadLibraryA("d3d12.dll") == nullptr) {
        spdlog::warn("D3D12 not surpported! Trying to hook D3D11");

        if (!hook_d3d11()) {
            spdlog::error("Failed to hook D3D11.");
            return false;
        }

        return true;
    }

    m_d3d12_hook.reset();
    m_d3d12_hook = std::make_unique<D3D12Hook>();
    m_d3d12_hook->on_present([this](D3D12Hook& hook) { on_frame_d3d12(); });
    m_d3d12_hook->on_resize_buffers([this](D3D12Hook&, const UINT& width, const UINT& height) { on_reset(width, height); });
    //m_d3d12_hook->on_resize_target([this](D3D12Hook& hook) { on_reset(); });
    //m_d3d12_hook->on_create_swap_chain([this](D3D12Hook& hook) { m_d3d12.m_pd3d_command_queue = m_d3d12_hook->get_command_queue(); });

    // Making sure D3D11 is not hooked
    if (!m_is_d3d11) {
        if (m_d3d12_hook->hook()) {
            spdlog::info("Hooked DirectX 12");
            m_valid = true;
            m_is_d3d12 = true;
            return true;
        }

        // We make sure to unhook any unwanted hooks if D3D12 didn't get hooked properly
        if (m_d3d12_hook->unhook())
            spdlog::info("D3D12 Unhooked!");
        else
            spdlog::info("Cannot unhook D3D12, this might crash.");

        m_valid = false;
        m_is_d3d12 = false;
        return false;
    }

    return false;
}

void ModFramework::set_style(const float& scale) noexcept {
    auto& style = ImGui::GetStyle();
    style = ImGuiStyle{};
    ImGui::StyleColorsDark(&style);
    style.ScaleAllSizes(scale);

    style.WindowRounding = 0.0f * scale;
    style.ChildRounding = 0.0f * scale;
    style.PopupRounding = 0.0f * scale;
    style.FrameRounding = 0.0f * scale;
    style.ScrollbarSize = 10.0f * scale;
    style.ScrollbarRounding = 2.0f * scale;
    style.GrabRounding = 0.0f * scale;
    style.TabRounding = 5.0f * scale;
    style.WindowBorderSize = 2.0f * scale;
    style.WindowPadding = ImVec2(8.0f, 5.0f) * scale;
    style.ItemSpacing.y = 8.0f * scale;

    auto& colors = style.Colors;

    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.50f, 0.93f, 0.93f, 0.80f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.29f, 0.29f, 0.29f, 0.80f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.09f, 0.60f, 0.64f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.50f, 0.93f, 0.93f, 0.50f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.50f, 0.93f, 0.93f, 0.83f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.94f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 0.94f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.95f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.49f, 0.92f, 0.91f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.92f, 0.91f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.92f, 0.91f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.50f, 0.93f, 0.93f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.74f, 0.88f, 0.71f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.24f, 0.74f, 0.88f, 0.91f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.44f, 0.80f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.29f, 0.55f, 0.80f, 0.50f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.29f, 0.55f, 0.80f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.51f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.67f, 0.17f, 0.18f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.67f, 0.17f, 0.18f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.50f, 0.93f, 0.93f, 0.80f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.0f, 0.20f, 0.30f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.35f, 0.58f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.30f, 0.50f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.0f, 0.15f, 0.25f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.00f, 0.25f, 0.40f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.35f, 0.43f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.70f, 0.90f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 0.60f, 1.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    // Font
    auto& io = ImGui::GetIO();
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;

    const float size = 16.0f * scale;

    io.FontDefault = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, size, &font_cfg);

    // Notification stuff
    ImGui::MergeIconsWithLatestFont(size, false);
}

void ModFramework::consume_input() {
    m_mouse_delta[0] = m_accumulated_mouse_delta[0];
    m_mouse_delta[1] = m_accumulated_mouse_delta[1];

    m_accumulated_mouse_delta[0] = 0.0f;
    m_accumulated_mouse_delta[1] = 0.0f;
}

bool ModFramework::on_message(HWND& wnd, UINT& message, WPARAM& w_param, LPARAM& l_param) {
    if (!m_initialized) {
        return true;
    }

    if (ImGui_ImplWin32_WndProcHandler(wnd, message, w_param, l_param))
        return true;

    RAWINPUT rawInputData{};
    bool is_mouse_moving = false;

    switch (message) {
    case WM_CLOSE: {
        // Don't allow any more hooks to get initialized
        FunctionHook::allow_hook(false);

	    while(g_currently_hooking != 0)
	    {
            Sleep(5);
		}
    } break;

    case WM_INPUT: {
        // RIM_INPUT means the window has focus
        if (GET_RAWINPUT_CODE_WPARAM(w_param) == RIM_INPUT) {
            uint32_t size = sizeof(RAWINPUT);

            // obtain size
            auto result = ::GetRawInputData((HRAWINPUT)l_param, RID_INPUT, &rawInputData, &size, sizeof(RAWINPUTHEADER));

            if (result != -1 && rawInputData.header.dwType == RIM_TYPEMOUSE) {
                m_accumulated_mouse_delta[0] += (float)rawInputData.data.mouse.lLastX;
                m_accumulated_mouse_delta[1] += (float)rawInputData.data.mouse.lLastY;

                // Allowing camera movement when the UI is hovered while not focused
                is_mouse_moving = rawInputData.data.mouse.lLastX || rawInputData.data.mouse.lLastY;
            }
        }
    } break;

    case RE_TOGGLE_CURSOR: {
        if (!m_is_internal_message) {
            m_main_game_cursor_state_buffer = w_param;

            if (m_draw_ui && !w_param)
            {
                w_param = true;
            }
        }
    } break;

    }
    m_is_internal_message = false;

    // If the user is interacting with the UI we block the message from going to the game.
    if (m_draw_ui && message == WM_INPUT) {
        // Fix of a bug that makes the input key down register but the key up will never register \
         when clicking on the ui while the game is not focused
        if (GET_RAWINPUT_CODE_WPARAM(w_param) == RIM_INPUTSINK)
        {
            return false;
        }

        const auto& io = ImGui::GetIO();
        if (m_is_ui_focused) {
            if (is_mouse_moving || io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput)
            {
                return false;
            }
        }
        else {
            if (!is_mouse_moving && (io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput))
            {
                return false;
            }
        }
    }

    return true;
}

// this is unfortunate.
void ModFramework::on_direct_input_keys(const std::array<uint8_t, 256>& keys) {
    KeyBinder::OnKeyboardUpdate(keys);

    if (m_kcw_buffers.drawWindow) {
        for (size_t i = 0; i < keys.size(); i++)
        {
            if (!m_kcw_buffers.needConfirmBuffer && m_kcw_buffers.drawWindow && !m_last_keys[i] && keys[i])
            {
                m_last_key_mode = UI::KeyMode_t::Keyboard;
            }
        }
    }

    m_last_keys = keys;
}

void ModFramework::on_gamepad_keys(const dmc5::HIDGamePadDevice& gamePadDevice) {
    KeyBinder::OnGamePadUpdate(gamePadDevice);

    if (!m_kcw_buffers.needConfirmBuffer && m_kcw_buffers.drawWindow && gamePadDevice.buttonsDown != REGPK_None)
    {
        m_last_key_mode = UI::KeyMode_t::Controller;
    }

    m_last_controller_state = gamePadDevice.buttons & 0b11111111111111111; // Least significant 17 Bits
}

void ModFramework::save_trainer_settings(utility::Config& cfg) const
{
    cfg.set<bool>("HotkeyNotifications", m_is_notif_enabled);
    cfg.set<bool>("SaveAfterEachUIClose", m_save_after_close_ui);
    cfg.set<bool>("LoadOnStartup", m_load_on_startup);
}

void ModFramework::load_trainer_settings(utility::Config& cfg)
{
    m_load_on_startup = cfg.get<bool>("LoadOnStartup").value_or(true);
    if (m_load_on_startup) {
        m_is_notif_enabled = cfg.get<bool>("HotkeyNotifications").value_or(false);
        m_save_after_close_ui = cfg.get<bool>("SaveAfterEachUIClose").value_or(false);
    }
}

void ModFramework::queue_notification(const ImGuiToast& notif) {
    if (m_is_notif_enabled) {
        ImGui::InsertNotification(notif);
    }
}

void ModFramework::on_frame_d3d11() {
    std::scoped_lock _{ m_ui_mutex };

    spdlog::debug("[D3D11] on_frame");

    if (!m_initialized) {
        if (!initialize()) {
            spdlog::error("[D3D11] Failed to initialize the trainer on DirectX 11");
            return;
        }

        spdlog::info("[D3D11] Trainer initialized");
        m_initialized = true;
        return;
    }

    consume_input();

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (m_error.empty() && m_game_data_initialized) {
        m_mods->on_frame();
    }

    draw_ui();

    draw_notifs();

    ImGui::EndFrame();
    ImGui::Render();

    ComPtr<ID3D11DeviceContext> context{};

    constexpr float clear_color[]{ 0.0f, 0.0f, 0.0f, 0.0f };

    m_d3d11_hook->get_device()->GetImmediateContext(&context);

    context->ClearRenderTargetView(m_d3d11.pd3d_blank_rt_rtv.Get(), clear_color);
    context->ClearRenderTargetView(m_d3d11.pd3d_rt_rtv.Get(), clear_color);
    context->OMSetRenderTargets(1, m_d3d11.pd3d_rt_rtv.GetAddressOf(), nullptr);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Set the back buffer to be the render target.
    context->OMSetRenderTargets(1, m_d3d11.pd3d_backbuffer_rtv.GetAddressOf(), nullptr);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    ImGui::UpdatePlatformWindows();

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::RenderPlatformWindowsDefault();
    }
}

void ModFramework::on_frame_d3d12() {
    std::scoped_lock _{ m_ui_mutex };

    spdlog::debug("[D3D12] on_frame");

    if (!m_initialized) {
        if (!initialize()) {
            spdlog::error("[D3D12] Failed to initialize the trainer on DirectX 12");
            return;
        }

        spdlog::info("[D3D12] Trainer initialized");
        m_initialized = true;
        return;
    }

    auto command_queue = m_d3d12_hook->get_command_queue();

    if (command_queue == nullptr) {
        spdlog::error("[D3D12] Null Command Queue");
        return;
    }

    consume_input();

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (m_error.empty() && m_game_data_initialized) {
        m_mods->on_frame();
    }

    draw_ui();

    draw_notifs();

    ImGui::EndFrame();
    ImGui::Render();

    //Rendering
    const UINT back_buffer_idx = m_d3d12_hook->get_swap_chain()->GetCurrentBackBufferIndex();
    const auto& bb_frame_context = m_d3d12.frame_context[back_buffer_idx];

    constexpr float clear_color[]{ 0.0f, 0.0f, 0.0f, 0.0f };

    const auto pd3d_device = m_d3d12_hook->get_device();

    D3D12_CPU_DESCRIPTOR_HANDLE render_targets[1]{};

    bb_frame_context.command_allocator->Reset();
    m_d3d12.pd3d_command_list->Reset(bb_frame_context.command_allocator.Get(), nullptr);

    // Draw to our render target.
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_d3d12.frame_context[m_d3d12.original_buffer_count].rt.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    m_d3d12.pd3d_command_list->ResourceBarrier(1, &barrier);

    {   
        m_d3d12.pd3d_command_list->ClearRenderTargetView(m_d3d12.get_cpu_rtv(pd3d_device, m_d3d12.original_buffer_count), clear_color, 0, nullptr);
        
        render_targets[0] = m_d3d12.get_cpu_rtv(pd3d_device, m_d3d12.original_buffer_count);
        
        m_d3d12.pd3d_command_list->OMSetRenderTargets(1, render_targets, FALSE, nullptr);
        m_d3d12.pd3d_command_list->SetDescriptorHeaps(1, m_d3d12.pd3d_srv_desc_heap.GetAddressOf());
        
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_d3d12.pd3d_command_list.Get());
        
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        
        m_d3d12.pd3d_command_list->ResourceBarrier(1, &barrier);
    }

    barrier.Transition.pResource = bb_frame_context.rt.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // Draw UI
    {
        m_d3d12.pd3d_command_list->ResourceBarrier(1, &barrier);

        render_targets[0] = m_d3d12.get_cpu_rtv(pd3d_device, back_buffer_idx);

        m_d3d12.pd3d_command_list->OMSetRenderTargets(1, render_targets, FALSE, nullptr);
        m_d3d12.pd3d_command_list->SetDescriptorHeaps(1, m_d3d12.pd3d_srv_desc_heap.GetAddressOf());

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_d3d12.pd3d_command_list.Get());

        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        m_d3d12.pd3d_command_list->ResourceBarrier(1, &barrier);
    }

    m_d3d12.pd3d_command_list->Close();

    command_queue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(m_d3d12.pd3d_command_list.GetAddressOf()));

    ImGui::UpdatePlatformWindows();

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::RenderPlatformWindowsDefault(nullptr, (void*)m_d3d12.pd3d_command_list.Get());
    }
}

void ModFramework::on_reset(const UINT& width, const UINT& height) {
    spdlog::info("Reset!");

    static auto last_size = UI::Vec2<UINT>{ 0, 0 };

    if (m_initialized) {
        // We need to reset the state of the ImGui keys because it can break some text inputs in D3D12
        auto& io = ImGui::GetIO();

        // Just setting these to false seems to fix it
        io.KeyAlt = false;
        io.KeyCtrl = false;
        io.KeySuper = false;
        io.KeyShift = false;

        io.MousePos.x = 0.0f;
        io.MousePos.y = 0.0f;

        for (auto& key : io.MouseDown) {
            key = false;
        }

        for (auto& key : io.KeysDown) {
            key = false;
        }
    }

    // Crashes if we don't release it at this point.
    if (m_is_d3d11) {
        if (m_initialized)
        {
            reset_d3d11();
        }
    }

    if (m_is_d3d12) {
        if (m_initialized)
        {
            reset_d3d12();
        }
    }

    // Reseting the size and pos conds
    if (UI::Vec2{ width, height } != last_size) {
        reset_window_transforms("##SSSiyan's Collaborative Trainer");
        reset_window_transforms("Settings");
    }

    last_size = UI::Vec2{ width, height };

    m_initialized = false;
}

void ModFramework::reset_d3d11()
{
    ImGui_ImplDX11_Shutdown();
    m_d3d11 = {};
}

void ModFramework::reset_d3d12()
{
    ImGui_ImplDX12_Shutdown();
    m_d3d12 = {};
}

void ModFramework::save_config() const
{
    spdlog::info("Saving config to file");

    utility::Config cfg(CONFIG_FILENAME);

    if (m_game_data_initialized) {
        spdlog::info("Saving mods settings in the config");
        m_mods->save_mods();
        cfg += m_mods->get_config();
    }

    save_trainer_settings(cfg);

    if (!KeyBinder::SaveAllBinds(true))
    {
        spdlog::error("Failed to save hotkeys config");
    }

    cfg.save();

    spdlog::info("Saved configs");
}

void ModFramework::load_config()
{
    spdlog::info("Loading config from file");

    utility::Config cfg(CONFIG_FILENAME);

    load_trainer_settings(cfg);

    spdlog::info("Loaded traienr settings");

	m_mods->load_mods(cfg);

    if (!KeyBinder::LoadAllBinds(true))
    {
        spdlog::error("Failed to load hotkeys from config");
    }

    spdlog::info("Loaded configs");
}

bool ModFramework::initialize() {
    if (m_initialized) {
        return true;
    }

	eh::setup_exception_handler();

    if (m_is_d3d11) {
        spdlog::info("[D3D11] Attempting to initialize DirectX 11");

        if (!m_d3d11_hook->is_hooked()) {
            return false;
        }

        const auto pd3d_device = m_d3d11_hook->get_device();
        const auto pd3d_swap_chain = m_d3d11_hook->get_swap_chain();

        // Wait.
        if (pd3d_device == nullptr || pd3d_swap_chain == nullptr) {
            spdlog::info("Device or SwapChain null. DirectX 12 may be in use. Unhooking D3D11...");

            // We unhook D3D11
            if (m_d3d11_hook->unhook())
                spdlog::info("D3D11 unhooked!");
            else
                spdlog::error("Cannot unhook D3D11, this might crash.");

            m_is_d3d11 = false;
            m_valid = false;

            // We hook D3D12
            if (!hook_d3d12()) {
                spdlog::error("Failed to hook D3D12 after unhooking D3D11.");
            }
            return false;
        }

        ComPtr<ID3D11DeviceContext> context;
        pd3d_device->GetImmediateContext(&context);

        pd3d_swap_chain->GetDesc(&m_swap_desc);

        // It can be called when the frame is not properly set yet
        if (m_swap_desc.BufferDesc.Width < 1 || m_swap_desc.BufferDesc.Height < 1)
        {
            spdlog::info("[D3D11] Frame size not set, skipping the initialization in this frame!");
            return false;
        }

        m_wnd = m_swap_desc.OutputWindow;

        m_scale = 1.0f;

        const auto x_ratio = m_swap_desc.BufferDesc.Width / m_window_size.x;
        const auto y_ratio = m_swap_desc.BufferDesc.Height / m_window_size.y;
        const auto smaller_ratio = std::min(x_ratio, y_ratio);
        if (smaller_ratio < 1.0)
        {
            constexpr float max_allowed_ratio = 750.0f / 900.0f;
            m_scale = max_allowed_ratio * smaller_ratio;
        }

        spdlog::info("[D3D11] Creating render target");

        if(!create_render_target_d3d11())
        {
            spdlog::error("[D3D11] Failed to create render targets!");
            return false;
        }

        spdlog::info("[D3D11] Window Handle: {0:x}", reinterpret_cast<uintptr_t>(m_wnd));
        spdlog::info("[D3D11] Initializing ImGui");

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        set_style(m_scale);

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

        spdlog::info("[D3D11] Initializing ImGui Win32");

        if (!ImGui_ImplWin32_Init(m_wnd)) {
            spdlog::error("[D3D11] Failed to initialize ImGui.");
            return false;
        }

        spdlog::info("[D3D11] Initializing ImGui D3D11");

        if (!ImGui_ImplDX11_Init(pd3d_device, context.Get())) {
            spdlog::error("[D3D11] Failed to initialize ImGui.");
            return false;
        }

        // Loading the custom textures for DX11
        m_logo_dx11 = UI::Texture2DDX11(logo.GetRGBAData(), logo.GetWidth(), logo.GetHeight(), pd3d_device);
        m_icons.kbIconDX11 = UI::Texture2DDX11(kbIcon.GetRGBAData(), kbIcon.GetWidth(), kbIcon.GetHeight(), pd3d_device);
        m_icons.kbIconActiveDX11 = UI::Texture2DDX11(kbIconActive.GetRGBAData(), kbIconActive.GetWidth(), kbIconActive.GetHeight(), pd3d_device);
        m_icons.keyIconsDX11 = UI::Texture2DDX11(keyIcons.GetRGBAData(), keyIcons.GetWidth(), keyIcons.GetHeight(), pd3d_device);
        m_icons.gearIconDX11 = UI::Texture2DDX11(gearIcon.GetRGBAData(), gearIcon.GetWidth(), gearIcon.GetHeight(), pd3d_device);

        if (!m_logo_dx11 || !m_icons.kbIconDX11 || !m_icons.kbIconActiveDX11
            || !m_icons.keyIconsDX11 || !m_icons.gearIconDX11) {
            spdlog::error("[D3D11] Failed to load textures!");

            if (!m_logo_dx11) {
                spdlog::error("[D3D11] m_logo_dx11 -> {}", m_logo_dx11.GetLastError().c_str());
            }

            if (!m_icons.kbIconDX11) {
                spdlog::error("[D3D11] m_icons.kbIconDX11 -> {}", m_icons.kbIconDX11.GetLastError().c_str());
            }

            if (!m_icons.kbIconActiveDX11) {
                spdlog::error("[D3D11] m_icons.kbIconActiveDX11 -> {}", m_icons.kbIconActiveDX11.GetLastError().c_str());
            }

            if (!m_icons.keyIconsDX11) {
                spdlog::error("[D3D11] m_icons.keyIconsDX11 -> {}", m_icons.keyIconsDX11.GetLastError().c_str());
            }

            if (!m_icons.gearIconDX11) {
                spdlog::error("[D3D11] m_icons.gearIconDX11 -> {}", m_icons.gearIconDX11.GetLastError().c_str());
            }

            return false;
        }
    }
    else if (m_is_d3d12) {
        spdlog::info("[D3D12] Attempting to initialize DirectX 12");

        if (!m_d3d12_hook->is_hooked()) {
            return false;
        }

        const auto pd3d_device = m_d3d12_hook->get_device();
        const auto pd3d_swap_chain = m_d3d12_hook->get_swap_chain();

        if (pd3d_device == nullptr || pd3d_swap_chain == nullptr) {
            spdlog::info("Device or SwapChain null. DirectX 11 may be in use. Unhooking D3D12...");

            // We unhook D3D12
            if (m_d3d12_hook->unhook())
                spdlog::info("D3D12 unhooked!");
            else
                spdlog::error("Cannot unhook D3D12, this might crash.");

            m_valid = false;
            m_is_d3d12 = false;

            // We hook D3D11
            if (LoadLibraryA("d3d12.dll") != nullptr) {
                if (!hook_d3d11()) {
                    spdlog::error("Failed to hook D3D11 after unhooking D3D12.");
                }
            }
            return false;
        }

        pd3d_swap_chain->GetDesc(&m_swap_desc);

        // It can be called when the frame is not properly set yet
        if (m_swap_desc.BufferDesc.Width < 1 || m_swap_desc.BufferDesc.Height < 1)
        {
            spdlog::info("[D3D12] Frame size not set, skipping the initialization in this frame!");
            return false;
        }

        m_wnd = m_swap_desc.OutputWindow;
        m_d3d12.original_buffer_count = m_swap_desc.BufferCount;
        m_d3d12.buffer_count = m_d3d12.original_buffer_count + 2;
        m_d3d12.frame_context.resize(m_d3d12.buffer_count);

        m_scale = 1.0f;

        const auto x_ratio = m_swap_desc.BufferDesc.Width / m_window_size.x;
        const auto y_ratio = m_swap_desc.BufferDesc.Height / m_window_size.y;
        const auto smaller_ratio = std::min(x_ratio, y_ratio);
        if (smaller_ratio < 1.0)
        {
            constexpr float max_allowed_ratio = 750.0f / 900.0f;
            m_scale = max_allowed_ratio * smaller_ratio;
        }

        if (!create_command_allocator_d3d12()) {
            spdlog::error("[D3D12] Failed to create Command Allocator.");
            return false;
        }

        if (!create_command_list_d3d12()) {
            spdlog::error("[D3D12] Failed to create Command List.");
            return false;
        }

        if (!create_rtv_descriptor_heap_d3d12()) {
            spdlog::error("[D3D12] Failed to create RTV Descriptor.");
            return false;
        }

        if (!create_srv_descriptor_heap_d3d12(8)) {
            spdlog::error("[D3D12] Failed to create SRV Descriptor.");
            return false;
        }

        if(!create_render_target_d3d12())
        {
            spdlog::error("[D3D12] Failed to create RenderTargets.");
            return false;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        set_style(m_scale);

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

        if (!ImGui_ImplWin32_Init(m_wnd)) {
            spdlog::error("[D3D12] Failed to initialize ImGui ImplWin32.");
            return false;
        }

        // Create our imgui and blank rts.
        const auto& backbuffer = m_d3d12.frame_context[0].rt;
        const auto bb_desc = backbuffer->GetDesc();

        if (!ImGui_ImplDX12_Init(pd3d_device, 1,
            bb_desc.Format, m_d3d12.pd3d_srv_desc_heap.Get(),
            m_d3d12.get_cpu_srv(pd3d_device, 0),
            m_d3d12.get_gpu_srv(pd3d_device, 0)))
        {
            spdlog::error("[D3D12] Failed to initialize ImGui ImplDX12.");
            return false;
        }

        //ImGui_ImplDX12_InvalidateDeviceObjects();
        //if (!ImGui_ImplDX12_CreateDeviceObjects()) {
        //    spdlog::error("[D3D12] Failed to initialize ImGui CreateDeviceObjects.");
        //    return false;
        //}

        // Getting the command quueue to initialize the textures using it
        auto cmd_queue = m_d3d12_hook->get_command_queue();
        if (cmd_queue == nullptr) {
            spdlog::error("[D3D12] Failed to retrieve the command queue for loading textures!");
            return false;
        }

        // Loading the custom textures for DX12
        m_logo_dx12 = UI::Texture2DDX12(logo.GetRGBAData(), logo.GetWidth(), logo.GetHeight(), pd3d_device, cmd_queue, m_d3d12.pd3d_srv_desc_heap.Get(), 3);
        m_icons.kbIconDX12 = UI::Texture2DDX12(kbIcon.GetRGBAData(), kbIcon.GetWidth(), kbIcon.GetHeight(), pd3d_device, cmd_queue, m_d3d12.pd3d_srv_desc_heap.Get(), 4);
        m_icons.kbIconActiveDX12 = UI::Texture2DDX12(kbIconActive.GetRGBAData(), kbIconActive.GetWidth(), kbIconActive.GetHeight(), pd3d_device, cmd_queue, m_d3d12.pd3d_srv_desc_heap.Get(), 5);
        m_icons.keyIconsDX12 = UI::Texture2DDX12(keyIcons.GetRGBAData(), keyIcons.GetWidth(), keyIcons.GetHeight(), pd3d_device, cmd_queue, m_d3d12.pd3d_srv_desc_heap.Get(), 6);
        m_icons.gearIconDX12 = UI::Texture2DDX12(gearIcon.GetRGBAData(), gearIcon.GetWidth(), gearIcon.GetHeight(), pd3d_device, cmd_queue, m_d3d12.pd3d_srv_desc_heap.Get(), 7);

        if (!m_logo_dx12 || !m_icons.kbIconDX12 || !m_icons.kbIconActiveDX12
            || !m_icons.keyIconsDX12 || !m_icons.gearIconDX12) {
            spdlog::error("[D3D12] Failed to load textures!");

            if (!m_logo_dx12) {
                spdlog::error("[D3D12] m_logo_dx12 -> {}", m_logo_dx12.GetLastError().c_str());
            }

            if (!m_icons.kbIconDX12) {
                spdlog::error("[D3D12] m_icons.kbIconDX12 -> {}", m_icons.kbIconDX12.GetLastError().c_str());
            }

            if (!m_icons.kbIconActiveDX12) {
                spdlog::error("[D3D12] m_icons.kbIconActiveDX12 -> {}", m_icons.kbIconActiveDX12.GetLastError().c_str());
            }

            if (!m_icons.keyIconsDX12) {
                spdlog::error("[D3D12] m_icons.keyIconsDX12 -> {}", m_icons.keyIconsDX12.GetLastError().c_str());
            }

            if (!m_icons.gearIconDX12) {
                spdlog::error("[D3D12] m_icons.gearIconDX12 -> {}", m_icons.gearIconDX12.GetLastError().c_str());
            }

            return false;
        }

        /*m_target_width = m_d3d12_hook->get_display_width();
        m_target_height = m_d3d12_hook->get_display_height();

        m_render_width = m_d3d12_hook->get_render_width();
        m_render_height = m_d3d12_hook->get_render_height();*/
    }
    else { return false; }

    // Explicitly call destructor first
    m_windows_message_hook.reset();
    m_windows_message_hook = std::make_unique<WindowsMessageHook>(m_wnd);
    m_windows_message_hook->on_message = [this](auto& wnd, auto& msg, auto& wParam, auto& lParam) {
        return on_message(wnd, msg, wParam, lParam);
    };

    // just do this instead of rehooking because there's no point.
    if (m_first_frame) {
        //std::lock_guard<std::mutex> lck(m_controllerhook_mtx);
        m_dinput_hook = std::make_unique<DInputHook>(m_wnd);
        m_controller_hook = std::make_unique<ControllerHook>();
    }
    else {
        m_dinput_hook->set_window(m_wnd);
    }

    if (m_first_frame) {
        m_first_frame = false;

        spdlog::info("Loading trainer specific settings.");

        KeyBinder::LoadBind("Menu Key");
        KeyBinder::LoadBind("Close Menu Key");

        // Game specific initialization stuff
        std::thread init_thread([this]() {
			spdlog::info("Loading REFramework SDK");
            reframework::initialize_sdk();

            m_mods = std::make_unique<Mods>();

            auto e = m_mods->on_initialize(m_load_on_startup);

            if (e) {
                if (e->empty()) {
                    m_error = "An unknown error has occurred.";
                }
                else {
                    m_error = *e;
                }
            }

            KeyBinder::LoadAllBinds(true);

            m_game_data_initialized = true;
            });

        init_thread.detach();
    }

    return true;
}

void ModFramework::prepare_textures()
{
    logo.Resize(800, 76);
    kbIcon.ResizeByRatioH(21);
    kbIconActive.ResizeByRatioH(21);
    keyIcons.ResizeByRatioW(512);
    gearIcon.ResizeByRatioW(25);
}

void ModFramework::initialize_key_bindings()
{
    KeyBinder::AddBind("Menu Key",
        [this]() {
            m_draw_ui = !m_draw_ui;
        }, OnState_Press, m_default_menu_key);

    KeyBinder::AddBind("Close Menu Key",
        [this]() {
            if (m_initialized && !ImGui::GetIO().WantTextInput && m_is_ui_focused && m_draw_ui) {
                m_draw_ui = false;

                m_close_menu_guard = true;
            }
        },
        [this]() {
            if (!m_draw_ui && m_close_menu_guard) m_dinput_hook->ignore_input();
        },
            [this]() {
            if (!m_draw_ui && m_close_menu_guard) {
                m_dinput_hook->acknowledge_input();
                m_close_menu_guard = false;
            }
        }, m_default_close_menu_key);
}

void ModFramework::focus_tab(const std::string_view& window_name)
{
    if (!m_initialized)
    {
        return;
    }

    ImGuiWindow* window = ImGui::FindWindowByName(window_name.data());

    if (window == nullptr || window->DockNode == nullptr || window->DockNode->TabBar == nullptr) {
        return;
    }

    window->DockNode->TabBar->NextSelectedTabId = window->ID;
}

bool ModFramework::is_window_focused(const std::string_view& window_name)
{
    if (!m_initialized)
    {
        return false;
    }

    const auto window = ImGui::FindWindowByName(window_name.data());
    const auto focusedWindow = ImGui::GetCurrentContext()->NavWindow;

	return window != nullptr && focusedWindow != nullptr && window == focusedWindow;
}

void ModFramework::reset_window_transforms(const std::string_view& window_name)
{
    if(!m_initialized)
    {
        return;
    }

    const auto window = ImGui::FindWindowByName(window_name.data());

    if (window == nullptr)
    {
        return;
    }

    // Resetting the pos and size conds to be able to be set once afterwards

    window->SetWindowPosAllowFlags |= ImGuiCond_Once;
    window->SetWindowSizeAllowFlags |= ImGuiCond_Once;
}

void ModFramework::draw_ui() {
    std::lock_guard _{ m_input_mutex };

    if (!m_draw_ui) {
        m_is_ui_focused = false;

        if (m_do_once_after_ui) {
            if (m_save_after_close_ui) {
                save_config();
            }

            m_is_internal_message = true;
            m_windows_message_hook->window_toggle_cursor(m_main_game_cursor_state_buffer);

            m_dinput_hook->acknowledge_input();
            m_controller_hook->acknowledge_input();
            KeyBinder::AcknowledgeInput();

            m_do_once_after_ui = false;
        }

        return;
    }

    // UI Specific code:
    m_is_ui_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);

    auto& style = ImGui::GetStyle();
    if (m_is_ui_focused) {
        style.Alpha = 1.0f;
    }
    else {
        if (ImGui::IsWindowHovered(ImGuiFocusedFlags_AnyWindow)) {
            style.Alpha = 0.7f;
        }
        else {
            style.Alpha = 0.4f;
        }
    }

    auto& io = ImGui::GetIO();

    if (m_swap_desc.BufferDesc.Width > 0 && m_swap_desc.BufferDesc.Height > 0) {
        //size = ImVec2((float)swap_desc.BufferDesc.Width, (float)swap_desc.BufferDesc.Height);
        io.DisplayFramebufferScale = ImVec2(
            static_cast<float>(m_swap_desc.BufferDesc.Width) / io.DisplaySize.x,
            static_cast<float>(m_swap_desc.BufferDesc.Height) / io.DisplaySize.y);
    }

    if (io.WantCaptureKeyboard || m_is_ui_focused) {
        m_dinput_hook->ignore_input();
        m_controller_hook->ignore_input();
    }
    else {
        m_dinput_hook->acknowledge_input();
        m_controller_hook->acknowledge_input();
    }

    if (!m_do_once_after_ui) {
        m_is_internal_message = true;
        m_windows_message_hook->window_toggle_cursor(true);
    }

    static constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;
    const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    ImGuiWindowClass mainWindowClass;
    mainWindowClass.DockNodeFlagsOverrideClear = 000;
    mainWindowClass.DockNodeFlagsOverrideSet = 000;
    ImGui::SetNextWindowClass(&mainWindowClass);

    ImGui::SetNextWindowPos(m_window_pos * m_scale, ImGuiCond_Once);
    ImGui::SetNextWindowSize(m_window_size * m_scale, ImGuiCond_Once);

    ImGui::SetNextWindowViewport(mainViewport->ID);

    ImGui::PushStyleColor(ImGuiCol_ResizeGrip, 0);

    const auto is_trainer_focused = !is_window_focused("Settings"); // Yeah...

    const auto border_col = ImGui::GetStyle().Colors[ImGuiCol_Border];
    const ImVec4 border_unfocused{ border_col.x, border_col.y, border_col.z, border_col.w * 0.7f };
    const auto border_shadow_col = ImGui::GetStyle().Colors[ImGuiCol_BorderShadow];
    const ImVec4 border_shadow_unfocused{ border_shadow_col.x, border_shadow_col.y, border_shadow_col.z, border_shadow_col.w * 0.7f };

	ImGui::PushStyleColor(ImGuiCol_Border, is_trainer_focused ? border_col : border_unfocused);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow, is_trainer_focused ? border_shadow_col : border_shadow_unfocused);
    ImGui::Begin("##SSSiyan's Collaborative Trainer", m_kcw_buffers.drawWindow ? nullptr : &m_draw_ui, windowFlags);

    ImGui::PopStyleColor(2);

	const auto trainer_width = ImGui::GetContentRegionAvailWidth();

    ImGui::SetCursorPosX(trainer_width / 2 - static_cast<float>(logo.GetWidth()) * m_scale / 2);

    if (m_is_d3d11)
        ImGui::Image(m_logo_dx11, m_logo_dx11.GetSize(m_scale));
    else
        ImGui::Image(m_logo_dx12, m_logo_dx12.GetSize(m_scale));

    const auto config_buttons_pos = ImGui::GetCursorPos();
    const auto window_pos = ImGui::GetWindowPos();

    UI::Vec2 settings_button_pos{ window_pos.x + style.WindowPadding.x + 5.0f,  window_pos.y + style.WindowPadding.y + 12.0f };

    ImGui::SetCursorScreenPos(settings_button_pos);

    ImGui::PushStyleColor(ImGuiCol_Button, 0);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 0);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, 0);
	if (m_is_d3d11) {
        if (ImGui::InvisibleButton("Settings", m_icons.gearIconDX11.GetSize(m_scale) + style.FramePadding * 2))
        {
            m_show_settings = !m_show_settings;
        }

        ImGui::SetCursorScreenPos(settings_button_pos + style.FramePadding);
        ImGui::Image(m_icons.gearIconDX11, m_icons.gearIconDX11.GetSize(m_scale), { 0, 0 }, { 1, 1 }, ImGui::IsItemHovered() ? UI::BUTTONCOLORHOVERED : UI::BUTTONCOLOR);

    } else
    {
        if (ImGui::InvisibleButton("Settings", m_icons.gearIconDX12.GetSize(m_scale) + style.FramePadding * 2))
        {
            m_show_settings = !m_show_settings;
        }

        ImGui::SetCursorScreenPos(settings_button_pos + style.FramePadding);
        ImGui::Image(m_icons.gearIconDX12, m_icons.gearIconDX12.GetSize(m_scale), { 0, 0 }, { 1, 1 }, ImGui::IsItemHovered() ? UI::BUTTONCOLORHOVERED : UI::BUTTONCOLOR);
    }
    ImGui::PopStyleColor(3);

    ImGui::SetCursorPos({ config_buttons_pos.x, config_buttons_pos.y + 15.0f });

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

    const auto save_label = "Save Config";
    const auto load_label = "Load Config";
    const auto save_label_size = ImGui::CalcTextSize(save_label);
    const auto load_label_size = ImGui::CalcTextSize(load_label);
    const auto save_button_size = ImGui::CalcItemSize({ 0.0f, 0.0f }, save_label_size.x + style.FramePadding.x * 2.0f, save_label_size.y + style.FramePadding.y * 2.0f);
    const auto load_button_size = ImGui::CalcItemSize({ 0.0f, 0.0f }, load_label_size.x + style.FramePadding.x * 2.0f, load_label_size.y + style.FramePadding.y * 2.0f);
    const auto tr_version_size = ImGui::CalcTextSize(TRAINER_VERSION_STR);

    ImGui::SetCursorPosX(trainer_width / 2 - (save_button_size.x + load_button_size.x) / 2);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.25f, 0.38f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.35f, 0.58f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.00f, 0.30f, 0.50f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 1.0f, 1.0f, 1.0f));
    if (ImGui::Button(save_label))
    {
        save_config();
    }
    ImGui::SameLine();
    if (ImGui::Button(load_label)) {
        load_config();
    }

    ImGui::SameLine();
    ImGui::SetCursorPosX(trainer_width - tr_version_size.x);
    ImGui::TextWrapped(TRAINER_VERSION_STR);

    ImGui::PopStyleColor(4);

    ImGui::PopStyleVar();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY());
    ImGui::Separator();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

    static ImGuiID left{}, right{};
    const ImGuiID dockSpaceId = ImGui::GetID("SSSiyan's Collaborative Trainer");
    if (!ImGui::DockBuilderGetNode(dockSpaceId))
    {
        ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockSpaceId, ImGui::GetContentRegionAvail());

        ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.4f, &left, &right);

        // Mods
        ImGui::DockBuilderDockWindow("Gameplay", left);
        ImGui::DockBuilderDockWindow("Scenario", left);
        ImGui::DockBuilderDockWindow("System", left);
        ImGui::DockBuilderDockWindow("Nero", left);
        ImGui::DockBuilderDockWindow("Dante", left);
        ImGui::DockBuilderDockWindow("V", left);
        ImGui::DockBuilderDockWindow("Vergil", left);

        // Settings
        ImGui::DockBuilderDockWindow("Options", right);

        ImGui::DockBuilderFinish(dockSpaceId);
    }

    ImGui::PushStyleColor(ImGuiCol_Header, 0);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);

    ImGui::DockSpace(dockSpaceId, ImGui::GetContentRegionAvail(), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton);

    draw_panels();
    draw_options();

    ImGui::PopStyleColor();

    draw_trainer_settings();

    ImGui::PopStyleColor();

    if (!m_is_focus_set)
    {
        focus_tab("Gameplay");

        m_is_focus_set = true;
    }

    // Store focused panels' ID
    if (const auto window = ImGui::FindWindowByID(ImGui::DockBuilderGetNode(left)->TabBar->SelectedTabId); window != nullptr) {
        if (const auto panelID = m_mods_panels_map.find(window->Name); panelID != m_mods_panels_map.end()) {
            m_focused_mod_panel = panelID->second;
        }
    }

    ImGui::PopStyleVar();

    ImGui::End();

    ImGui::PopStyleColor(2);

    // If we are binding a key
    if (m_kcw_buffers.drawWindow) {
        KeyBinder::IgnoreInput(); // Ignoring binds when listening for new key presses
        m_kcw_buffers.windowSizeBuffer = { ImGui::GetMainViewport()->Size.x / 5.0f, ImGui::GetMainViewport()->Size.y / 5.0f };
        UI::KeyCaptureWindow(m_kcw_buffers);
    }
    else {
        KeyBinder::AcknowledgeInput(); // Acknowledging the binds when we are done adding binds
    }

    m_do_once_after_ui = true;
}

void ModFramework::draw_panels() const
{
	const float modListIndent = 10.0f * m_scale;

    static const ImVec4 activeTabText = { 0.5f, 1.0f, 1.0f, 1.0f };
    static const ImVec4 inactiveTabText = { 0.5f, 1.0f, 1.0f, 0.7f };

    static constexpr ImGuiWindowFlags panel_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing;

    ImGui::PushStyleColor(ImGuiCol_Header, { 0.26f, 0.59f, 0.98f, 0.31f });

    ImGui::PushStyleColor(ImGuiCol_Text, m_focused_mod_panel == PanelID_Gameplay ? activeTabText : inactiveTabText);
    ImGui::Begin("Gameplay", nullptr, panel_flags);
    ImGui::PopStyleColor();
    {
        if (m_error.empty() && m_game_data_initialized) {
            ImGui::Text("Shared Cheats");
            m_mods->on_pagelist_ui(Mod::Page_CommonCheat, modListIndent);

            ImGui::Separator();
            ImGui::Text("Common Mechanics");
            m_mods->on_pagelist_ui(Mod::Page_Mechanics, modListIndent);

            ImGui::Separator();
            ImGui::Text("Animation");
            m_mods->on_pagelist_ui(Mod::Page_Animation, modListIndent);

            ImGui::Separator();
            ImGui::Text("Enemy Step");
            m_mods->on_pagelist_ui(Mod::Page_EnemyStep, modListIndent);

            ImGui::Separator();
            ImGui::Text("Taunts");
            m_mods->on_pagelist_ui(Mod::Page_Taunt, modListIndent);
        }
        else if (!m_game_data_initialized) {
            ImGui::TextWrapped("Trainer is currently initializing...");
        }
        else if (!m_error.empty()) {
            ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
        }
    }
    ImGui::End();

    ImGui::PushStyleColor(ImGuiCol_Text, m_focused_mod_panel == PanelID_Scenario ? activeTabText : inactiveTabText);
    ImGui::Begin("Scenario", nullptr, panel_flags);
    ImGui::PopStyleColor();
    {
        if (m_error.empty() && m_game_data_initialized) {

            ImGui::Text("Game Modes");
            m_mods->on_pagelist_ui(Mod::Page_GameMode, modListIndent);

            ImGui::Separator();
            
            ImGui::Text("Encounter Settings");
            m_mods->on_pagelist_ui(Mod::Page_Encounters, modListIndent);

            ImGui::Separator();

            ImGui::Text("Enemy Settings");
            m_mods->on_pagelist_ui(Mod::Page_Enemies, modListIndent);

            ImGui::Separator();
            
            ImGui::Text("Bloody Palace");
            m_mods->on_pagelist_ui(Mod::Page_BloodyPalace, modListIndent);
        }
        else if (!m_game_data_initialized) {
            ImGui::TextWrapped("Trainer is currently initializing...");
        }
        else if (!m_error.empty()) {
            ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
        }
    }
    ImGui::End();

    ImGui::PushStyleColor(ImGuiCol_Text, m_focused_mod_panel == PanelID_System ? activeTabText : inactiveTabText);
    ImGui::Begin("System", nullptr, panel_flags);
    ImGui::PopStyleColor();
    {
        if (m_error.empty() && m_game_data_initialized) {
            ImGui::Text("Camera");
            m_mods->on_pagelist_ui(Mod::Page_Camera, modListIndent);

            ImGui::Separator();
            ImGui::Text("Quality-of-life");
            m_mods->on_pagelist_ui(Mod::Page_QOL, modListIndent);
        }
        else if (!m_game_data_initialized) {
            ImGui::TextWrapped("Trainer is currently initializing...");
        }
        else if (!m_error.empty()) {
            ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
        }
    }
    ImGui::End();

    ImGui::PushStyleColor(ImGuiCol_Text, m_focused_mod_panel == PanelID_Nero ? activeTabText : inactiveTabText);
    ImGui::Begin("Nero", nullptr, panel_flags);
    ImGui::PopStyleColor();
    {
        if (m_error.empty() && m_game_data_initialized) {
            ImGui::Text("Breaker");
            m_mods->on_pagelist_ui(Mod::Page_Breaker, modListIndent);

            ImGui::Separator();
            ImGui::Text("Wiresnatch");
            m_mods->on_pagelist_ui(Mod::Page_Wiresnatch, modListIndent);

            ImGui::Separator();
            ImGui::Text("Cheats");
            m_mods->on_pagelist_ui(Mod::Page_Nero, modListIndent);
        }
        else if (!m_game_data_initialized) {
            ImGui::TextWrapped("Trainer is currently initializing...");
        }
        else if (!m_error.empty()) {
            ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
        }
    }
    ImGui::End();

    ImGui::PushStyleColor(ImGuiCol_Text, m_focused_mod_panel == PanelID_Dante ? activeTabText : inactiveTabText);
    ImGui::Begin("Dante", nullptr, panel_flags);
    ImGui::PopStyleColor();
    {
        if (m_error.empty() && m_game_data_initialized) {
            ImGui::Text("SDT");
            m_mods->on_pagelist_ui(Mod::Page_DanteSDT, modListIndent);

            ImGui::Separator();
            ImGui::Text("Cheats");
            m_mods->on_pagelist_ui(Mod::Page_DanteCheat, modListIndent);
        }
        else if (!m_game_data_initialized) {
            ImGui::TextWrapped("Trainer is currently initializing...");
        }
        else if (!m_error.empty()) {
            ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
        }
    }
    ImGui::End();

    ImGui::PushStyleColor(ImGuiCol_Text, m_focused_mod_panel == PanelID_Gilver ? activeTabText : inactiveTabText);
    ImGui::Begin("V", nullptr, panel_flags);
    ImGui::PopStyleColor();
    {
        if (m_error.empty() && m_game_data_initialized) {
            ImGui::Text("Cheats");
            m_mods->on_pagelist_ui(Mod::Page_Gilver, modListIndent);
        }
        else if (!m_game_data_initialized) {
            ImGui::TextWrapped("Trainer is currently initializing...");
        }
        else if (!m_error.empty()) {
            ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
        }
    }
    ImGui::End();

    ImGui::PushStyleColor(ImGuiCol_Text, m_focused_mod_panel == PanelID_Vergil ? activeTabText : inactiveTabText);
    ImGui::Begin("Vergil", nullptr, panel_flags);
    ImGui::PopStyleColor();
    {
        if (m_error.empty() && m_game_data_initialized) {
            ImGui::Text("SDT");
            m_mods->on_pagelist_ui(Mod::Page_VergilSDT, modListIndent);

            ImGui::Separator();
            ImGui::Text("Dark Slayer");
            m_mods->on_pagelist_ui(Mod::Page_VergilTrick, modListIndent);

            ImGui::Separator();
            ImGui::Text("Cheats");
            m_mods->on_pagelist_ui(Mod::Page_VergilCheat, modListIndent);

            ImGui::Separator();
            ImGui::Text("Doppelganger");
            m_mods->on_pagelist_ui(Mod::Page_VergilDoppel, modListIndent);

            ImGui::Separator();
            ImGui::Text("VFX Settings");
            m_mods->on_pagelist_ui(Mod::Page_VergilVFXSettings, modListIndent);
        }
        else if (!m_game_data_initialized) {
            ImGui::TextWrapped("Trainer is currently initializing...");
        }
        else if (!m_error.empty()) {
            ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
        }
    }
    ImGui::End();

    ImGui::PopStyleColor();
}

void ModFramework::draw_options() const
{
    static constexpr ImGuiWindowFlags panel_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing
	| ImGuiWindowFlags_NoTitleBar;

    ImGuiWindowClass windowClass;
    windowClass.DockNodeFlagsOverrideClear = 000;
    windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;

    ImGui::SetNextWindowClass(&windowClass);

    ImGui::Begin("Options", nullptr, panel_flags);
    {
        const auto current_mod = m_mods->get_mod(m_mods->get_focused_mod());

        if (current_mod != nullptr) {
            ImGui::TextWrapped("Selected Mod: %s", current_mod->m_full_name_string.c_str());
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
            ImGui::TextWrapped("Description: %s", current_mod->m_description_string.c_str());
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
            ImGui::TextWrapped("Author: %s", current_mod->m_author_string.c_str());
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);

            ImGui::Separator();

            current_mod->on_draw_ui();
        } else
        {
            if (m_error.empty() && m_game_data_initialized) {
                ImGui::TextWrapped("Welcome to SSSiyan's collaborative cheat trainer!\n"
                    "Click any cheat marked with (+) to view additional options.");
            }
            else if (!m_game_data_initialized) {
                ImGui::TextWrapped("Trainer is currently initializing...");
            }
        }
    }
    ImGui::End();
}

void ModFramework::draw_trainer_settings()
{
    if(!m_show_settings)
    {
        return;
    }

    ImGui::SetNextWindowPos(ImVec2(m_window_pos.x + 25.0f, m_window_pos.y + 15.0f) * m_scale, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(500.0f, 650.0f) * m_scale, ImGuiCond_Once);

    static const ImVec4 activeTabText = { 0.5f, 1.0f, 1.0f, 1.0f };
    static const ImVec4 inactiveTabText = { 0.5f, 1.0f, 1.0f, 0.7f };

    const bool is_settings_active = is_window_focused("Settings");

    const auto border_col = ImGui::GetStyle().Colors[ImGuiCol_Border];
    const ImVec4 border_unfocused{ border_col.x, border_col.y, border_col.z, border_col.w * 0.7f };
    const auto border_shadow_col = ImGui::GetStyle().Colors[ImGuiCol_BorderShadow];
    const ImVec4 border_shadow_unfocused{ border_shadow_col.x, border_shadow_col.y, border_shadow_col.z, border_shadow_col.w * 0.7f };

    ImGui::PushStyleColor(ImGuiCol_Border, is_settings_active ? border_col : border_unfocused);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow, is_settings_active ? border_shadow_col : border_shadow_unfocused);
    ImGui::PushStyleColor(ImGuiCol_Text, is_settings_active ? activeTabText : inactiveTabText);
    ImGui::Begin("Settings", &m_show_settings, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse);
    ImGui::PopStyleColor(3);
    {
        ImGui::Text("Menu Key:"); ImGui::SameLine();
        ImGui::SetCursorScreenPos(UI::Vec2<float>(ImGui::GetCursorScreenPos()) - UI::Vec2(10.0f, 2.0f) * m_scale);
        UI::KeyBindButton("Menu Key", "Menu Key", m_kcw_buffers, 1.0f, true, UI::BUTTONCOLOR);
        ImGui::Text("Close Menu Key:"); ImGui::SameLine();
        ImGui::SetCursorScreenPos(UI::Vec2<float>(ImGui::GetCursorScreenPos()) - UI::Vec2(10.0f, 2.0f) * m_scale);
        UI::KeyBindButton("Close Menu Key", "Close Menu Key", m_kcw_buffers, 1.0f, true, UI::BUTTONCOLOR);

        ImGui::Checkbox("Hotkey Toggle Notifications", &m_is_notif_enabled);
        ImGui::Checkbox("Save Config Automatically After UI/Game Gets Closed", &m_save_after_close_ui);
        ImGui::Checkbox("Load Config Automatically When The Game Launches", &m_load_on_startup);
        //ImGui::ShowHelpMarker("Some mods like \"DMC3JCE\", \"Boss Vergil Moves\", \"quicksilver\", etc. are using coroutine system which allows to exceute actions with some delay. Check this if you want to sync "
         //   "all this delays with turbo mod speed when it enabled.");

        ImGui::Spacing();
        if (ImGui::TreeNodeEx("About")) {
            if (ImGui::TreeNodeEx("Credits"))
            {
                const ImVec4 defaultColor{ 0.46f, 0.46f, 0.92f, 1.0f };
                { // NOBODY TOUCHES THIS OR I SWEAR... 
                  //sorry Dark, it was for greater goods....
                    static float r, g, b;
                    ImGui::ColorConvertHSVtoRGB(static_cast<float>(std::abs(std::sin(static_cast<double>(GetTickCount64() % (20LL * 360LL)) / (20.0 * 360.0)))), 1.0f, 1.0f, r, g, b);
                    ImGui::PushStyleColor(ImGuiCol_Text, { r, g, b, 1.0f });

                    if (ImGui::TreeNode("Darkness"))
                    {
                        ImGui::TextHyperlink("Nexus", "https://www.nexusmods.com/devilmaycry5/users/70013978?tab=about+me", defaultColor);
                        ImGui::TextHyperlink("Github", "https://github.com/amir-120", defaultColor);
                        ImGui::TreePop();
                    }

                    ImGui::Dummy({ 0.0f, 1.0f * m_scale });
                    ImGui::PopStyleColor();
                }

                if (ImGui::TreeNode("The Hitchhiker"))
                {
                    ImGui::TextHyperlink("Nexus", "https://www.nexusmods.com/devilmaycry5/users/49242683", defaultColor);
                    ImGui::TextHyperlink("Youtube", "https://www.youtube.com/c/TheHitchhiker", defaultColor);
                    ImGui::TextHyperlink("Twitch", "https://www.twitch.tv/hitchhikingthrough", defaultColor);
                    ImGui::TextHyperlink("Twitter", "https://twitter.com/Hitchhikingthr2", defaultColor);
                    ImGui::TextHyperlink("Patreon", "https://www.patreon.com/HitchhikingThrough", defaultColor);
                    ImGui::TextHyperlink("Discord server", "https://discord.com/invite/fyrKPURnGW", defaultColor);
                    ImGui::TextHyperlink("Github", "https://github.com/youwereeatenbyalid", defaultColor);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Siyan"))
                {
                    ImGui::TextHyperlink("Nexus", "https://www.nexusmods.com/devilmaycry5/users/68550442", defaultColor);
                    ImGui::TextHyperlink("Twitch", "https://www.twitch.tv/sssiyan", defaultColor);
                    ImGui::TextHyperlink("Twitter", "https://twitter.com/SSSiyan", defaultColor);
                    ImGui::TextHyperlink("Github", "https://github.com/SSSiyan", defaultColor);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("V.P.Zadov"))
                {
                    ImGui::TextHyperlink("Nexus", "https://www.nexusmods.com/devilmaycry5/users/58891706", defaultColor);
                    ImGui::TextHyperlink("Youtube", "https://www.youtube.com/@missingname9779", defaultColor);
                    ImGui::TextHyperlink("Boosty", "https://boosty.to/iregretnothing", defaultColor);
                    ImGui::TextHyperlink("Github", "https://github.com/NewRussianPirate", defaultColor);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Deepdarkkapustka"))
                {
                    ImGui::TextHyperlink("Youtube", "https://www.youtube.com/@mstislavcapusta7573", defaultColor);
                    ImGui::TextHyperlink("Github", "https://github.com/muhopensores", defaultColor);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Lidemi"))
                {
                    ImGui::TextHyperlink("Nexus", "https://www.nexusmods.com/devilmaycry5/users/3813819", defaultColor);
                    ImGui::TextHyperlink("Twitter", "https://twitter.com/Phelfar", defaultColor);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Dr. Penguin"))
                {
                    ImGui::TextHyperlink("Nexus", "https://www.nexusmods.com/devilmaycry5/users/28156995", defaultColor);
                    //ImGui::TextHyperlink("Twitter", "???", defaultColor);
                    ImGui::TreePop();
                }
                ImGui::TextWrapped("Special thanks to Praydog and Cursey for their awesome work on REFramework which inspired this project!");
                if (ImGui::TreeNode("Praydog"))
                {
                    ImGui::TextHyperlink("Github", "https://github.com/praydog", defaultColor);
                    ImGui::TextHyperlink("Patreon", "https://www.patreon.com/praydog", defaultColor);
                    ImGui::TextHyperlink("Personal site", "https://praydog.com/", defaultColor);
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Cursey"))
                {
                    ImGui::TextHyperlink("Github", "https://github.com/cursey", defaultColor);
                    ImGui::TextHyperlink("Twitter", "https://twitter.com/cursey_dev", defaultColor);
                    ImGui::TextHyperlink("Personal site", "https://cursey.dev/", defaultColor);
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("License"))
            {
                ImGui::TextWrapped(license::glm);
                ImGui::Separator();
                ImGui::TextWrapped(license::imgui);
                ImGui::Separator();
                ImGui::TextWrapped(license::minhook);
                ImGui::Separator();
                ImGui::TextWrapped(license::spdlog);
                ImGui::Separator();
                ImGui::TextWrapped(license::ref);
                ImGui::Separator();
                ImGui::TextWrapped(license::jsonstthm);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void ModFramework::draw_notifs() const
{
    // Notifications
    ImGui::GetStyle().Alpha = 1.0f;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f));
    ImGui::RenderNotifications();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

bool ModFramework::create_render_target_d3d11() {
    const auto pd3d_device = m_d3d11_hook->get_device();
    const auto pd3d_swap_chain = m_d3d11_hook->get_swap_chain();

    if(pd3d_device == nullptr)
    {
        spdlog::error("[D3D11] Failed to retrieve the device for creting render targets!");
        return false;
    }

    if(pd3d_swap_chain == nullptr)
    {
        spdlog::error("[D3D11] Failed to retrieve the swap chain for creting render targets!");
        return false;
    }

    ComPtr<ID3D11Texture2D> backbuffer;

    if (FAILED(pd3d_swap_chain->GetBuffer(0, IID_PPV_ARGS(&backbuffer)))) {
        spdlog::error("[D3D11] Failed to get back buffer!");
        return false;
    }

    if(FAILED(pd3d_device->CreateRenderTargetView(backbuffer.Get(), nullptr, &m_d3d11.pd3d_backbuffer_rtv)))
    {
        spdlog::error("[D3D11] Failed to create back buffer render target view!");
        return false;
    }

    // Get backbuffer description.
    D3D11_TEXTURE2D_DESC backbuffer_desc{};

    backbuffer->GetDesc(&backbuffer_desc);

    backbuffer_desc.BindFlags |= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    // Create our blank render target.
    spdlog::info("[D3D11] Creating render targets...");

    if (FAILED(pd3d_device->CreateTexture2D(&backbuffer_desc, nullptr, &m_d3d11.pd3d_blank_rt))) {
        spdlog::error("[D3D11] Failed to create render target texture!");
        return false;
    }

    // Create our render target.
    if (FAILED(pd3d_device->CreateTexture2D(&backbuffer_desc, nullptr, &m_d3d11.pd3d_rt))) {
        spdlog::error("[D3D11] Failed to create render target texture!");
        return false;
    }

    // Create our blank render target view.
    spdlog::info("[D3D11] Creating rtvs...");

    if (FAILED(pd3d_device->CreateRenderTargetView(m_d3d11.pd3d_blank_rt.Get(), nullptr, &m_d3d11.pd3d_blank_rt_rtv))) {
        spdlog::error("[D3D11] Failed to create render terget view!");
        return false;
    }


    // Create our render target view.
    if (FAILED(pd3d_device->CreateRenderTargetView(m_d3d11.pd3d_rt.Get(), nullptr, &m_d3d11.pd3d_rt_rtv))) {
        spdlog::error("[D3D11] Failed to create render terget view!");
        return false;
    }

    // Create our render target shader resource view.
    spdlog::info("[D3D11] Creating srvs...");

    if (FAILED(pd3d_device->CreateShaderResourceView(m_d3d11.pd3d_rt.Get(), nullptr, &m_d3d11.pd3d_rt_srv))) {
        spdlog::error("[D3D11] Failed to create shader resource view!");
        return false;
    }

    return true;
}

bool ModFramework::create_rtv_descriptor_heap_d3d12() {
	const auto pd3d_device = m_d3d12_hook->get_device();

    if(pd3d_device == nullptr)
    {
        spdlog::error("[D3D12] Failed to retrieve the device for creating rtv descriptor heap!");
        return false;
    }

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.NumDescriptors = m_d3d12.buffer_count;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 1;

    if (pd3d_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_d3d12.pd3d_rtv_desc_heap)) != S_OK) {
        return false;
    }

	const SIZE_T rtv_descriptor_size = pd3d_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = m_d3d12.pd3d_rtv_desc_heap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < m_d3d12.buffer_count; i++)
    {
        m_d3d12.frame_context[i].rt_cpu_desc_handle = rtv_handle;
        rtv_handle.ptr += rtv_descriptor_size;
    }

    return true;
}

bool ModFramework::create_srv_descriptor_heap_d3d12(UINT descriptorCount) {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = descriptorCount;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (m_d3d12_hook->get_device()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_d3d12.pd3d_srv_desc_heap)) != S_OK)
    {
	    return false;
    }

    return true;
}

bool ModFramework::create_command_allocator_d3d12() {
    for (UINT i = 0; i < m_d3d12.buffer_count; i++)
    {
	    if (m_d3d12_hook->get_device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_d3d12.frame_context[i].command_allocator)) != S_OK)
	    {
		    return false;
	    }
    }

    return true;
}

bool ModFramework::create_command_list_d3d12() {
    if (m_d3d12_hook->get_device()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_d3d12.frame_context[0].command_allocator.Get(), nullptr, IID_PPV_ARGS(&m_d3d12.pd3d_command_list)) != S_OK)
    {
	    return false;
    }

    if (m_d3d12.pd3d_command_list->Close() != S_OK)
    {
        spdlog::error("[D3D12] Failed to close command list after creation.");
        return false;
    }

    return true;
}

bool ModFramework::create_render_target_d3d12()
{
    const auto pd3d_device = m_d3d12_hook->get_device();
    const auto pd3d_swap_chain = m_d3d12_hook->get_swap_chain();

    if (pd3d_device == nullptr)
    {
        spdlog::error("[D3D12] Failed to retrieve the device for creting render targets!");
        return false;
    }

    if (pd3d_swap_chain == nullptr)
    {
        spdlog::error("[D3D12] Failed to retrieve the swap chain for creting render targets!");
        return false;
    }

    for (UINT i = 0; i < m_d3d12.original_buffer_count; i++) {
        if (pd3d_swap_chain->GetBuffer(i, IID_PPV_ARGS(&m_d3d12.frame_context[i].rt)) == S_OK) {
            pd3d_device->CreateRenderTargetView(m_d3d12.frame_context[i].rt.Get(), nullptr, m_d3d12.get_cpu_rtv(pd3d_device, i));
        }
    }

    // Create our imgui and blank rts.
    const auto& backbuffer = m_d3d12.frame_context[0].rt;
    const auto bb_desc = backbuffer->GetDesc();

    spdlog::info("[D3D12] Back buffer format is {}", bb_desc.Format);

    D3D12_HEAP_PROPERTIES props{};
    props.Type = D3D12_HEAP_TYPE_DEFAULT;
    props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_CLEAR_VALUE clear_value{};
    clear_value.Format = bb_desc.Format;

    if (FAILED(pd3d_device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &bb_desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clear_value,
        IID_PPV_ARGS(&m_d3d12.frame_context[m_d3d12.original_buffer_count].rt)))) {
        spdlog::error("[D3D12] Failed to create the ui render target.");
        return false;
    }

    if (FAILED(pd3d_device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &bb_desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, &clear_value,
        IID_PPV_ARGS(&m_d3d12.frame_context[m_d3d12.original_buffer_count + 1].rt)))) {
        spdlog::error("[D3D12] Failed to create the blank render target.");
        return false;
    }

    // Create imgui and blank rtvs and srvs.
    pd3d_device->CreateRenderTargetView(m_d3d12.frame_context[m_d3d12.original_buffer_count].rt.Get(), nullptr, m_d3d12.get_cpu_rtv(pd3d_device, m_d3d12.original_buffer_count));
    pd3d_device->CreateRenderTargetView(m_d3d12.frame_context[m_d3d12.original_buffer_count + 1].rt.Get(), nullptr, m_d3d12.get_cpu_rtv(pd3d_device, m_d3d12.original_buffer_count + 1));
    pd3d_device->CreateShaderResourceView(m_d3d12.frame_context[m_d3d12.original_buffer_count].rt.Get(), nullptr, m_d3d12.get_cpu_srv(pd3d_device, 1));
    pd3d_device->CreateShaderResourceView(m_d3d12.frame_context[m_d3d12.original_buffer_count + 1].rt.Get(), nullptr, m_d3d12.get_cpu_srv(pd3d_device, 2));

    return true;
}
