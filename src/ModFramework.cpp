#include <spdlog/sinks/basic_file_sink.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h> // Wondering why? Good question

// ours with XInput removed
#include "fw-imgui/imgui_impl_win32.h"
#include "fw-imgui/imgui_impl_dx11.h"
#include "fw-imgui/imgui_impl_dx12.h"
#include "fw-imgui/ui.hpp"

#include "fw-imgui/font_robotomedium.hpp"
#include "dmc5font.hpp"
#include "logo.h"

#include "utility/Module.hpp"

#include "Mod.hpp"
#include "Mods.hpp"

#include "LicenseStrings.hpp"
#include "ModFramework.hpp"

#include "Config.hpp"

// clang-format off

std::unique_ptr<ModFramework> g_framework{};

ModFramework::ModFramework()
    : m_game_module{ GetModuleHandle(0) },
    m_logger{ spdlog::basic_logger_mt("ModFramework", LOG_FILENAME, true) }
{
    spdlog::set_default_logger(m_logger);
    spdlog::flush_on(spdlog::level::info);
    spdlog::info(LOG_ENTRY);

    // Preparing the logo
    logo.width_resized = 800;
    logo.height_resized = 76;
    logo.rgba_data = new unsigned char[UI::stb_decompress_length((const unsigned char*)logo.data_compressed)];
    UI::stb_decompress(logo.rgba_data, (const unsigned char*)logo.data_compressed, logo.size);
    logo.rgba_resized = new unsigned char[logo.width_resized*logo.height_resized*4];
    UI::stb_resize(logo.rgba_data, logo.width, logo.height, 0, logo.rgba_resized, logo.width_resized, logo.height_resized, 0, 4);
    delete[] logo.rgba_data;

#ifdef DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif

    if (!hook_d3d12()) {
        spdlog::error("Failed to hook D3D12 for initial test.");
    }
}

ModFramework::~ModFramework() {
    delete[] logo.rgba_resized;

	if (m_is_d3d11) ImGui_ImplDX11_Shutdown();
    if (m_is_d3d12) ImGui_ImplDX12_Shutdown();

    ImGui_ImplWin32_Shutdown();
	if (m_initialized)
	{
		ImGui::DestroyContext();
	}

    // Save config if the game gets closed
    save_config();
}

bool ModFramework::hook_d3d11()
{
	m_d3d11_hook = std::make_unique<D3D11Hook>();
	m_d3d11_hook->on_present([this](D3D11Hook& hook) { on_frame_d3d11(); });
	m_d3d11_hook->on_resize_buffers([this](D3D11Hook& hook) { on_reset(); });

    // Making sure D3D12 is not hooked
	if (!m_is_d3d12) {
		if (m_d3d11_hook->hook()) {
			spdlog::info("Hooked DirectX 11");
			m_valid = true;
			m_is_d3d11 = true;
			return true;
		}
		else {
			// We make sure to no unhook any unwanted hooks if D3D11 didn't get hooked properly
			if (m_d3d11_hook->unhook())
				spdlog::info("D3D11 unhooked!");
			else
				spdlog::info("Cannot unhook D3D11, this might crash.");

			m_valid = false;
			m_is_d3d11 = false;
			return false;
		}
	}

    return false;
}

bool ModFramework::hook_d3d12()
{
	m_d3d12_hook = std::make_unique<D3D12Hook>();
	m_d3d12_hook->on_present([this](D3D12Hook& hook) { on_frame_d3d12(); });
	m_d3d12_hook->on_resize_buffers([this](D3D12Hook& hook) { on_reset(); });
	m_d3d12_hook->on_resize_target([this](D3D12Hook& hook) { /*on_reset();*/ });
	m_d3d12_hook->on_create_swap_chain([this](D3D12Hook& hook) { m_pd3d_command_queue_d3d12 = m_d3d12_hook->get_command_queue(); });

    // Making sure D3D11 is not hooked
	if (!m_is_d3d11) {
		if (m_d3d12_hook->hook()) {
			spdlog::info("Hooked DirectX 12");
			m_valid = true;
			m_is_d3d12 = true;
			return true;
		}
		else {
			// We make sure to no unhook any unwanted hooks if D3D12 didn't get hooked properly
			if (m_d3d12_hook->unhook())
				spdlog::info("D3D12 Unhooked!");
			else
				spdlog::info("Cannot unhook D3D12, this might crash.");

			m_valid = false;
			m_is_d3d12 = false;
			return false;
		}
	}

    return false;
}

