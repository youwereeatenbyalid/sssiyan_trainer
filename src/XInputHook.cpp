#if 0
#include <spdlog/spdlog.h>

#include "ModFramework.hpp"
#include "XInputHook.hpp"

static XInputHook* g_xinput_hook{ nullptr };

XInputHook::XInputHook()
	: m_null_state{ 0 },
	m_is_ignoring_input{ false },
	m_do_once{ true }
{
	if (g_xinput_hook == nullptr) {
		if (hook()) {
			spdlog::info("XInputHook hooked successfully.");
			g_xinput_hook = this;
		}
		else {
			spdlog::info("XInputHook failed to hook.");
		}
	}
}

XInputHook::~XInputHook()
{
	g_xinput_hook = nullptr;
}

bool XInputHook::hook()
{
	spdlog::info("Entering XInputHook::hook().");

	auto xinput = LoadLibrary("XINPUT1_3.dll");
	auto xinput_GetState = (decltype(XInputGetState)*)GetProcAddress(xinput, "XInputGetState");

	if (xinput_GetState == nullptr) {
		spdlog::info("Failed to find XInputGetState.");
		return false;
	}

	spdlog::info("Got XInputGetState {:p}", (void*)xinput_GetState);

	// Hook them.
	m_get_state_hook = std::make_unique<FunctionHook>((uintptr_t)xinput_GetState, (uintptr_t)&XInputHook::get_state);

	return m_get_state_hook->create();
}

DWORD XInputHook::get_state_internal(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	auto original_get_state = (decltype(XInputHook::get_state)*)m_get_state_hook->get_original();

	auto res = original_get_state(dwUserIndex, pState);

	if (m_is_ignoring_input) {
		pState->Gamepad = m_null_state;
	}

	return res;
}

DWORD WINAPI XInputHook::get_state(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	return g_xinput_hook->get_state_internal(dwUserIndex, pState);
}

#endif