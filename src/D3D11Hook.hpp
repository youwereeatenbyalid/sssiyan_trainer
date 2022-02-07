#pragma once

#include <functional>

#include <d3d11.h>
#include <dxgi.h>

#include "utility/FunctionHook.hpp"

#define D3D11HOOK_INTERNAL(X) D3D11Hook::skip_detours([&]() X)

class D3D11Hook {
public:
    typedef std::function<void(D3D11Hook&)> OnPresentFn;
    typedef std::function<void(D3D11Hook&)> OnResizeBuffersFn;

    D3D11Hook() = default;
    virtual ~D3D11Hook();

    bool hook();
    bool unhook();

    bool is_hooked() const
    {
        return m_hooked;
    }

    static void skip_detours(const std::function<void()>& code);

    static void signal_present_detour(const bool& state)
    {
        m_execute_present_detour = state;
    }

    static void signal_resize_buffer_detour(const bool& state)
    {
        m_execute_resize_buffer_detour = state;
    }

    void on_present(OnPresentFn fn) { m_on_present = fn; }
    void on_resize_buffers(OnResizeBuffersFn fn) { m_on_resize_buffers = fn; }

    ID3D11Device* get_device() { return m_device; }
    IDXGISwapChain* get_swap_chain() { return m_swap_chain; }

protected:
    ID3D11Device* m_device{ nullptr };
    IDXGISwapChain* m_swap_chain{ nullptr };
    bool m_hooked{ false };

    std::unique_ptr<FunctionHook> m_present_hook{};
    std::unique_ptr<FunctionHook> m_resize_buffers_hook{};
    OnPresentFn m_on_present{ nullptr };
    OnResizeBuffersFn m_on_resize_buffers{ nullptr };

    static bool m_execute_present_detour;
    static bool m_execute_resize_buffer_detour;

    static HRESULT WINAPI present(IDXGISwapChain* swap_chain, UINT sync_interval, UINT flags);
    static HRESULT WINAPI resize_buffers(IDXGISwapChain* swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags);
};
