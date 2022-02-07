#if 0
#pragma once
#include <memory>
#include <vector>

#ifndef _XBOX_CONTROLLER_H_
#define _XBOX_CONTROLLER_H_
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <XInput.h>

#include "utility/FunctionHook.hpp"

class XInputHook {
public:
	XInputHook();
	XInputHook(const XInputHook& other) = delete;
	XInputHook(XInputHook&& other) = delete;
	virtual ~XInputHook();

	void ignore_input() {
		m_is_ignoring_input = true;
	}

	void acknowledge_input() {
		m_is_ignoring_input = false;
	}

	auto is_ignoring_input() const {
		return m_is_ignoring_input;
	}

	auto is_valid() const {
		return m_get_state_hook->is_valid();
	}

	XInputHook& operator=(const XInputHook& other) = delete;
	XInputHook& operator=(XInputHook&& other) = delete;

private:
	XINPUT_GAMEPAD m_null_state;
	
	std::unique_ptr<FunctionHook> m_get_state_hook;

	bool m_is_ignoring_input;
	bool m_do_once;

	bool hook();

	DWORD get_state_internal(DWORD dwUserIndex, XINPUT_STATE* pState);
	static DWORD WINAPI get_state(DWORD dwUserIndex, XINPUT_STATE* pState);
};

#endif