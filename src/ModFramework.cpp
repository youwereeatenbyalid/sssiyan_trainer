#include <spdlog/sinks/basic_file_sink.h>

#include <imgui/imgui.h>

// ours with XInput removed
//#include "fw-imgui/imgui_impl_win32.h"
//#include "fw-imgui/imgui_impl_dx11.h"

#include "utility/Module.hpp"

#include "Mods.hpp"

#include "LicenseStrings.hpp"
#include "ModFramework.hpp"

#include "Config.hpp"

#include "WindowLayout/sample.h"
#include "ImWindowDX11/ImwWindowManagerDX11.h"

std::unique_ptr<ModFramework> g_framework{};

bool draw_imwindow = false;
bool should_quit = false;

void update_thread_func(ModFramework* mf) {
	spdlog::info("update thread entry");

	PreInitSample();

	ImWindow::ImwWindowManagerDX11 o_mgr(true);

	o_mgr.Init();

	InitSample(mf);

	while (o_mgr.Run(false) && o_mgr.Run(draw_imwindow)) {
		mf->on_frame();
		if (GetAsyncKeyState(VK_INSERT) & 1) {
			auto mpw = o_mgr.GetMainPlatformWindow();
			draw_imwindow = !draw_imwindow;
			mpw->Show(draw_imwindow);
		}
		if (should_quit) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(17));
	}

	ImGui::Shutdown();
}

ModFramework::ModFramework()
    : m_game_module{ GetModuleHandle(0) },
    m_logger{ spdlog::basic_logger_mt("ModFramework", LOG_FILENAME, true) }
{
    spdlog::set_default_logger(m_logger);
    spdlog::flush_on(spdlog::level::info);
    spdlog::info(LOG_ENTRY);

#ifdef DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif
	m_update_thread = std::thread{ update_thread_func, this };
	m_update_thread.detach();
#ifdef HOOK_D3D
    m_d3d11_hook = std::make_unique<D3D11Hook>();
    m_d3d11_hook->on_present([this](D3D11Hook& hook) { on_frame(); });
    m_d3d11_hook->on_resize_buffers([this](D3D11Hook& hook) { on_reset(); });

    m_valid = m_d3d11_hook->hook();

    if (m_valid) {
        spdlog::info("Hooked D3D11");
    }
#endif
}

ModFramework::~ModFramework() {
	should_quit = true;
}

// this is unfortunate.
void ModFramework::on_direct_input_keys(const std::array<uint8_t, 256>& keys) {
	if (keys[m_menu_key] && m_last_keys[m_menu_key] == 0) {
		std::lock_guard _{ m_input_mutex };
		m_draw_ui = !m_draw_ui;

		// Save the config if we close the UI
		if (!m_draw_ui && m_game_data_initialized) {
			save_config();
		}
	}

	m_last_keys = keys;
}


void ModFramework::on_frame() {
    spdlog::debug("on_frame");

    if (!m_initialized) {
        if (!initialize()) {
            spdlog::error("Failed to initialize ModFramework");
            return;
        }

        spdlog::info("ModFramework initialized");
        m_initialized = true;
        return;
    }

    if (m_error.empty() && m_game_data_initialized) {
        m_mods->on_frame();
    }

}
#if HOOK_D3D
void ModFramework::on_reset() {
    spdlog::info("Reset!");

    // Crashes if we don't release it at this point.
    cleanup_render_target();
    m_initialized = false;
}

bool ModFramework::on_message(HWND wnd, UINT message, WPARAM w_param, LPARAM l_param) {
    if (!m_initialized) {
        return true;
    }

    if (m_draw_ui && ImGui_ImplWin32_WndProcHandler(wnd, message, w_param, l_param) != 0) {
        // If the user is interacting with the UI we block the message from going to the game.
        auto& io = ImGui::GetIO();

        if (io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput) {
            return false;
        }
    }

    return true;
}

// this is unfortunate.
void ModFramework::on_direct_input_keys(const std::array<uint8_t, 256>& keys) {
    if (keys[m_menu_key] && m_last_keys[m_menu_key] == 0) {
        std::lock_guard _{ m_input_mutex };
        m_draw_ui = !m_draw_ui;

        // Save the config if we close the UI
        if (!m_draw_ui && m_game_data_initialized) {
            save_config();
        }
    }

    m_last_keys = keys;
}
#endif
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

	spdlog::info("Attempting to initialize");

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

#if HOOK_D3D
void ModFramework::draw_ui() {
    std::lock_guard _{ m_input_mutex };

    if (!m_draw_ui) {
        m_dinput_hook->acknowledge_input();
        ImGui::GetIO().MouseDrawCursor = false;
        return;
    }

    auto& io = ImGui::GetIO();

    if (io.WantCaptureKeyboard) {
        m_dinput_hook->ignore_input();
    }
    else {
        m_dinput_hook->acknowledge_input();
    }

    ImGui::GetIO().MouseDrawCursor = true;

    //ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_::ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetWorkPos());
    ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_::ImGuiCond_Once);

    ImGui::Begin("ModFramework", &m_draw_ui);

#ifdef GIT_HASH
	ImGui::Text("Version: %s", GIT_HASH);
	ImGui::Text("Date: %s", GIT_DATE);
#endif
    ImGui::Text("Menu Key: Insert");

    draw_about();

    if (m_error.empty() && m_game_data_initialized) {
        m_mods->on_draw_ui();
    }
    else if (!m_game_data_initialized) {
        ImGui::TextWrapped("ModFramework is currently initializing...");
    }
    else if(!m_error.empty()) {
        ImGui::TextWrapped("ModFramework error: %s", m_error.c_str());
    }

    ImGui::End();
}

void ModFramework::draw_about() {
    if (!ImGui::CollapsingHeader("About")) {
        return;
    }

    ImGui::TreePush("About");

    ImGui::Text("Author: praydog");
    ImGui::Text("Inspired by the Kanan project.");
    ImGui::Text("https://github.com/praydog/RE2-Mod-Framework");

    if (ImGui::CollapsingHeader("Licenses")) {
        ImGui::TreePush("Licenses");

        if (ImGui::CollapsingHeader("glm")) {
            ImGui::TextWrapped(license::glm);
        }

        if (ImGui::CollapsingHeader("imgui")) {
            ImGui::TextWrapped(license::imgui);
        }

        if (ImGui::CollapsingHeader("minhook")) {
            ImGui::TextWrapped(license::minhook);
        }

        if (ImGui::CollapsingHeader("spdlog")) {
            ImGui::TextWrapped(license::spdlog);
        }

        ImGui::TreePop();
    }

    ImGui::TreePop();
}

void ModFramework::create_render_target() {
    cleanup_render_target();

    ID3D11Texture2D* back_buffer{ nullptr };
    if (m_d3d11_hook->get_swap_chain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer) == S_OK) {
        m_d3d11_hook->get_device()->CreateRenderTargetView(back_buffer, NULL, &m_main_render_target_view);
        back_buffer->Release();
    }
}

void ModFramework::cleanup_render_target() {
    if (m_main_render_target_view != nullptr) {
        m_main_render_target_view->Release();
        m_main_render_target_view = nullptr;
    }
}
#endif

