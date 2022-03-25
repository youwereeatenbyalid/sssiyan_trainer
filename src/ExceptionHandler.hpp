#pragma once
#include <windows.h>

namespace eh {
	LONG WINAPI global_exception_handler(struct _EXCEPTION_POINTERS* ei);
	void setup_exception_handler();
}