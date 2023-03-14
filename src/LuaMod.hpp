#pragma once
#include "Mod.hpp"
#include <sol/sol.hpp>
#include <reframework/API.hpp>
using namespace reframework;


class LuaMod : public Mod {
public:
	 void on_lua_state_created(lua_State* l) {
		 m_global_state = l;
		 m_global_state_exists = true;
		 
	};
	 void on_lua_state_destroyed(lua_State* l) {
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
		API::LuaLock _{};
		on_unload_lua_mod();//should this go first in case we need to 
		API::get()->param()->functions->delete_script_state(m_mod_state);
		m_mod_state = nullptr;
		m_lua_loaded = false;

	}
	lua_State* get_mod_state() {
		return m_mod_state;
	}
	bool is_lua_loaded() { return m_lua_loaded; }

	void on_frame() override {
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


class LuaDragFloat {
public:
	LuaDragFloat(LuaMod& mod, float& value_address, std::string lua_label, std::string imgui_label = "") {
		m_imgui_label = imgui_label;
		m_lua_label = lua_label;
		m_value = &value_address;
		m_mod_reference = &mod;
	};

	void draw() {
		if (ImGui::DragFloat(m_imgui_label.c_str(), m_value, 1.0f, 0.0f, 100.0f)) {
			API::LuaLock _{};
		}
	};
private:
	float* m_value;
	std::string m_imgui_label;
	std::string m_lua_label;
	LuaMod* m_mod_reference;
};