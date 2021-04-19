#pragma once

#include <spdlog/spdlog.h>

class Mods;

#include "D3D11Hook.hpp"
#include "WindowsMessageHook.hpp"
#include "DInputHook.hpp"

// Global facilitator
class ModFramework {
public:
	ModFramework();
	virtual ~ModFramework();

	bool is_valid() const {
		return m_valid;
	}

	const auto& get_mods() const {
		return m_mods;
	}
	
	const auto& get_error() const {
		return m_error;
	}
    /*const auto& get_types() const {
        return m_types;
    }*/

	const auto& get_keyboard_state() const {
		return m_last_keys;
	}

    /*const auto& get_globals() const {
        return m_globals;
    }*/

	Address get_module() const {
		return m_game_module;
	}

	bool is_ready() const {
		return m_game_data_initialized;
	}

	bool is_error() const {
		return m_error.empty();
	}

	void on_frame();
	void on_reset();
	bool on_message(HWND wnd, UINT message, WPARAM w_param, LPARAM l_param);
	void on_direct_input_keys(const std::array<uint8_t, 256>& keys);

	void save_config();
	
	bool m_draw_ui{ false };

private:
	void draw_ui();
	void draw_about();

	bool initialize();
	void create_render_target();
	void cleanup_render_target();

	bool m_first_frame{ true };
	bool m_valid{ false };
	bool m_initialized{ false };

	std::atomic<bool> m_game_data_initialized{ false };

	std::mutex m_input_mutex{};

	HWND m_wnd{ 0 };
	HMODULE m_game_module{ 0 };
	uint8_t m_menu_key{ DIK_DELETE };

	std::array<uint8_t, 256> m_last_keys{ 0 };
	std::unique_ptr<D3D11Hook> m_d3d11_hook{};
	std::unique_ptr<WindowsMessageHook> m_windows_message_hook;
	std::unique_ptr<DInputHook> m_dinput_hook;
    std::shared_ptr<spdlog::logger> m_logger;

	std::thread m_update_thread{};

    std::string m_error{ "" };

    // Game-specific stuff
    std::unique_ptr<Mods> m_mods;

    ID3D11RenderTargetView* m_main_render_target_view{ nullptr };
};

extern std::unique_ptr<ModFramework> g_framework;
