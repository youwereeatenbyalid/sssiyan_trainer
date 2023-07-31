#include "LuaDevilBreakerSwitcher.hpp"
#include "mods/PlayerTracker.hpp"
bool LuaDevilBreakerSwitcher::cheaton{NULL};
void LuaDevilBreakerSwitcher::init_check_box_info() {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}


std::optional<std::string> LuaDevilBreakerSwitcher::on_initialize() {
    init_check_box_info();
    m_scripts = {"BreakerSwitcher.lua"};//scripts here
    m_is_enabled = &LuaDevilBreakerSwitcher::cheaton;
    m_on_page = Page_Breaker;
    m_depends_on = { "PlayerTracker" };
    m_full_name_string = "Breaker Switcher V5 (+)";
    m_author_string = "The Hitchhiker";
    m_description_string = "Even funkier than before.";
    infinite_breakers = false;
    breaker_to_breaker_cancel = false;
    verify_scripts();
    set_up_hotkey();

    g_keyBinds.Get()->AddBind(std::string(get_name()) + "SLOT1",
        [this]() {
            //if (PlayerTracker::ingameplay && PlayerTracker::playerid == 0) {
                keyboard_breaker = 1;
                //}

        }, OnState_Press);

    g_keyBinds.Get()->AddBind(std::string(get_name()) + "SLOT2",
        [this]() {
            //if (PlayerTracker::ingameplay && PlayerTracker::playerid == 0) {
                keyboard_breaker = 2;
                //}

        }, OnState_Press);

    g_keyBinds.Get()->AddBind(std::string(get_name()) + "SLOT3",
        [this]() {
            //if (PlayerTracker::ingameplay && PlayerTracker::playerid == 0) {
                keyboard_breaker = 3;
                //}

        }, OnState_Press);

    g_keyBinds.Get()->AddBind(std::string(get_name()) + "SLOT4",
        [this]() {
            //if (PlayerTracker::ingameplay && PlayerTracker::playerid == 0) {
                keyboard_breaker = 4;
                //}

        }, OnState_Press);

    g_keyBinds.Get()->AddBind(std::string(get_name()) + "SLOT5",
        [this]() {
            //if (PlayerTracker::ingameplay && PlayerTracker::playerid == 0) {
                keyboard_breaker = 5;
                //}

        }, OnState_Press);

    g_keyBinds.Get()->AddBind(std::string(get_name()) + "SLOT6",
        [this]() {
            //if (PlayerTracker::ingameplay && PlayerTracker::playerid == 0) {
                keyboard_breaker = 6;
            //}

        }, OnState_Press);

    g_keyBinds.Get()->AddBind(std::string(get_name()) + "SLOT7",
        [this]() {
            //if (PlayerTracker::ingameplay && PlayerTracker::playerid == 0) {
                keyboard_breaker = 7;
                //}

        }, OnState_Press);

    g_keyBinds.Get()->AddBind(std::string(get_name()) + "SLOT8",
        [this]() {
            //if (PlayerTracker::ingameplay && PlayerTracker::playerid == 0) {
                keyboard_breaker = 8;
            //}

        }, OnState_Press);

    return Mod::on_initialize();
}

void LuaDevilBreakerSwitcher::on_load_lua_mod()
{
    API::LuaLock _{};
    load_scripts();
    //running lua on load here
}

void LuaDevilBreakerSwitcher::on_config_load(const utility::Config& cfg) {
    for (int i = 0; i < 8; i++) {
        breakers[i] =
            cfg.get<int>("breaker_slot_" + std::to_string(i)).value_or(0);
    }
    //BreakerSwitcher::switcher_mode = cfg.get<int>("switcher_mode").value_or(0);
    breakaway_type = cfg.get<int>("breakaway_type").value_or(0);
    //breakaway_index = cfg.get<int>("breakaway_button").value_or(0);
    infinite_breakers = cfg.get<bool>("infinite_breakers").value_or(false);
    //BreakerSwitcher::use_secondary = cfg.get<bool>("use_secondary").value_or(false);
    breaker_to_breaker_cancel = cfg.get<bool>("faster_breaker").value_or(false);
    //BreakerSwitcher::instantkeyboardbreakers = cfg.get<bool>("instant_keyboard_breakers").value_or(false);
    //BreakerSwitcher::breakaway_button = bsinputs[breakaway_index];
    disable_diagonal = cfg.get<bool>("disable_diagonal").value_or(false);
}