void ModFramework::set_style() noexcept {
    ImGui::StyleColorsDark();

    auto& style = ImGui::GetStyle();
    style.WindowRounding    = 0.0f;
    style.ChildRounding     = 0.0f;
    style.PopupRounding     = 0.0f;
    style.FrameRounding     = 0.0f;
    style.ScrollbarSize     = 8.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding      = 0.0f;
    style.TabRounding       = 5.0f;
    style.WindowBorderSize  = 2.0f;
    style.WindowPadding     = ImVec2(8.0f, 5.0f);
    style.ItemSpacing.y     = 8.0f;

    auto& colors = ImGui::GetStyle().Colors;

    ImVec4 (*color_htof)(uint32_t) = [](uint32_t rgba){

        return ImVec4{
            ((float)((rgba >> 24) & 0x000000FF) / 255),
            ((float)((rgba >> 16) & 0x000000FF) / 255), 
            ((float)((rgba >> 8) & 0x000000FF) / 255),
            ((float)(rgba & 0x000000FF) / 255)
        };
    };

    // Constants to be able to change color schemes later easier
    /*
    // Black and white scheme:
    const ImVec4 color_normal(0.2f, 0.205f, 0.21f, 1.0f);
    const ImVec4 color_hovered(0.3f, 0.305f, 0.31f, 1.0f);
    const ImVec4 color_active(0.55f, 0.5505f, 0.551f, 1.0f);
    const ImVec4 color_title_bg(0.25f, 0.2505f, 0.251f, 1.0f);
    const ImVec4 color_focused_active(0.265f, 0.2655f, 0.266f, 1.0f);
    const ImVec4 color_unfocused(0.18f, 0.1805f, 0.181f, 1.0f);
    const ImVec4 color_collapsed(0.55f, 0.5505f, 0.551f, 1.0f);
    */
	/*

    const ImVec4 color_normal = color_htof(0x7DE8E8FF);
    const ImVec4 color_hovered = color_htof(0x5616BCFF);
    const ImVec4 color_active =  color_htof(0x7827CCFF);
    const ImVec4 color_title_bg(0.25f, 0.2505f, 0.251f, 1.0f);
    const ImVec4 color_focused_active(0.265f, 0.2655f, 0.266f, 1.0f);
    const ImVec4 color_unfocused(0.18f, 0.1805f, 0.181f, 1.0f);
    const ImVec4 color_collapsed(0.55f, 0.5505f, 0.551f, 1.0f);

    // Window BG
    colors[ImGuiCol_WindowBg] = color_htof(0x0B0019ff);

    // Navigatation highlight
    colors[ImGuiCol_NavHighlight] = color_hovered;

    // Headers
    colors[ImGuiCol_Header] = color_normal;
    colors[ImGuiCol_HeaderHovered] = color_hovered;
    colors[ImGuiCol_HeaderActive] = color_active;

    // Buttons
    colors[ImGuiCol_Button] = color_normal;
    colors[ImGuiCol_ButtonHovered] = color_hovered;
    colors[ImGuiCol_ButtonActive] = color_active;

    // Checkmark
    colors[ImGuiCol_CheckMark] = color_active;

    // Slider
    colors[ImGuiCol_SliderGrab] = color_normal;
    colors[ImGuiCol_SliderGrabActive] = color_active;

    // Frame BG
    colors[ImGuiCol_FrameBg] = color_normal;
    colors[ImGuiCol_FrameBgHovered] = color_hovered;
    colors[ImGuiCol_FrameBgActive] = color_active;

    // Tabs
    colors[ImGuiCol_Tab] = color_normal;
    colors[ImGuiCol_TabHovered] = color_hovered;
    colors[ImGuiCol_TabActive] = color_active;
    colors[ImGuiCol_TabUnfocused] = color_unfocused;
    colors[ImGuiCol_TabUnfocusedActive] = color_unfocused;

    // Resize Grip
    colors[ImGuiCol_ResizeGrip] = color_normal;
    colors[ImGuiCol_ResizeGripHovered] = color_hovered;
    colors[ImGuiCol_ResizeGripActive] = color_active;

    // Title
    colors[ImGuiCol_TitleBg] = color_title_bg;
    colors[ImGuiCol_TitleBgActive] = color_active;
    colors[ImGuiCol_TitleBgCollapsed] = color_collapsed;

    // Borders
    colors[ImGuiCol_Border] = color_normal;
    colors[ImGuiCol_BorderShadow] = color_normal;
*/

    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.50f, 0.93f, 0.93f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.29f, 0.29f, 0.29f, 0.00f);
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
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.67f, 0.17f, 0.18f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.67f, 0.17f, 0.18f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.67f, 0.17f, 0.18f, 0.82f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.02f, 0.02f, 0.02f, 0.86f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.09f, 0.60f, 0.64f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.14f, 0.38f, 0.47f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
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
    io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 16.0f);
}

void ModFramework::consume_input() {
    m_mouse_delta[0] = m_accumulated_mouse_delta[0];
    m_mouse_delta[1] = m_accumulated_mouse_delta[1];

    m_accumulated_mouse_delta[0] = 0.0f;
    m_accumulated_mouse_delta[1] = 0.0f;
}

