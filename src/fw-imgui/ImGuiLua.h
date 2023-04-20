#pragma once
#include <sol/sol.hpp>
#include <reframework/API.hpp>
class LuaMod;
namespace bindings {
	void open_imgui(LuaMod* s);
}

namespace api::imnodes {
	// pop the active nodes/editors/attributes etc so we dont crash.
	void cleanup();
}