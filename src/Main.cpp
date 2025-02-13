#include <chrono>
#include <mutex>
#include <thread>
#include <windows.h>

#include "ModFramework.hpp"

HMODULE g_dinput = 0;
std::mutex g_load_mutex{};

void failed() {
    MessageBox(0, "ModFramework: Unable to load the original dinput8.dll. Please report this to the developer.", "ModFramework", 0);
    ExitProcess(0);
}

bool load_dinput8() {
    std::scoped_lock _{ g_load_mutex };

    if (g_dinput) {
        return true;
    }

    wchar_t buffer[MAX_PATH]{ 0 };
    if (GetSystemDirectoryW(buffer, MAX_PATH) != 0) {
        // Load the original dinput8.dll
        if ((g_dinput = LoadLibraryW((std::wstring{ buffer } + L"\\dinput8.dll").c_str())) == NULL) {
            failed();
            return false;
        }
        return true;
    }

    failed();
    return false;
}

extern "C" {
    // DirectInput8Create wrapper for dinput8.dll
    __declspec(dllexport) HRESULT WINAPI
        direct_input8_create(HINSTANCE hinst, DWORD dw_version, const IID& riidltf, LPVOID* ppv_out, LPUNKNOWN punk_outer) {
        // This needs to be done because when we include dinput.h in DInputHook,
        // It is a redefinition, so we assign an export by not using the original name
#pragma comment(linker, "/EXPORT:DirectInput8Create=direct_input8_create")

        load_dinput8();
        return ((decltype(DirectInput8Create)*)GetProcAddress(g_dinput, "DirectInput8Create"))(hinst, dw_version, riidltf, ppv_out, punk_outer);
    }
}

void startup_thread() {
#ifndef NDEBUG
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif

    if (load_dinput8()) {
        g_framework = std::make_unique<ModFramework>();
    }
}

BOOL APIENTRY DllMain(HANDLE handle, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
#ifndef NDEBUG
        MessageBox(NULL,
            "Now's a good time to attach a debugger if you want.",
            "Attach", MB_ICONINFORMATION);
#endif
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)startup_thread, handle, 0, nullptr);
    }

    return TRUE;
}