bool ModFramework::on_message(HWND wnd, UINT message, WPARAM w_param, LPARAM l_param) {
    if (!m_initialized) {
       return true;
    }

    bool is_mouse_moving = false;
    switch (message) {
    case WM_INPUT: {
        // RIM_INPUT means the window has focus
        if (GET_RAWINPUT_CODE_WPARAM(w_param) == RIM_INPUT) {
            uint32_t size = sizeof(RAWINPUT);
            RAWINPUT raw{};
            
            // obtain size
            GetRawInputData((HRAWINPUT)l_param, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));

            auto result = GetRawInputData((HRAWINPUT)l_param, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));

            if (raw.header.dwType == RIM_TYPEMOUSE) {
                m_accumulated_mouse_delta[0] += (float)raw.data.mouse.lLastX;
                m_accumulated_mouse_delta[1] += (float)raw.data.mouse.lLastY;

                // Allowing camera movement when the UI is hovered while not focused
                if (raw.data.mouse.lLastX || raw.data.mouse.lLastY) {
                    is_mouse_moving = true;
                }
            }
        }
    } break;

    case RE_TOGGLE_CURSOR: {
        if (!m_is_internal_message) {
            m_cursor_state = w_param;

            if (m_draw_ui && !w_param)
                return ImGui_ImplWin32_WndProcHandler(wnd, RE_TOGGLE_CURSOR, true, l_param);
        }
    } break;
    default:
        break;
    }
    m_is_internal_message = false;

    ImGui_ImplWin32_WndProcHandler(wnd, message, w_param, l_param);

    {
        // If the user is interacting with the UI we block the message from going to the game.
        const auto& io = ImGui::GetIO();
        if (m_draw_ui) {
            // Fix of a bug that makes the input key down register but the key up will never register \
            when clicking on the ui while the game is not focused
            if (message == WM_INPUT && GET_RAWINPUT_CODE_WPARAM(w_param) == RIM_INPUTSINK)
                return false;

            if (m_is_ui_focused) {
                if (is_mouse_moving || io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput)
                    return false;
            } else {
                if (!is_mouse_moving && (io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput))
                    return false;
            }
        }
    }

    return true;
}

// this is unfortunate.
void ModFramework::on_direct_input_keys(const std::array<uint8_t, 256>& keys) {
    if (keys[m_menu_key] && m_last_keys[m_menu_key] == 0) {
        std::lock_guard _{ m_input_mutex };
        m_draw_ui = !m_draw_ui;

        if (!m_draw_ui && m_game_data_initialized) {
            save_config();
        }
    }

    if (m_initialized && !ImGui::GetIO().WantTextInput) {
        if (keys[m_close_menu_key] && m_last_keys[m_close_menu_key] == 0 && m_draw_ui && m_is_ui_focused) {
            m_draw_ui = false;

            if (m_game_data_initialized) {
                save_config();
            }
        }
        else if (keys[m_close_menu_key] && m_last_keys[m_close_menu_key] && !m_draw_ui) {
            m_dinput_hook->ignore_input();
        }
        else if (keys[m_close_menu_key] == 0 && m_last_keys[m_close_menu_key] && !m_draw_ui) {
            m_dinput_hook->acknowledge_input();
        }
    }

    m_last_keys = keys;
}


