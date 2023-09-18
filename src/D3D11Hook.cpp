#include <spdlog/spdlog.h>
#include <utility/Module.hpp>
#include <utility/Thread.hpp>

#include "D3D11Hook.hpp"

#include "ModFramework.hpp"

using namespace std;

bool D3D11Hook::s_execute_present_detour{ true };
bool D3D11Hook::s_execute_resize_buffer_detour{ true };

static D3D11Hook* g_d3d11_hook = nullptr;

D3D11Hook::~D3D11Hook() {
    g_d3d11_hook = nullptr;
    unhook();
}

bool D3D11Hook::hook() {
    spdlog::info("Hooking D3D11");

    g_d3d11_hook = this;

    HWND h_wnd = GetDesktopWindow();
    IDXGISwapChain* swap_chain = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;

    D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
    DXGI_SWAP_CHAIN_DESC swap_chain_desc;

    ZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));

    swap_chain_desc.BufferCount = 1;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.OutputWindow = h_wnd;
    swap_chain_desc.SampleDesc.Count = 1;
    swap_chain_desc.Windowed = TRUE;
    swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    utility::ThreadSuspender suspender{};

	const auto original_bytes = utility::get_original_bytes(&D3D11CreateDeviceAndSwapChain);

	// Temporarily unhook D3D11CreateDeviceAndSwapChain
	// it allows compatibility with ReShade and other overlays that hook it
	// this is just a dummy device anyways, we don't want the other overlays to be able to use it
	if (original_bytes) {
		spdlog::info("[D3D11] D3D11CreateDeviceAndSwapChain appears to be hooked, temporarily unhooking");

		std::vector<uint8_t> hooked_bytes(original_bytes->size());
		memcpy(hooked_bytes.data(), &D3D11CreateDeviceAndSwapChain, original_bytes->size());

		ProtectionOverride protection_override{ &D3D11CreateDeviceAndSwapChain, original_bytes->size(), PAGE_EXECUTE_READWRITE };
		memcpy(&D3D11CreateDeviceAndSwapChain, original_bytes->data(), original_bytes->size());

		if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_NULL, nullptr, 0, &feature_level, 1, D3D11_SDK_VERSION,
			&swap_chain_desc, &swap_chain, &device, nullptr, &context)))
		{
			spdlog::error("[D3D11] Failed to create D3D11 device");
			memcpy(&D3D11CreateDeviceAndSwapChain, hooked_bytes.data(), hooked_bytes.size());
			return false;
		}

		spdlog::info("[D3D11] Restoring hooked bytes for D3D11CreateDeviceAndSwapChain");
		memcpy(&D3D11CreateDeviceAndSwapChain, hooked_bytes.data(), hooked_bytes.size());
	}
	else {
		if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_NULL, nullptr, 0, &feature_level, 1, D3D11_SDK_VERSION,
			&swap_chain_desc, &swap_chain, &device, nullptr, &context)))
		{
			spdlog::error("[D3D11] Failed to create D3D11 device");
			return false;
		}
	}

    try {
		m_present_hook.reset();
        m_resize_buffers_hook.reset();

        auto& present_fn = (*(void***)swap_chain)[8];
        auto& resize_buffers_fn = (*(void***)swap_chain)[13];

        m_present_hook = std::make_unique<PointerHook>(&present_fn, (void*)&D3D11Hook::present);
        m_resize_buffers_hook = std::make_unique<PointerHook>(&resize_buffers_fn, (void*)&D3D11Hook::resize_buffers);
		
        //m_hooked = m_present_hook->enable() && m_resize_buffers_hook->enable();
        m_hooked = true;
    }
	catch (const std::exception& e) {
		spdlog::error("[D3D11] Failed to hook D3D11: {}", e.what());
		m_hooked = false;
	}

    suspender.resume();

    device->Release();
    context->Release();
    swap_chain->Release();

    return m_hooked;
}

bool D3D11Hook::unhook() {
	if (!m_hooked) {
		return true;
	}

    utility::ThreadSuspender suspender{};

    try {
        if (m_present_hook->remove() && m_resize_buffers_hook->remove()) {
            m_hooked = false;
            return true;
        }
    }
	catch (const std::exception& e) {
		spdlog::error("[D3D11] Failed to unhook D3D11: {}", e.what());
    }

    return false;
}

