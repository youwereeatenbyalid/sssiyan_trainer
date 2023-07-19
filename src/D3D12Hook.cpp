#include <spdlog/spdlog.h>

#include "D3D12Hook.hpp"

#include "ModFramework.hpp"

bool D3D12Hook::m_execute_present_detour{ true };
bool D3D12Hook::m_execute_resize_buffer_detour{ true };
bool D3D12Hook::m_execute_resize_target_detour{ true };
//bool D3D12Hook::m_execute_create_swap_chain_detour{ true };

static D3D12Hook* g_d3d12_hook = nullptr;

D3D12Hook::~D3D12Hook() {
    unhook();
}

bool D3D12Hook::hook() {
    spdlog::info("Hooking D3D12");

    g_d3d12_hook = this;

    IDXGISwapChain1* swap_chain1{ nullptr };
    IDXGISwapChain3* swap_chain{ nullptr };
    ID3D12Device* device{ nullptr };

    D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc1;

    ZeroMemory(&swap_chain_desc1, sizeof(swap_chain_desc1));

    swap_chain_desc1.Width = 1;
    swap_chain_desc1.Height = 1;
    swap_chain_desc1.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swap_chain_desc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swap_chain_desc1.BufferCount = 2;
    swap_chain_desc1.SampleDesc.Count = 1;
    swap_chain_desc1.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

    // Manually get D3D12CreateDevice export because the user may be running Windows 7
    const auto d3d12_module = LoadLibraryA("d3d12.dll");
    if (d3d12_module == nullptr) {
        spdlog::error("Failed to load d3d12.dll");
        return false;
    }

    auto d3d12_create_device = (decltype(D3D12CreateDevice)*)GetProcAddress(d3d12_module, "D3D12CreateDevice");
    if (d3d12_create_device == nullptr) {
        spdlog::error("Failed to get D3D12CreateDevice export");
        return false;
    }

    spdlog::info("Creating dummy device");

    if (FAILED(d3d12_create_device(nullptr, feature_level, IID_PPV_ARGS(&device)))) {
        spdlog::error("Failed to create D3D12 Dummy device\n");
        return false;
    }

    // Manually get CreateDXGIFactory export because the user may be running Windows 7
    const auto dxgi_module = LoadLibraryA("dxgi.dll");
    if (dxgi_module == nullptr) {
        spdlog::error("Failed to load dxgi.dll");
        return false;
    }

    auto create_dxgi_factory = (decltype(CreateDXGIFactory)*)GetProcAddress(dxgi_module, "CreateDXGIFactory");

    if (create_dxgi_factory == nullptr) {
        spdlog::error("Failed to get CreateDXGIFactory export");
        return false;
    }

    spdlog::info("Creating dummy DXGI factory");

    IDXGIFactory4* factory{ nullptr };
    if (FAILED(create_dxgi_factory(IID_PPV_ARGS(&factory)))) {
        spdlog::error("Failed to create D3D12 Dummy DXGI Factory");
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC queue_desc;
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queue_desc.Priority = 0;
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.NodeMask = 0;

    spdlog::info("Creating dummy command queue");

    ID3D12CommandQueue* command_queue{ nullptr };
    if (FAILED(device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue)))) {
        spdlog::error("Failed to create D3D12 Dummy Command Queue\n");
        return false;
    }

    spdlog::info("Creating dummy swapchain");

    if (FAILED(factory->CreateSwapChainForComposition(command_queue, &swap_chain_desc1, NULL, &swap_chain1))) {
        spdlog::error("Failed to create D3D12 Dummy DXGI SwapChain");
        return false;
    }

    spdlog::info("Querying dummy swapchain");

    if (FAILED(swap_chain1->QueryInterface(IID_PPV_ARGS(&swap_chain)))) {
        spdlog::error("Failed to retrieve D3D12 DXGI SwapChain\n");
        return false;
    }

    spdlog::info("Finding command queue offset");
    
    // Find the command queue offset in the swapchain
    //m_command_queue_offset = (uintptr_t)std::find((uintptr_t*)swap_chain, (uintptr_t*)((uintptr_t)swap_chain + 512), (uintptr_t)command_queue) - (uintptr_t)swap_chain; //1 byte aigned search

    m_command_queue_offset_list.reserve(5);

    for (size_t i = 0; i < 512 * sizeof(void*); i += sizeof(void*)) { //8 byte aligned search
        const auto base = (uintptr_t)swap_chain + i;

        // reached the end
        if (IsBadReadPtr((void*)base, sizeof(void*))) {
            break;
        }

        auto data = *(ID3D12CommandQueue**)base;

        if (data == command_queue) {
            m_command_queue_offset_list.push_back(i);
            spdlog::info("Found command queue offset: {:x}", i);
        }
    }

    // Scan throughout the swapchain for a valid pointer to scan through
    // this is usually only necessary for Proton
    if (m_command_queue_offset_list.empty()) {
        for (uintptr_t base = 0; base < 512 * sizeof(void*); base += sizeof(void*)) { //8 byte aligned search
            const auto pre_scan_base = (uintptr_t)swap_chain + base;

            // reached the end
            if (IsBadReadPtr((void*)pre_scan_base, sizeof(void*))) {
                break;
            }

            const auto scan_base = *(uintptr_t*)pre_scan_base;

            if (scan_base == 0 || IsBadReadPtr((void*)scan_base, sizeof(void*))) {
                continue;
            }

            for (size_t i = 0; i < 512 * sizeof(void*); i += sizeof(void*)) {
                const auto pre_data = scan_base + i;

                if (IsBadReadPtr((void*)pre_data, sizeof(void*))) {
                    break;
                }

                auto data = *(ID3D12CommandQueue**)pre_data;

                if (data == command_queue) {
                    m_is_using_proton_swapchain = true;
                    m_command_queue_offset_list.push_back(i);
                    m_proton_swapchain_offset = base;

                    spdlog::info("Proton potentially detected");
                    spdlog::info("Found command queue offset: {:x}", i);
                }
            }

            if (m_is_using_proton_swapchain) {
                break;
            }
        }
    }

    auto present_fn = (*(uintptr_t**)swap_chain)[8]; // Present
    auto resize_buffers_fn = (*(uintptr_t**)swap_chain)[13]; // ResizeBuffers
    auto resize_target_fn = (*(uintptr_t**)swap_chain)[14]; // ResizeTarget
    //auto create_swap_chain_fn = (*(uintptr_t**)factory)[15]; // CreateSwapChainForHwnd

    m_present_hook = std::make_unique<FunctionHook>(present_fn, (uintptr_t)&D3D12Hook::present);
    m_resize_buffers_hook = std::make_unique<FunctionHook>(resize_buffers_fn, (uintptr_t)&D3D12Hook::resize_buffers);
    m_resize_target_hook = std::make_unique<FunctionHook>(resize_target_fn, (uintptr_t)&D3D12Hook::resize_target);
    //m_create_swap_chain_hook = std::make_unique<FunctionHook>(create_swap_chain_fn, (uintptr_t)&D3D12Hook::create_swap_chain);

    device->Release();
    command_queue->Release();
    factory->Release();
    swap_chain1->Release();
    swap_chain->Release();

    m_hooked = m_present_hook->create() && m_resize_buffers_hook->create() && m_resize_target_hook->create();//&& m_create_swap_chain_hook->create()'

    return m_hooked;
}

