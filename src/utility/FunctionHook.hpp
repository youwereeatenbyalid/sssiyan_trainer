#pragma once

#include <mutex>
#include <windows.h>
#include <cstdint>

#include <utility/Address.hpp>

class FunctionHook {
public:
    FunctionHook() = delete;
    FunctionHook(const FunctionHook& other) = delete;
    FunctionHook(FunctionHook&& other) = delete;
    FunctionHook(Address target, Address destination);
    virtual ~FunctionHook();

    bool create();

    // Called automatically by the destructor, but you can call it explicitly
    // if you need to remove the hook.
    bool remove();

    auto get_original() const {
        return m_original;
    }

    template <typename T>
    T* get_original() const {
        return reinterpret_cast<T*>(m_original);
    }

    auto is_valid() const {
        return m_original != 0;
    }

    FunctionHook& operator=(const FunctionHook& other) = delete;
    FunctionHook& operator=(FunctionHook&& other) = delete;

    // Thread synchronization

    // Set the value to either allow adding hooks (true) or don't let any more hooks to get created (false)
    static void allow_hook(const bool& allow) {
        s_allow_hook = allow;
    }

    // A callback that gets called after each hook initialization, nullptr if there is none 
    static auto& get_hook_end_callback() {
        return s_hook_init_end_call_back;
    }

    // A callback that gets called before each hook initialization, nullptr if there is none 
    static auto& get_hook_begin_callback() {
        return s_hook_init_begin_call_back;
    }

private:
    uintptr_t m_target{ 0 };
    uintptr_t m_destination{ 0 };
    uintptr_t m_original{ 0 };

    // Thread synchronization

    std::recursive_mutex m_hook_mutex;

    inline static bool s_allow_hook = true;
    inline static void (*s_hook_init_end_call_back)(HANDLE current_thread) = nullptr;
    inline static void (*s_hook_init_begin_call_back)(HANDLE current_thread) = nullptr;
};