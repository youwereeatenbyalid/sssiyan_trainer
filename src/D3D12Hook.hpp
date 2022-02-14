#pragma once

#include <iostream>
#include <functional>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi")

#include <d3d12.h>
#include <dxgi1_4.h>

#include "utility/FunctionHook.hpp"

#define D3D12HOOK_INTERNAL(X) D3D12Hook::skip_detours([&]() X)

class D3D12Hook
{
public:
	typedef std::function<void(D3D12Hook&)> OnPresentFn;
	typedef std::function<void(D3D12Hook&)> OnResizeBuffersFn;
    typedef std::function<void(D3D12Hook&)> OnResizeTargetFn;
    typedef std::function<void(D3D12Hook&)> OnCreateSwapChainFn;

	D3D12Hook() = default;
	virtual ~D3D12Hook();

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

	static void signal_resize_target_detour(const bool& state)
    {
	    m_execute_resize_target_detour = state;
    }

	/*static void signal_create_swap_chain_detour(const bool& state)
    {
	    m_execute_create_swap_chain_detour = state;
    }*/

    void on_present(OnPresentFn fn) {
        m_on_present = fn;
    }

    void on_resize_buffers(OnResizeBuffersFn fn) {
        m_on_resize_buffers = fn;
    }

    void on_resize_target(OnResizeTargetFn fn) {
        m_on_resize_target = fn;
    }

    /*void on_create_swap_chain(OnCreateSwapChainFn fn) {
        m_on_create_swap_chain = fn;
    }*/

    ID3D12Device4* get_device() const {
        return m_device;
    }

    IDXGISwapChain3* get_swap_chain() const {
        return m_swap_chain;
    }

    auto get_swapchain_0() { return m_swapchain_0; }
    auto get_swapchain_1() { return m_swapchain_1; }

    ID3D12CommandQueue* get_command_queue() const
	{
        return m_command_queue;
	}

    UINT get_display_width() const {
        return m_display_width;
    }

    UINT get_display_height() const {
        return m_display_height;
    }

    UINT get_render_width() const {
        return m_render_width;
    }

    UINT get_render_height() const {
        return m_render_height;
    }

    bool is_inside_present() const {
        return m_inside_present;
    }

    bool is_proton_swapchain() const {
        return m_is_using_proton_swapchain;
    }

protected:
    ID3D12Device4* m_device{ nullptr };
    IDXGISwapChain3* m_swap_chain{ nullptr };
    IDXGISwapChain3* m_swapchain_0{ nullptr };
    IDXGISwapChain3* m_swapchain_1{ nullptr };
    ID3D12CommandQueue* m_command_queue{ nullptr };
    UINT m_display_width{ 0 };
    UINT m_display_height{ 0 };
    UINT m_render_width{ 0 };
    UINT m_render_height{ 0 };

    std::vector<uint64_t> m_command_queue_offset_list{};
    uint64_t m_command_queue_offset{ 0 };
    uint64_t m_proton_swapchain_offset{ 0 };

    bool m_is_using_proton_swapchain{ false };
    bool m_hooked{ false };
    bool m_inside_present{ false };

    std::unique_ptr<FunctionHook> m_present_hook{};
    std::unique_ptr<FunctionHook> m_resize_buffers_hook{};
    std::unique_ptr<FunctionHook> m_resize_target_hook{};
    //std::unique_ptr<FunctionHook> m_create_swap_chain_hook{};

    OnPresentFn m_on_present{ nullptr };
    OnResizeBuffersFn m_on_resize_buffers{ nullptr };
    OnResizeTargetFn m_on_resize_target{ nullptr };
    //OnCreateSwapChainFn m_on_create_swap_chain{ nullptr };

    static bool m_execute_present_detour;
    static bool m_execute_resize_buffer_detour;
    static bool m_execute_resize_target_detour;
    //static bool m_execute_create_swap_chain_detour;

    static HRESULT WINAPI present(IDXGISwapChain3* swap_chain, UINT sync_interval, UINT flags);
    static HRESULT WINAPI resize_buffers(IDXGISwapChain3* swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags);
    static HRESULT WINAPI resize_target(IDXGISwapChain3* swap_chain, const DXGI_MODE_DESC* new_target_parameters);
    //static HRESULT WINAPI create_swap_chain(IDXGIFactory4* factory, IUnknown* device, HWND hwnd, const DXGI_SWAP_CHAIN_DESC* desc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* p_fullscreen_desc, IDXGIOutput* p_restrict_to_output, IDXGISwapChain** swap_chain);
};