bool D3D12Hook::unhook() {
    if (m_present_hook == nullptr) {
        spdlog::error("m_present_hook is null");
        return false;
    }
        
    if (m_resize_buffers_hook == nullptr){
        spdlog::error("m_resize_buffers_hook is null");
        return false;
    }
        
    if (m_resize_target_hook == nullptr){
        spdlog::error("m_resize_target_hook is null");
        return false;
    }
        
    try{
        if (m_present_hook->remove() && m_resize_buffers_hook->remove() && m_resize_target_hook->remove() /*&& m_create_swap_chain_hook->remove()*/) {
            m_hooked = false;
            return true;
        }
    }
    catch(...) {
        spdlog::error("Something went horribly wrong unhooking D3D12");
    };
    return false;
}

void D3D12Hook::skip_detours(const std::function<void()>& code)
{
    const bool back_present             = m_execute_present_detour;
    const bool back_resize_buffer       = m_execute_resize_buffer_detour;
    const bool back_resize_target       = m_execute_resize_target_detour;
	//const bool back_create_swap_chain   = m_execute_create_swap_chain_detour;

    m_execute_present_detour            = false;
    m_execute_resize_buffer_detour      = false;
    m_execute_resize_target_detour      = false;
    //m_execute_create_swap_chain_detour  = false;

    code();

    m_execute_present_detour            = back_present;
    m_execute_resize_buffer_detour      = back_resize_buffer;
    m_execute_resize_target_detour      = back_resize_target;
    //m_execute_create_swap_chain_detour  = back_create_swap_chain;
}