void ModFramework::on_frame_d3d11() {
    spdlog::debug("on_frame (D3D11)");

    if (!m_initialized) {
        if (!initialize()) {
            spdlog::error("Failed to initialize the trainer on DirectX 11");
            return;
        }

        spdlog::info("Trainer initialized");
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
    m_last_draw_ui = m_draw_ui;

    ImGui::EndFrame();
    ImGui::Render();

    ID3D11DeviceContext* context = nullptr;
    m_d3d11_hook->get_device()->GetImmediateContext(&context);

    context->OMSetRenderTargets(1, &m_main_render_target_view_d3d11, NULL);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void ModFramework::on_frame_d3d12() {
    spdlog::debug("on_frame (D3D12)");

    if (!m_initialized) {
        if (!initialize()) {
            spdlog::error("Failed to initialize the trainer on DirectX 12");
            return;
        }

        spdlog::info("Trainer initialized");
        m_initialized = true;
        return;
    }
	
    if (m_pd3d_command_queue_d3d12 == nullptr) {
		spdlog::error("Null Command Queue");
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
    m_last_draw_ui = m_draw_ui;

    ImGui::EndFrame();
    //ImGui::UpdatePlatformWindows();
    ImGui::Render();

	//Rendering
	UINT back_buffer_idx = m_d3d12_hook->get_swap_chain()->GetCurrentBackBufferIndex();
	FrameContext_D3D12* frame_context = &m_frame_context_d3d12[back_buffer_idx];
	frame_context->CommandAllocator->Reset();

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_main_render_target_resource_d3d12[back_buffer_idx];
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	m_pd3d_command_list_d3d12->Reset(frame_context->CommandAllocator, NULL);
	m_pd3d_command_list_d3d12->ResourceBarrier(1, &barrier);

	// Render Dear ImGui graphics
	m_pd3d_command_list_d3d12->OMSetRenderTargets(1, &m_main_render_target_descriptor_d3d12[back_buffer_idx], FALSE, NULL);
	m_pd3d_command_list_d3d12->SetDescriptorHeaps(1, &m_pd3d_srv_desc_heap_d3d12);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_pd3d_command_list_d3d12);
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_pd3d_command_list_d3d12->ResourceBarrier(1, &barrier);
	m_pd3d_command_list_d3d12->Close();

	m_pd3d_command_queue_d3d12->ExecuteCommandLists(1, (ID3D12CommandList* const*)&m_pd3d_command_list_d3d12);
}

void ModFramework::on_reset() {
    spdlog::info("Reset!");

    // Crashes if we don't release it at this point.
    if (m_is_d3d11) cleanup_render_target_d3d11();

    if (m_is_d3d12) {
        // For some reason if we don't destroy the context and recreate it later, the text fields will not work.
		if (m_initialized)
		{
			ImGui::DestroyContext();
		}
		cleanup_render_target_d3d12();
    }

    m_initialized = false;
}

void ModFramework::save_config() {
    spdlog::info("Saving config to file");

    utility::Config cfg{};

    for (auto& mod : m_mods->get_mods()) {
        mod->on_config_save(cfg);
    }

    if (!cfg.save(CONFIG_FILENAME)) {
        spdlog::info("Failed to save config");
        return;
    }

    spdlog::info("Saved config");
}

bool ModFramework::initialize() {
    if (m_initialized) {
        return true;
    }

    if (m_is_d3d11) {
        spdlog::info("Attempting to initialize DirectX 11");

		if (!m_d3d11_hook->is_hooked()) {
			return false;
		}

        auto device = m_d3d11_hook->get_device();
        auto swap_chain = m_d3d11_hook->get_swap_chain();

        // Wait.
        if (device == nullptr || swap_chain == nullptr) {
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

        ID3D11DeviceContext* context = nullptr;
        device->GetImmediateContext(&context);

        swap_chain->GetDesc(&m_swap_desc);

        m_wnd = m_swap_desc.OutputWindow;

        // Explicitly call destructor first
        m_windows_message_hook.reset();
        m_windows_message_hook = std::make_unique<WindowsMessageHook>(m_wnd);
        m_windows_message_hook->on_message = [this](auto wnd, auto msg, auto wParam, auto lParam) {
            return on_message(wnd, msg, wParam, lParam);
        };

        // just do this instead of rehooking because there's no point.
        if (m_first_frame) {
            m_dinput_hook = std::make_unique<DInputHook>(m_wnd);
        }
        else {
            m_dinput_hook->set_window(m_wnd);
        }

        spdlog::info("Creating render target");

        create_render_target_d3d11();

        spdlog::info("Window Handle: {0:x}", (uintptr_t)m_wnd);
        spdlog::info("Initializing ImGui");

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        set_style();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

        spdlog::info("Initializing ImGui Win32");

        if (!ImGui_ImplWin32_Init(m_wnd)) {
            spdlog::error("Failed to initialize ImGui.");
            return false;
        }

        spdlog::info("Initializing ImGui D3D11");

        if (!ImGui_ImplDX11_Init(device, context)) {
            spdlog::error("Failed to initialize ImGui.");
            return false;
        }

        //Loading the logo for DX11
        m_logo_dx11 = UI::Texture2DDX11(logo.rgba_resized, logo.width_resized, logo.height_resized, device);
    }
    else if (m_is_d3d12) {
        spdlog::info("Attempting to initialize DirectX 12");

		if (!m_d3d12_hook->is_hooked()) {
			return false;
		}

		auto device = m_d3d12_hook->get_device();
		auto swap_chain = m_d3d12_hook->get_swap_chain();

		if (device == nullptr || swap_chain == nullptr || m_pd3d_command_queue_d3d12 == nullptr) {
			spdlog::info("Device or SwapChain null. DirectX 11 may be in use. Unhooking D3D12...");

			// We unhook D3D12
			if (m_d3d12_hook->unhook())
				spdlog::info("D3D12 unhooked!");
			else
				spdlog::error("Cannot unhook D3D12, this might crash.");
            
			m_valid = false;
			m_is_d3d12 = false;

			// We hook D3D11
            if (!hook_d3d11()) {
                spdlog::error("Failed to hook D3D11 after unhooking D3D12.");
			}
			return false;
		}

        swap_chain->GetDesc(&m_swap_desc);

		m_wnd = m_swap_desc.OutputWindow;

		m_windows_message_hook.reset();
		m_windows_message_hook = std::make_unique<WindowsMessageHook>(m_wnd);
		m_windows_message_hook->on_message = [this](auto wnd, auto msg, auto wParam, auto lParam) {
			return on_message(wnd, msg, wParam, lParam);
		};

		if (m_first_frame) {
			m_dinput_hook = std::make_unique<DInputHook>(m_wnd);
		}
		else {
			m_dinput_hook->set_window(m_wnd);
		}

		if (!create_rtv_descriptor_heap_d3d12()) {
            spdlog::error("Failed to create RTV Descriptor.");
			return false;
		}

		if (!create_srv_descriptor_heap_d3d12()) {
            spdlog::error("Failed to create SRV Descriptor.");
			return false;
		}

		if (!create_command_allocator_d3d12()) {
            spdlog::error("Failed to create Command Allocator.");
			return false;
		}

		if (!create_command_list_d3d12()) {
            spdlog::error("Failed to create Command List.");
			return false;
		}

		create_render_target_d3d12();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		set_style();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		if (!ImGui_ImplWin32_Init(m_wnd)) {
            spdlog::error("Failed to initialize ImGui ImplWin32.");
			return false;
		}

		if (!ImGui_ImplDX12_Init(device, m_NUM_FRAMES_IN_FLIGHT_D3D12,
            DXGI_FORMAT_R8G8B8A8_UNORM, m_pd3d_srv_desc_heap_d3d12,
            m_pd3d_srv_desc_heap_d3d12->GetCPUDescriptorHandleForHeapStart(),
            m_pd3d_srv_desc_heap_d3d12->GetGPUDescriptorHandleForHeapStart()))
		{
            spdlog::error("Failed to initialize ImGui ImplDX12.");
			return false;
		}

		ImGui_ImplDX12_InvalidateDeviceObjects();
		if (!ImGui_ImplDX12_CreateDeviceObjects()) {
            spdlog::error("Failed to initialize ImGui CreateDeviceObjects.");
			return false;
		}

        //Loading the logo for DX12
        m_logo_dx12 = UI::Texture2DDX12(logo.rgba_resized, logo.width_resized, logo.height_resized, device, m_pd3d_srv_desc_heap_d3d12);

		/*m_target_width = m_d3d12_hook->get_display_width();
		m_target_height = m_d3d12_hook->get_display_height();

		m_render_width = m_d3d12_hook->get_render_width();
		m_render_height = m_d3d12_hook->get_render_height();*/
    }
    else { return false; }
    
	if (m_first_frame) {
		m_first_frame = false;

		spdlog::info("Starting game data initialization thread");

		// Game specific initialization stuff
		std::thread init_thread([this]() {
			m_mods = std::make_unique<Mods>();

			auto e = m_mods->on_initialize();

			if (e) {
				if (e->empty()) {
					m_error = "An unknown error has occurred.";
				}
				else {
					m_error = *e;
				}
			}

			m_game_data_initialized = true;
			});

		init_thread.detach();
	}

    return true;
}

void ModFramework::draw_ui() {
    std::lock_guard _{ m_input_mutex };

    if (!m_draw_ui) {
        m_is_ui_focused = false;
        if (m_last_draw_ui) {
            m_is_internal_message = true;
            m_windows_message_hook->window_toggle_cursor(m_cursor_state);
        }
        m_dinput_hook->acknowledge_input();
        // ImGui::GetIO().MouseDrawCursor = false;
        return;
    }


    // UI Specific code:
    m_is_ui_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);

    auto& style = ImGui::GetStyle();
    if (m_is_ui_focused) {
        style.Alpha = 1.0f;
    } else {
        if (ImGui::IsWindowHovered(ImGuiFocusedFlags_AnyWindow)) {
            style.Alpha = 0.7f;
        } else {
            style.Alpha = 0.4f;
        }
    }
      

    auto& io = ImGui::GetIO();

	if (m_swap_desc.BufferDesc.Width > 0 && m_swap_desc.BufferDesc.Height > 0) {
		//size = ImVec2((float)swap_desc.BufferDesc.Width, (float)swap_desc.BufferDesc.Height);
		io.DisplayFramebufferScale = ImVec2(
			(float)m_swap_desc.BufferDesc.Width / io.DisplaySize.x,
			(float)m_swap_desc.BufferDesc.Height / io.DisplaySize.y);
	}

    if (io.WantCaptureKeyboard || m_is_ui_focused) {
        m_dinput_hook->ignore_input();
    }
    else {
        m_dinput_hook->acknowledge_input();
    }

    if (!m_last_draw_ui) {
        m_is_internal_message = true;
        m_windows_message_hook->window_toggle_cursor(true);
    }

	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavFocus;
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGuiWindowClass windowClass;
    windowClass.DockNodeFlagsOverrideClear = 000;
    windowClass.DockNodeFlagsOverrideSet = 000;
    ImGui::SetNextWindowClass(&windowClass);

    ImGui::SetNextWindowPos(m_window_pos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(m_window_size, ImGuiCond_Once);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.93f, 0.93f, 1.00f));

    ImGui::Begin("##SSSiyan's Collaborative Trainer", &m_draw_ui, windowFlags);

    m_window_pos = ImGui::GetWindowPos();
    m_window_size = ImGui::GetWindowSize();
    ImGui::PopStyleColor();

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetWindowSize().x/2.f - (float)logo.width_resized/2);
    if(m_is_d3d11)
        ImGui::Image(m_logo_dx11, ImVec2(m_logo_dx11.GetWidth(), m_logo_dx11.GetHeight()));
    else
        ImGui::Image(m_logo_dx12, ImVec2(m_logo_dx12.GetWidth(), m_logo_dx12.GetHeight()));

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 15.f);
    //ImGui::Text("Menu Key: Delete");
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetWindowSize().x/2.f - ImGui::CalcTextSize("Save Settings Load Settings").x/2);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

    if (ImGui::Button("Save Settings")) { m_mods->save_mods(); };
    ImGui::SameLine(); if (ImGui::Button("Load Settings")) { m_mods->load_mods(); };
    
    ImGui::PopStyleVar();
    
    ImGui::SetCursorPosY(ImGui::GetCursorPosY());
    ImGui::Separator();