// during load

// during save
void LuaDevilBreakerSwitcher::on_config_save(utility::Config& cfg) {
    for (int i = 0; i < 8; i++) {
        cfg.set<int>("breaker_slot_" + std::to_string(i), breakers[i]);
    }
    //cfg.set<int>("switcher_mode", BreakerSwitcher::switcher_mode);
    cfg.set<int>("breakaway_type", breakaway_type);
    //cfg.set<int>("breakaway_button", breakaway_index);
    cfg.set<bool>("infinite_breakers", infinite_breakers);
    //cfg.set<bool>("use_secondary", BreakerSwitcher::use_secondary);
    cfg.set<bool>("faster_breaker", breaker_to_breaker_cancel);
    cfg.set<bool>("disable_diagonal", disable_diagonal);
    //cfg.set<bool>("instant_keyboard_breakers", BreakerSwitcher::instantkeyboardbreakers);
}

void LuaDevilBreakerSwitcher::on_unload_lua_mod()
{
    API::LuaLock _{};
}

void LuaDevilBreakerSwitcher::on_lua_mod_update()
{
    API::LuaLock _{};
    sol::state_view mod_state_view{ m_mod_state };
    //lua actions on update here
    mod_state_view["breakerlist"][1] = breakers[0];
    mod_state_view["breakerlist"][2] = breakers[1];
    mod_state_view["breakerlist"][3] = breakers[2];
    mod_state_view["breakerlist"][4] = breakers[3];
    mod_state_view["breakerlist"][5] = breakers[4];
    mod_state_view["breakerlist"][6] = breakers[5];
    mod_state_view["breakerlist"][7] = breakers[6];
    mod_state_view["breakerlist"][8] = breakers[7];
    //request keyboard breaker switch
    mod_state_view["keyboard_breaker"] = keyboard_breaker;
    keyboard_breaker = -1;
    //check if infinite breakers
    mod_state_view["nero_removeGauntlet"]["noConsumption"] = infinite_breakers;
    mod_state_view["disable_diagonal"] = disable_diagonal;
    if (breakaway_type == 1) {
        mod_state_view["alwaysbreakaway"] = true;
    }
    else {
        mod_state_view["alwaysbreakaway"] = false;
    }
    mod_state_view["breaker_to_breaker_cancel"] = breaker_to_breaker_cancel;
    
}