thread_local size_t g_d3d12_internal_inside_present = 0;
HRESULT last_d3d12_present_result = S_OK;

HRESULT WINAPI D3D12Hook::present(IDXGISwapChain3* swap_chain, UINT sync_interval, UINT flags) {
    std::scoped_lock _{ g_framework->get_hook_monitor_mutex() };

    auto d3d12 = g_d3d12_hook;

    // This line must be called before calling our detour function because we might have to unhook the function inside our detour.
    auto present_fn = d3d12->m_present_hook->get_original<decltype(present)>();

    if (m_execute_present_detour && g_d3d12_internal_inside_present == 0) {
        D3D12HOOK_INTERNAL({
            d3d12->m_swap_chain = swap_chain;
            swap_chain->GetDevice(IID_PPV_ARGS(&d3d12->m_device));


            if (d3d12->m_device != nullptr) {
                if (d3d12->m_is_using_proton_swapchain) {
                    const auto real_swapchain = *(uintptr_t*)((uintptr_t)swap_chain + d3d12->m_proton_swapchain_offset);

                    if (d3d12->m_command_queue == nullptr) {
                        for (const auto& cmd_queue_offset : d3d12->m_command_queue_offset_list) {
                            if (const auto cmd_queue_tmp = *(ID3D12CommandQueue**)(real_swapchain + cmd_queue_offset); !IsBadReadPtr(cmd_queue_tmp, sizeof(cmd_queue_tmp)))
                            {
                                d3d12->m_command_queue_offset = cmd_queue_offset;
                                spdlog::info("Using Command Queue offset: {:x}", d3d12->m_command_queue_offset);

                                break;
                            }
                        }
                    }

                    d3d12->m_command_queue = *(ID3D12CommandQueue**)(real_swapchain + d3d12->m_command_queue_offset);
                }
                else {
                    if (d3d12->m_command_queue == nullptr) {
                        for (const auto& cmd_queue_offset : d3d12->m_command_queue_offset_list) {
                            if (const auto cmd_queue_tmp = *(ID3D12CommandQueue**)((uintptr_t)swap_chain + cmd_queue_offset); !IsBadReadPtr(cmd_queue_tmp, sizeof(cmd_queue_tmp)))
                            {
                                d3d12->m_command_queue_offset = cmd_queue_offset;
                                spdlog::info("Using Command Queue offset: {:x}", d3d12->m_command_queue_offset);

                                break;
                            }
                        }
                    }

                    d3d12->m_command_queue = *(ID3D12CommandQueue**)((uintptr_t)swap_chain + d3d12->m_command_queue_offset);
                }
            }

            if (d3d12->m_swapchain_0 == nullptr) {
                d3d12->m_swapchain_0 = swap_chain;
            }
            else if (d3d12->m_swapchain_1 == nullptr && swap_chain != d3d12->m_swapchain_0) {
                d3d12->m_swapchain_1 = swap_chain;
            }

            if (d3d12->m_on_present) {
                d3d12->m_on_present(*d3d12);
            }
        });
    }

    g_d3d12_internal_inside_present++;

    last_d3d12_present_result = present_fn(swap_chain, sync_interval, flags);

    g_d3d12_internal_inside_present--;

    return last_d3d12_present_result;
}