/*

    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.1f, 0.1005f, 0.101f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.05f, 0.0505f, 0.051f, 1.0f));
*/

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.50f, 0.93f, 0.93f, 1.00f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.26f, 0.59f, 0.98f, 0.00f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.05f, 0.11f, 0.20f, 1.00f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.07f, 0.07f, 0.07f, 1.00f));
    
    //ImGui::SliderFloat("Label", &ttt, 0.0f, 20.0f, "%f", 1.0f);

    ImGuiID dockSpaceId = ImGui::GetID("SSSiyan's Collaborative Trainer");
    if (!ImGui::DockBuilderGetNode(dockSpaceId))
    {
        ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockSpaceId, ImVec2(viewport->Size.x*1.1f, viewport->Size.y));

        ImGuiID dockMainId = dockSpaceId;
        ImGuiID dockRightId = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Right, 0.48f, nullptr, &dockMainId);

        ImGui::DockBuilderDockWindow("Mods", dockMainId);

        ImGui::DockBuilderDockWindow("Mod Settings", dockRightId);

        ImGui::DockBuilderFinish(dockMainId);
    }

    ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton);
    ImGui::End();
    
    ImGuiWindowFlags panel_flags =   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavFocus |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;

    {
		ImGui::Begin("Mods", NULL, panel_flags);

        float tabSpacingOffset = -5.0f;

        ImVec2 tabBtnSize(0.0f, 25.0f);
        float tabBtnRounding = 5.0f;

		if (UI::TabBtn("Gameplay", m_focused_panel == PanelID_Gameplay ? true : false, tabBtnSize, tabBtnRounding))
			m_focused_panel = PanelID_Gameplay;
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + tabSpacingOffset);

		if (UI::TabBtn("Scenario", m_focused_panel == PanelID_Scenario ? true : false, tabBtnSize, tabBtnRounding))
			m_focused_panel = PanelID_Scenario;
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + tabSpacingOffset);

		if (UI::TabBtn("System", m_focused_panel == PanelID_System ? true : false, tabBtnSize, tabBtnRounding))
			m_focused_panel = PanelID_System;
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + tabSpacingOffset);

		if (UI::TabBtn("Nero", m_focused_panel == PanelID_Nero ? true : false, tabBtnSize, tabBtnRounding))
			m_focused_panel = PanelID_Nero;
		ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + tabSpacingOffset);

		if (UI::TabBtn("Dante", m_focused_panel == PanelID_Dante ? true : false, tabBtnSize, tabBtnRounding))
			m_focused_panel = PanelID_Dante;
		ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + tabSpacingOffset);

		if (UI::TabBtn("V", m_focused_panel == PanelID_Gilver ? true : false, tabBtnSize, tabBtnRounding))
			m_focused_panel = PanelID_Gilver;
		ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + tabSpacingOffset);

		if (UI::TabBtn("Vergil", m_focused_panel == PanelID_Vergil ? true : false, tabBtnSize, tabBtnRounding))
			m_focused_panel = PanelID_Vergil;
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 9.0f);

        ImGui::PushStyleColor(ImGuiCol_Separator, OUTLINE_NORM);
        ImGui::Separator();
        ImGui::PopStyleColor();

		draw_panel(m_focused_panel);
		ImGui::End();
    }

    {
        ImGui::PushStyleColor(ImGuiCol_Text, ELEMENT_TEXT_ACT);
        ImGui::Begin("Mod Settings", NULL, panel_flags);
        ImGui::PopStyleColor();

        auto& current_mod = m_mods->get_mod(m_mods->get_focused_mod());

        ImGui::TextWrapped("Selected Mod: %s", current_mod->full_name_string.c_str());
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
        ImGui::TextWrapped("Description: %s", current_mod->description_string.c_str());
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
        ImGui::TextWrapped("Author: %s", current_mod->author_string.c_str());
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);

        ImGui::Separator();

        current_mod->on_draw_ui();

        ImGui::End();
    }

    ImGui::PopStyleColor(6);
    ImGui::PopStyleVar();

    //ImGui::PopStyleColor(3);
}