void D3D11Hook::skip_detours(const std::function<void()>& code)
{
    const bool back_present         = s_execute_present_detour;
    const bool back_resize_buffer   = s_execute_resize_buffer_detour;

    s_execute_present_detour        = false;
    s_execute_resize_buffer_detour  = false;

    code();

    s_execute_present_detour        = back_present;
    s_execute_resize_buffer_detour  = back_resize_buffer;
}

thread_local size_t g_d3d11_internal_inside_present = 0;
HRESULT last_d3d11_present_result = S_OK;

HRESULT WINAPI D3D11Hook::present(IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags) {
    // Some D3D11 games test presentation for timing and composition purposes
	// These calls are not rendering related, but rather a status request for the D3D runtime and as such should be ignored
	if ((flags & DXGI_PRESENT_TEST) != 0)
		return last_d3d11_present_result;

    // Also skip if the main body of the application is not initialized yet
    if (!g_framework)
        return last_d3d11_present_result;

    std::scoped_lock _{ g_framework->get_hook_monitor_mutex() };

    auto d3d11 = g_d3d11_hook;

    // This line must be called before calling our detour function because we might have to unhook the function inside our detour.
    auto present_fn = d3d11->m_present_hook->get_original<decltype(D3D11Hook::present)*>();

    if (g_d3d11_internal_inside_present > 0) {
		auto original_bytes = utility::get_original_bytes(Address{ present_fn });

		if (original_bytes) {
			ProtectionOverride protection_override{ present_fn, original_bytes->size(), PAGE_EXECUTE_READWRITE };

			memcpy(present_fn, original_bytes->data(), original_bytes->size());

			spdlog::info("[D3D11] Present recursion detected, present restored to default!");
		}

		return last_d3d11_present_result;
	}

    if (s_execute_present_detour /*&& g_d3d11_internal_inside_present == 0*/) {
        D3D11HOOK_INTERNAL({
            d3d11->m_swap_chain = swap_chain;
            swap_chain->GetDevice(__uuidof(d3d11->m_device), (void**)&d3d11->m_device);

            if (d3d11->m_on_present) {
                d3d11->m_on_present(*d3d11);
            }
        });
    }

    g_d3d11_internal_inside_present++;

    last_d3d11_present_result = present_fn(swap_chain, sync_interval, flags);

    g_d3d11_internal_inside_present--;

    return last_d3d11_present_result;
}

thread_local size_t g_d3d11_internal_inside_resize_buffers = 0;
HRESULT last_d3d11_resize_buffers_result = S_OK;

HRESULT WINAPI D3D11Hook::resize_buffers(IDXGISwapChain* swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags) {
    std::scoped_lock _{ g_framework->get_hook_monitor_mutex() };
    
    auto d3d11 = g_d3d11_hook;

    auto resize_buffers_fn = d3d11->m_resize_buffers_hook->get_original<decltype(D3D11Hook::resize_buffers)*>();

	if (g_d3d11_internal_inside_resize_buffers > 0) {
		auto original_bytes = utility::get_original_bytes(Address{ resize_buffers_fn });

		if (original_bytes) {
			ProtectionOverride protection_override{ resize_buffers_fn, original_bytes->size(), PAGE_EXECUTE_READWRITE };

			memcpy(resize_buffers_fn, original_bytes->data(), original_bytes->size());

			spdlog::info("[D3D11] Resize buffers recursion detected, resize buffers restored to default!");
		}

		return last_d3d11_resize_buffers_result;
	}

    if (s_execute_resize_buffer_detour /*&& g_d3d11_internal_inside_resize_buffers == 0*/) {
        D3D11HOOK_INTERNAL({
            if (d3d11->m_on_resize_buffers) {
                d3d11->m_on_resize_buffers(*d3d11, width, height);
            }
        });
    }

    g_d3d11_internal_inside_resize_buffers++;

    last_d3d11_resize_buffers_result = resize_buffers_fn(swap_chain, buffer_count, width, height, new_format, swap_chain_flags);

    g_d3d11_internal_inside_resize_buffers--;

    return last_d3d11_resize_buffers_result;
}