thread_local size_t g_d3d12_internal_inside_resize_buffers = 0;
HRESULT last_d3d12_resize_buffers_result = S_OK;

HRESULT WINAPI D3D12Hook::resize_buffers(IDXGISwapChain3* swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags) {
    std::scoped_lock _{ g_framework->get_hook_monitor_mutex() };

	auto d3d12 = g_d3d12_hook;

    auto resize_buffers_fn = d3d12->m_resize_buffers_hook->get_original<decltype(resize_buffers)>();

    if (m_execute_resize_buffer_detour && g_d3d12_internal_inside_resize_buffers == 0) {
    	D3D12HOOK_INTERNAL({
            d3d12->m_display_width = width;
            d3d12->m_display_height = height;

            if (d3d12->m_on_resize_buffers) {
                d3d12->m_on_resize_buffers(*d3d12, width, height);
            }
    	});
    }

    g_d3d12_internal_inside_resize_buffers++;

    last_d3d12_resize_buffers_result = resize_buffers_fn(swap_chain, buffer_count, width, height, new_format, swap_chain_flags);

    g_d3d12_internal_inside_resize_buffers--;

    return last_d3d12_resize_buffers_result;
}

thread_local size_t g_d3d12_internal_inside_resize_target = 0;
HRESULT last_d3d12_resize_target_result = S_OK;

HRESULT WINAPI D3D12Hook::resize_target(IDXGISwapChain3* swap_chain, const DXGI_MODE_DESC* new_target_parameters)
{
    std::scoped_lock _{ g_framework->get_hook_monitor_mutex() };

    auto d3d12 = g_d3d12_hook;

    auto resize_buffers_fn = d3d12->m_resize_target_hook->get_original<decltype(D3D12Hook::resize_target)>();

    if (m_execute_resize_target_detour && g_d3d12_internal_inside_resize_target == 0) {
        D3D12HOOK_INTERNAL({
            d3d12->m_render_width = new_target_parameters->Width;
            d3d12->m_render_height = new_target_parameters->Height;

            if (d3d12->m_on_resize_target) {
                d3d12->m_on_resize_target(*d3d12);
            }
        });
    }

    g_d3d12_internal_inside_resize_target++;

    last_d3d12_resize_target_result = resize_buffers_fn(swap_chain, new_target_parameters);

    g_d3d12_internal_inside_resize_target--;

    return last_d3d12_resize_target_result;
}

/*HRESULT WINAPI D3D12Hook::create_swap_chain(IDXGIFactory4* factory, IUnknown* device, HWND hwnd, const DXGI_SWAP_CHAIN_DESC* desc, const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* p_fullscreen_desc, IDXGIOutput* p_restrict_to_output, IDXGISwapChain** swap_chain)
{
    auto d3d12 = g_d3d12_hook;

    auto create_swap_chain_fn = d3d12->m_create_swap_chain_hook->get_original<decltype(D3D12Hook::create_swap_chain)>();

    if (m_execute_create_swap_chain_detour) {
        D3D12HOOK_INTERNAL({
            d3d12->m_command_queue = (ID3D12CommandQueue*)device;

            if (d3d12->m_on_create_swap_chain) {
                d3d12->m_on_create_swap_chain(*d3d12);
            }
        });
    }

    return create_swap_chain_fn(factory, device, hwnd, desc, p_fullscreen_desc, p_restrict_to_output, swap_chain);
}*/

