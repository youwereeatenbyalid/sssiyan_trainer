#include <spdlog/spdlog.h>
#include <MinHook.h>

#include "FunctionHook.hpp"

using namespace std;

bool g_isMinHookInitialized{ false };

FunctionHook::FunctionHook(Address target, Address destination, bool useLog)
    : m_target{ 0 },
    m_destination{ 0 },
    m_original{ 0 },
    _useLog(useLog)
{
    std::scoped_lock _{ m_hook_mutex };

    if(!s_allow_hook)
    {
        return;
    }

    // Call back before the initialiazation begins
    if (s_hook_init_begin_call_back != nullptr)
    {
        s_hook_init_begin_call_back(::GetCurrentThread());
    }
    if(_useLog)
        spdlog::info("Attempting to hook {:p}->{:p}", target.ptr(), destination.ptr());

    // Initialize MinHook if it hasn't been already.
    if (!g_isMinHookInitialized && MH_Initialize() == MH_OK) {
        g_isMinHookInitialized = true;
    }

    // Create the hook. Call create afterwards to prevent race conditions accessing FunctionHook before it leaves its constructor.
    if (const auto status = MH_CreateHook(target.as<LPVOID>(), destination.as<LPVOID>(), reinterpret_cast<LPVOID*>(&m_original)); status == MH_OK) {
        m_target = target;
        m_destination = destination;

        if (_useLog)
            spdlog::info("Hook init successful {:p}->{:p}", target.ptr(), destination.ptr());
    }
    else if (_useLog) {
        spdlog::error("Failed to hook {:p}: {}", target.ptr(), MH_StatusToString(status));
    }

    // Call back after the initialiazation is finished
    if(s_hook_init_end_call_back != nullptr)
    {
        s_hook_init_end_call_back(::GetCurrentThread());
    }
}

FunctionHook::~FunctionHook() {
    remove();
}

bool FunctionHook::create() {
    if (m_target == 0 || m_destination == 0 || m_original == 0) {
        if (_useLog)
            spdlog::error("FunctionHook not initialized");
        return false;
    }

    if (auto status = MH_EnableHook(reinterpret_cast<LPVOID>(m_target)); status != MH_OK) {
        m_original = 0;
        m_destination = 0;
        m_target = 0;

        if (_useLog)
            spdlog::error("Failed to hook {:x}: {}", m_target, MH_StatusToString(status));
        return false;
    }

    if (_useLog)
        spdlog::info("Hooked {:x}->{:x}", m_target, m_destination);
    return true;
}

bool FunctionHook::remove() {
    // Don't try to remove invalid hooks.
    if (m_original == 0) {
        return true;
    }

    // Disable then remove the hook.
    if (MH_DisableHook(reinterpret_cast<LPVOID>(m_target)) != MH_OK ||
        MH_RemoveHook(reinterpret_cast<LPVOID>(m_target)) != MH_OK) {
        return false;
    }

    // Invalidate the members.
    m_target = 0;
    m_destination = 0;
    m_original = 0;

    return true;
}