void ModFramework::draw_panel(PanelID panelID)
{
    PanelID current_focused_panel;

    ImGuiWindowFlags panel_flags =   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavFocus |
                                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;
    
    float modListIndent = 10.0f;
    float sprtSpcYOfst = 0.0f;

    switch (panelID) {
        case PanelID_Gameplay:
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ELEMENT_TEXT_ACT);
                ImGui::PopStyleColor();

                if(ImGui::IsWindowFocused())
                    current_focused_panel = PanelID_Gameplay;

                if (m_error.empty() && m_game_data_initialized) {
                    ImGui::Text("Common Mechanics");
	                m_mods->on_pagelist_ui(Mod::mechanics, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("Enemy Step");
                    m_mods->on_pagelist_ui(Mod::enemystep, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("Shared Cheats");
                    m_mods->on_pagelist_ui(Mod::commoncheat, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("Taunts");
                    m_mods->on_pagelist_ui(Mod::taunt, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("Animation");
                    m_mods->on_pagelist_ui(Mod::animation, modListIndent);
                }
                else if (!m_game_data_initialized) {
                    ImGui::TextWrapped("Trainer is currently initializing...");
                }
                else if(!m_error.empty()) {
                    ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
                }
            }
            break;
        
        case PanelID_Scenario:
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ELEMENT_TEXT_ACT);
                ImGui::PopStyleColor();

                if(ImGui::IsWindowFocused())
                    current_focused_panel = PanelID_Scenario;

                if (m_error.empty() && m_game_data_initialized) {
                    ImGui::Text("Game Modes");
	            	m_mods->on_pagelist_ui(Mod::gamemode, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("Bloody Palace");
                    m_mods->on_pagelist_ui(Mod::bloodypalace, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("Game Balance");
                    m_mods->on_pagelist_ui(Mod::balance, modListIndent);
                }
                else if (!m_game_data_initialized) {
                    ImGui::TextWrapped("Trainer is currently initializing...");
                }
                else if(!m_error.empty()) {
                    ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
                }
            }
            break;

        case PanelID_System:
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ELEMENT_TEXT_ACT);
                ImGui::PopStyleColor();

                if(ImGui::IsWindowFocused())
                    current_focused_panel = PanelID_System;

                if (m_error.empty() && m_game_data_initialized) {
                    ImGui::Text("Camera");
	            	m_mods->on_pagelist_ui(Mod::camera, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("Quality-of-life");
                    m_mods->on_pagelist_ui(Mod::qol, modListIndent);
                }
                else if (!m_game_data_initialized) {
                    ImGui::TextWrapped("Trainer is currently initializing...");
                }
                else if(!m_error.empty()) {
                    ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
                }
            }
            break;

        case PanelID_Nero:
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ELEMENT_TEXT_ACT);
                ImGui::PopStyleColor();

                if(ImGui::IsWindowFocused())
                    current_focused_panel = PanelID_Nero;

                if (m_error.empty() && m_game_data_initialized) {
                    ImGui::Text("Breaker");
	            	m_mods->on_pagelist_ui(Mod::breaker, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("Wiresnatch");
                    m_mods->on_pagelist_ui(Mod::wiresnatch, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("Cheats");
                    m_mods->on_pagelist_ui(Mod::nero, modListIndent);
                }
                else if (!m_game_data_initialized) {
                    ImGui::TextWrapped("Trainer is currently initializing...");
                }
                else if(!m_error.empty()) {
                    ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
                }
            }
            break;

        case PanelID_Dante:
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ELEMENT_TEXT_ACT);
                ImGui::PopStyleColor();

                if(ImGui::IsWindowFocused())
                    current_focused_panel = PanelID_Dante;

                if (m_error.empty() && m_game_data_initialized) {
                    ImGui::Text("SDT");
	            	m_mods->on_pagelist_ui(Mod::dantesdt, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("Cheats");
                    m_mods->on_pagelist_ui(Mod::dantecheat, modListIndent);
                }
                else if (!m_game_data_initialized) {
                    ImGui::TextWrapped("Trainer is currently initializing...");
                }
                else if(!m_error.empty()) {
                    ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
                }
            }
            break;

        case PanelID_Gilver:
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ELEMENT_TEXT_ACT);
                ImGui::PopStyleColor();

                if(ImGui::IsWindowFocused())
                    current_focused_panel = PanelID_Gilver;

                if (m_error.empty() && m_game_data_initialized) {
                    ImGui::Text("V cheats");
	            	m_mods->on_pagelist_ui(Mod::gilver, modListIndent);
                }
                else if (!m_game_data_initialized) {
                    ImGui::TextWrapped("Trainer is currently initializing...");
                }
                else if(!m_error.empty()) {
                    ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
                }
            }
            break;

        case PanelID_Vergil:
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ELEMENT_TEXT_ACT);
                ImGui::PopStyleColor();

                if(ImGui::IsWindowFocused())
                    current_focused_panel = PanelID_Vergil;

                if (m_error.empty() && m_game_data_initialized) {
                    ImGui::Text("SDT");
	            	m_mods->on_pagelist_ui(Mod::vergilsdt, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("Dark Slayer");
                    m_mods->on_pagelist_ui(Mod::vergiltrick, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("Cheats");
                    m_mods->on_pagelist_ui(Mod::vergilcheat, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("DoppelGanger");
                    m_mods->on_pagelist_ui(Mod::vergildoppel, modListIndent);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sprtSpcYOfst);

                    ImGui::Separator();
                    ImGui::Text("EFX Settings");
                    m_mods->on_pagelist_ui(Mod::vergilefxsettings, modListIndent);
                }
                else if (!m_game_data_initialized) {
                    ImGui::TextWrapped("Trainer is currently initializing...");
                }
                else if(!m_error.empty()) {
                    ImGui::TextWrapped("Trainer error: %s", m_error.c_str());
                }
            }
            break;
    }

    m_last_focused_panel = current_focused_panel;
}

