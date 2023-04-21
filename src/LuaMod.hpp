#pragma once
#include "Mod.hpp"
#include <sol/sol.hpp>
#include <reframework/API.hpp>
#include "fw-imgui/ImGuiLua.h"
#include <filesystem>
using namespace reframework;
using namespace std::filesystem;
class LuaMod : public Mod {
public:
	 void on_lua_state_created(lua_State* l) {
		 m_global_state = l;
		 
	};
	 void on_lua_state_destroyed(lua_State* l) {
		 m_global_state = nullptr;
		 unload_lua_mod();
	 };




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
		on_unload_lua_mod();//should this go first in case we need to 
		//then clear the ui functions vector
		API::LuaLock _{};
		m_on_draw_ui_fns.clear();
		API::get()->param()->functions->delete_script_state(m_mod_state);
		m_mod_state = nullptr;
		m_lua_loaded = false;

	}
	lua_State* get_mod_state() {
		return m_mod_state;
	}
	bool is_lua_loaded() { return m_lua_loaded; }
	/// <summary>
	/// Draw any imgui calls made by a lua script to the mod window. 
	/// </summary>
	void draw_lua_ui() {
		if (ModFramework::get_ref_lua_enabled() && m_is_enabled != nullptr) {
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
			else if (*m_is_enabled && m_lua_loaded) {
				on_lua_mod_update();
			}
			else if (!*m_is_enabled && m_lua_loaded) {
				unload_lua_mod();
			}
		}
	}

	virtual void on_load_lua_mod() = 0;
	virtual void on_unload_lua_mod() = 0;
	virtual void on_lua_mod_update() {};

	/// <summary>
	/// Loads scripts from the m_scripts vector into the mod state.
	/// Needs LuaLock Before calling
	/// </summary>
	void load_scripts() {		
		for (auto& script : m_scripts) {
			load_script(script);
		}
	}

	/// <summary>
	/// Wrapped for load script that uses the Scripts Folder as the default filepath parameter.
	/// </summary>
	/// <param name="script">name of lua file</param>
	void load_script(std::string script) {
		load_script(script, path(SCRIPT_FOLDER));
	}
	/// <summary>
	/// Takes a file name and file path and attempts to load the file into the mod's lua script state.
	/// </summary>
	/// <param name="script">name of lua file</param>
	/// <param name="script_path">path to lua file</param>
	void load_script(std::string filename,std::filesystem::path filepath) {
		sol::state_view mod_state_view{ m_mod_state };
		auto scriptpath = filepath / filename;
		std::string old_path = mod_state_view["package"]["path"];
		try {
			auto dir = scriptpath.parent_path();

			std::string package_path = mod_state_view["package"]["path"];

			package_path = old_path + ";" + dir.string() + "/?.lua";
			package_path = package_path + ";" + dir.string() + "/?/init.lua";
			package_path = package_path + ";" + dir.string() + "/?.dll";

			mod_state_view["package"]["path"] = package_path;
			mod_state_view.safe_script_file(scriptpath.string());
		}
		catch (const std::exception& e) {
			spdlog::error(e.what());
			store_last_error = e.what();
		}
		catch (...) {
			spdlog::error((std::stringstream{} << "Unknown error when running script " << scriptpath).str().c_str());
			store_last_error = "Unknown error when running script " + scriptpath.string();
		}
		mod_state_view["package"]["path"] = old_path;
	}
protected:
	std::vector<sol::protected_function> m_on_draw_ui_fns{};
	std::vector<std::string> m_scripts{};
	std::string store_last_error = "";
	lua_State* m_global_state;
	lua_State* m_mod_state;
	bool m_lua_loaded = false;

private:

	/// <summary>
	/// Shamelessly borrowed from Re framework, acts as a handler for protected function results.
	/// </summary>
	/// <param name="result">takes the result of a protected function</param>
	/// <returns></returns>
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
};
