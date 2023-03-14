#pragma once
#include "Mod.hpp"
#include <sol/sol.hpp>
#include <reframework/API.hpp>
using namespace reframework;




class LuaMod : public Mod {
public:
	 void on_lua_state_created(lua_State* l) {
		 API::LuaLock _{};
		 m_global_state = l;
		 m_global_state_exists = true;
		 
	};
	 void on_lua_state_destroyed(lua_State* l) {
		 API::LuaLock _{};
		 m_global_state = nullptr;
		 m_global_state_exists = false;
		 unload_lua_mod();
	 };

	
	void load_lua_mod(){
		m_mod_state = API::get()->param()->functions->create_script_state();
		on_load_lua_mod();
		m_lua_loaded = true;
	}
	void unload_lua_mod(){

		on_unload_lua_mod();//should this go first in case we need to 
		
		API::get()->param()->functions->delete_script_state(m_mod_state);
		m_mod_state = nullptr;
		m_lua_loaded = false;
		
	}

	void on_frame() override {
		API::LuaLock _{};
		if (m_global_state_exists && m_is_enabled != nullptr) {
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
	lua_State* m_global_state;
	lua_State* m_mod_state;
	bool m_global_state_exists = false;
	bool m_lua_loaded = false;
};