void ModFramework::create_render_target_d3d11() {
    cleanup_render_target_d3d11();

    ID3D11Texture2D* back_buffer{ nullptr };
    if (m_d3d11_hook->get_swap_chain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer) == S_OK) {
        m_d3d11_hook->get_device()->CreateRenderTargetView(back_buffer, NULL, &m_main_render_target_view_d3d11);
        back_buffer->Release();
    }
}

void ModFramework::cleanup_render_target_d3d11() {
    if (m_main_render_target_view_d3d11 != nullptr) {
        m_main_render_target_view_d3d11->Release();
        m_main_render_target_view_d3d11 = nullptr;
    }
}

bool ModFramework::create_rtv_descriptor_heap_d3d12() {
	auto device = m_d3d12_hook->get_device();

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = m_NUM_BACK_BUFFERS_D3D12;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 1;
	if (device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pd3d_rtv_desc_heap_d3d12)) != S_OK)
		return false;

	SIZE_T rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = m_pd3d_rtv_desc_heap_d3d12->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_NUM_BACK_BUFFERS_D3D12; i++)
	{
		m_main_render_target_descriptor_d3d12[i] = rtv_handle;
		rtv_handle.ptr += rtv_descriptor_size;
	}

	return true;
}

bool ModFramework::create_srv_descriptor_heap_d3d12() {
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (m_d3d12_hook->get_device()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pd3d_srv_desc_heap_d3d12)) != S_OK)
		return false;

	return true;
}

