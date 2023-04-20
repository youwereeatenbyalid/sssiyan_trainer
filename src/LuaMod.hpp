#pragma once
#include "Mod.hpp"
#include <sol/sol.hpp>
#include <reframework/API.hpp>
#include "fw-imgui/ImGuiLua.h"
using namespace reframework;



class LuaMod : public Mod {
public:
	 void on_lua_state_created(lua_State* l) {
		 m_global_state = l;
		 
	};
	 void on_lua_state_destroyed(lua_State* l) {
		 m_global_state = nullptr;
		 unload_lua_mod();
	 };

	 //Shamelessly borrowed from Re framework
	 sol::protected_function_result handle_protected_result(sol::protected_function_result result) {
		 if (result.valid()) {
			 return result;
		 }
		 sol::script_default_on_error(m_mod_state, std::move(result));
		 return result;
	 }

		
	 //re.on_draw_ui -> store in our ui functions list instead of reframework.
	 void bind_ui() {
		 //don't want to remove the scoped requirement from on_load_lua_mod so
		 API::LuaLock _{};
		 sol::state_view mod_state_view{ m_mod_state };
		 sol::table re = mod_state_view["re"];
		 re["on_draw_ui"] = [this](sol::function fn) { m_on_draw_ui_fns.emplace_back(fn); };
	 }


	void load_lua_mod(){
		m_mod_state = API::get()->param()->functions->create_script_state();
		//we might need to clear the imgui binds first but if not
		bindings::open_imgui(this);
		//Rebind the on draw UI
		bind_ui();
		//mod specific loading
		on_load_lua_mod();
		m_lua_loaded = true;
	}
	void unload_lua_mod(){
		API::LuaLock _{};
		on_unload_lua_mod();//should this go first in case we need to 
		//then clear the ui functions vector
		m_on_draw_ui_fns.clear();

		API::get()->param()->functions->delete_script_state(m_mod_state);
		m_mod_state = nullptr;
		m_lua_loaded = false;

	}
	lua_State* get_mod_state() {
		return m_mod_state;
	}
	bool is_lua_loaded() { return m_lua_loaded; }

	void draw_lua_ui() {
		if (m_lua_loaded) {
			API::LuaLock _{};
			try {
				for (auto& fn : m_on_draw_ui_fns) {
					handle_protected_result(fn());
				}
			}
			catch (const std::exception& e) {
				spdlog::error(e.what());
			}
			catch (...) {
				spdlog::error("Unknown error in on_draw_ui for " + m_full_name_string);
			}
		}
	}

	void on_frame() override {
		if (ModFramework::get_ref_lua_enabled() && m_is_enabled != nullptr) {
			if (*m_is_enabled && !m_lua_loaded) {
				load_lua_mod();
			}
			else if (!*m_is_enabled && m_lua_loaded) {
				unload_lua_mod();
			}
			else if (*m_is_enabled && m_lua_loaded) {
				on_lua_mod_update();
			}
		}
	}

	virtual void on_load_lua_mod() = 0;
	virtual void on_unload_lua_mod() = 0;
	virtual void on_lua_mod_update() {};
protected:
	std::vector<sol::protected_function> m_on_draw_ui_fns{};
	lua_State* m_global_state;
	lua_State* m_mod_state;
	bool m_lua_loaded = false;
};
