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

// clang-format off

std::unique_ptr<ModFramework> g_framework{};

bool draw_imwindow = false;
bool should_quit = false;

void update_thread_func(ModFramework* mf) {
	spdlog::info("update thread entry");
	auto& io = ImGui::GetIO(); (void)io;
	io.FontDefault = io.Fonts->AddFontFromFileTTF("Collab Trainer/Fonts/Open_Sans/OpenSans-Regular.ttf", 18.0f);
	while (!should_quit) {
		if (mf->m_draw_ui) {
			mf->m_draw_ui = false;

			PreInitSample();

			ImWindow::ImwWindowManagerDX11 o_mgr(true);

			o_mgr.Init();

			InitSample(mf);
	
			while (o_mgr.Run(false) && o_mgr.Run(true)) {
				auto mpw = o_mgr.GetMainPlatformWindow();
				mpw->Show(true);
				mf->on_frame();
				ImGuiIO& io = mpw->GetContext()->IO;
				HWND hwNd = (HWND)io.ImeWindowHandle;
				HWND SetActiveWindow(hwNd);
				//if (mf->m_draw_ui){
				//	o_mgr.Destroy();
				//}
				std::this_thread::sleep_for(std::chrono::milliseconds(17));
			}
			mf->m_draw_ui = false;
			ImGui::Shutdown();
		}
		mf->on_frame();
		std::this_thread::sleep_for(std::chrono::milliseconds(17));
	}
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
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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
		
		m_wnd = FindWindowA(NULL, "Devil May Cry 5");
		m_dinput_hook = std::make_unique<DInputHook>(m_wnd);

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

			FunctionHook::enable_queued();

			m_game_data_initialized = true;
		});

		init_thread.detach();
	}

	return true;
}
