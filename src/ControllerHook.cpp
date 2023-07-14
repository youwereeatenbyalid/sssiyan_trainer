#include <spdlog/spdlog.h>

#include "ModFramework.hpp"
#include "ControllerHook.hpp"

static ControllerHook* g_controllerHook{ nullptr };

bool ControllerHook::m_hooked{ false };
bool ControllerHook::m_is_ignoring_input{ false };

ControllerHook::ControllerHook()
{
	if (g_controllerHook == nullptr) {
		if (hook()) {
			spdlog::info("ControllerHook hooked successfully.");
			g_controllerHook = this;
		}
		else {
			spdlog::info("ControllerHook failed to hook.");
		}
	}
}

ControllerHook::~ControllerHook()
{
	m_controllerUpdateHook->remove();
	m_controllerUpdateHook.reset();

	g_controllerHook = nullptr;
}

bool ControllerHook::hook()
{
	const uintptr_t base = g_framework->get_module();
	const uintptr_t sub14288CD10Fn = base + 0x2891060; // E3 5D C3 CC CC CC CC CC CC CC CC 48 89 5C 24 08 48 89 6C 24 18 +0x0B

	m_controllerUpdateHook = std::make_unique<FunctionHook>(sub14288CD10Fn, &Sub14288CD10);

	m_hooked = m_controllerUpdateHook->create();

	return m_hooked;
}

void ControllerHook::Sub14288CD10(uintptr_t uknPtr, dmc5::HIDGamePadDevice*& gamePadDeviceP, uint64_t uknArg, uint64_t uknIndex)
{
	const auto Sub14288CD10Fn = g_controllerHook->m_controllerUpdateHook->get_original<decltype(Sub14288CD10)>();
	Sub14288CD10Fn(uknPtr, gamePadDeviceP, uknArg, uknIndex);

	if (gamePadDeviceP) {
		g_framework->on_gamepad_keys(*gamePadDeviceP);

		if(m_is_ignoring_input)
		{
			gamePadDeviceP->buttons = 0;
			gamePadDeviceP->buttonsDown = 0;
			gamePadDeviceP->buttonsUp = 0;
			gamePadDeviceP->triggerL = 0.0f;
			gamePadDeviceP->triggerR = 0.0f;
		}
	}
}