void LuaDevilBreakerSwitcher::on_draw_ui()
{
    draw_lua_ui();

    //ImGui::InputInt("breaker up", (int*)&BreakerSwitcher::breakers[0]);
//ImGui::InputInt("breaker down", (int*)&BreakerSwitcher::breakers[1]);
//ImGui::InputInt("breaker left", (int*)&BreakerSwitcher::breakers[2]);
//ImGui::InputInt("breaker right", (int*)&BreakerSwitcher::breakers[3]);
    //UI::Combo("Breaker Type", (int*)&BreakerSwitcher::switcher_mode, "Off\0Switcher\0Cycler\0");
    //"Off\0Hold button to override\0On\0"
    UI::Combo("Breakaway Type", &this->breakaway_type, "Hold button to override\0On\0");
    if (breakaway_type == 0) {
        /*if (UI::Combo("Override button", &breakaway_index,
            "Sword\0Gun\0Jump\0Taunt\0"
            "Lock-on\0Change Target\0"
            "Dpad Up\0Dpad Down\0Dpad Left\0Dpad Right\0"
            "Breaker Action\0Exceed\0Reset Camera\0")) {
            BreakerSwitcher::breakaway_button = bsinputs[breakaway_index];
        }
        */
        ImGui::Text("Currently only hold button is breaker");
    }
    //ImGui::Checkbox("Use Secondary Breaker", (bool*)&BreakerSwitcher::use_secondary);
    //ImGui::ShowHelpMarker("If Nero's magazine has at least two breakers, the second breaker will be used in place of the devil bringer.");
    ImGui::Checkbox("Infinite Breakers##What Even", (bool*)&infinite_breakers);
    ImGui::Checkbox("Faster Breakers", (bool*)&breaker_to_breaker_cancel);
    ImGui::ShowHelpMarker("Removes Recovery on the breaker, allowing it to cancel into itself.");
    ImGui::Checkbox("Disable diagonal slots", (bool*)&disable_diagonal);
    

    //ImGui::Checkbox("Instant Keyboard Breakers", (bool*)&BreakerSwitcher::instantkeyboardbreakers);
    //ImGui::ShowHelpMarker("Activating the breaker swap hotkeys will also perform the breaker move.");

    auto breakerboxstring =
        "Overture\0Ragtime\0Helter Skelter\0Gerbera\0Punchline\0Buster "
        "Arm\0Rawhide\0Tomboy\0Mega Buster\0Gerbera GP01\0Pasta Breaker\0Sweet "
        "Surrender\0Monkey Business\0";
    UI::Combo("breaker up", (int*)&breakers[0], breakerboxstring);
    ImGui::SameLine();
    UI::KeyBindButton(std::string(get_name()) + "SLOT1", std::string(get_name()) + "SLOT1", g_framework->get_kcw_buffers(), 1.0f, true, UI::BUTTONCOLOR);
    UI::Combo("breaker down", (int*)&breakers[1], breakerboxstring);
    ImGui::SameLine();
    UI::KeyBindButton(std::string(get_name()) + "SLOT2", std::string(get_name()) + "SLOT2", g_framework->get_kcw_buffers(), 1.0f, true, UI::BUTTONCOLOR);
    UI::Combo("breaker left", (int*)&breakers[2], breakerboxstring);
    ImGui::SameLine();
    UI::KeyBindButton(std::string(get_name()) + "SLOT3", std::string(get_name()) + "SLOT3", g_framework->get_kcw_buffers(), 1.0f, true, UI::BUTTONCOLOR);
    UI::Combo("breaker right", (int*)&breakers[3], breakerboxstring);
    ImGui::SameLine();
    UI::KeyBindButton(std::string(get_name()) + "SLOT4", std::string(get_name()) + "SLOT4", g_framework->get_kcw_buffers(), 1.0f, true, UI::BUTTONCOLOR);
    UI::Combo("breaker up left", (int*)&breakers[4], breakerboxstring);
    ImGui::SameLine();
    UI::KeyBindButton(std::string(get_name()) + "SLOT5", std::string(get_name()) + "SLOT5", g_framework->get_kcw_buffers(), 1.0f, true, UI::BUTTONCOLOR);
    UI::Combo("breaker up right", (int*)&breakers[5], breakerboxstring);
    ImGui::SameLine();
    UI::KeyBindButton(std::string(get_name()) + "SLOT6", std::string(get_name()) + "SLOT6", g_framework->get_kcw_buffers(), 1.0f, true, UI::BUTTONCOLOR);
    UI::Combo("breaker down left", (int*)&breakers[7], breakerboxstring);
    ImGui::SameLine();
    UI::KeyBindButton(std::string(get_name()) + "SLOT7", std::string(get_name()) + "SLOT7", g_framework->get_kcw_buffers(), 1.0f, true, UI::BUTTONCOLOR);
    UI::Combo("breaker down right", (int*)&breakers[6], breakerboxstring);
    ImGui::SameLine();
    UI::KeyBindButton(std::string(get_name()) + "SLOT8", std::string(get_name()) + "SLOT8", g_framework->get_kcw_buffers(), 1.0f, true, UI::BUTTONCOLOR);
}