bool ModFramework::create_command_allocator_d3d12() {
	for (UINT i = 0; i < m_NUM_FRAMES_IN_FLIGHT_D3D12; i++)
		if (m_d3d12_hook->get_device()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frame_context_d3d12[i].CommandAllocator)) != S_OK)
			return false;
	return true;
}

bool ModFramework::create_command_list_d3d12() {
	if (m_d3d12_hook->get_device()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frame_context_d3d12[0].CommandAllocator, NULL, IID_PPV_ARGS(&m_pd3d_command_list_d3d12)) != S_OK || m_pd3d_command_list_d3d12->Close() != S_OK)
		return false;
	return true;
}

void ModFramework::cleanup_render_target_d3d12()
{
	for (UINT i = 0; i < m_NUM_BACK_BUFFERS_D3D12; i++) {
		if (m_main_render_target_resource_d3d12[i]) {
			m_main_render_target_resource_d3d12[i]->Release();
			m_main_render_target_resource_d3d12[i] = NULL;
		}
	}
}

void ModFramework::create_render_target_d3d12()
{
	cleanup_render_target_d3d12();

	for (UINT i = 0; i < m_NUM_BACK_BUFFERS_D3D12; i++) {
		ID3D12Resource* back_buffer{ nullptr };
		if (m_d3d12_hook->get_swap_chain()->GetBuffer(i, IID_PPV_ARGS(&back_buffer)) == S_OK) {
			m_d3d12_hook->get_device()->CreateRenderTargetView(back_buffer, NULL, m_main_render_target_descriptor_d3d12[i]);
			m_main_render_target_resource_d3d12[i] = back_buffer;
		}
	}
}