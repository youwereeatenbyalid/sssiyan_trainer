#pragma once

#include <sdk/ReClass.hpp>
#include "utility/FunctionHook.hpp"

class ControllerHook
{
public:
	ControllerHook();
	~ControllerHook();

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
		return m_hooked;
	}

	ControllerHook& operator=(const ControllerHook& other) = delete;
	ControllerHook& operator=(ControllerHook&& other) = delete;

private:
	bool hook();

	static bool m_hooked;

	std::unique_ptr<FunctionHook> m_controllerUpdateHook{};

	static bool m_is_ignoring_input;

	static void Sub14288CD10(uintptr_t uknPtr, HIDGamePadDevice*& gamePadDeviceP, uint64_t uknArg, uint64_t uknIndex);
};

