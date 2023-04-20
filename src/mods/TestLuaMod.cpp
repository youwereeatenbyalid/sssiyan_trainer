#include "TestLuaMod.hpp"
bool TestLuaMod::cheaton{NULL};
char TestLuaMod::filenamestring[64]{"sandboxscript2.lua"};
void TestLuaMod::init_check_box_info() {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}


std::optional<std::string> TestLuaMod::on_initialize() {
    init_check_box_info();

    m_is_enabled = &TestLuaMod::cheaton;
    m_on_page = Page_CommonCheat;
    m_full_name_string = "Test Lua Mod ";
    m_author_string = "The HitchHiker";
    m_description_string = "Let's hope this works.";

    set_up_hotkey();

    return Mod::on_initialize();
}

void TestLuaMod::on_load_lua_mod()
{
    API::LuaLock _{};
    sol::state_view mod_state_view{m_mod_state};
    try {
        mod_state_view.safe_script_file(filenamestring);
    }
    catch (const std::exception& e) {
        spdlog::error(e.what());
    }
    catch (...) {
        spdlog::error((std::stringstream{} << "Unknown error when running script " << filenamestring).str().c_str());
    }
    spdlog::info("Hello from TestLuaMod::on_load_lua_mod!");
}

void TestLuaMod::on_unload_lua_mod()
{
   // API::LuaLock _{};
    spdlog::info("Hello from TestLuaMod::on_unload_lua_mod!");
}

void TestLuaMod::on_lua_mod_update()
{
    API::LuaLock _{};
    sol::state_view mod_state_view{ m_mod_state };
    mod_state_view["royalguardgauge"] = royalguardgauge;
}

void TestLuaMod::on_draw_ui()
{
    ImGui::InputText("Filename", filenamestring, IM_ARRAYSIZE(filenamestring));
    ImGui::DragFloat("Royalguardvalue", &royalguardgauge, 1.0f, 0.0f, 30000.0f);
    draw_lua_ui();
}

