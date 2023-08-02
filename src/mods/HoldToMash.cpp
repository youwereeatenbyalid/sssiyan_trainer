#include "HoldToMash.hpp"
#include "mods/PlayerTracker.hpp"
//jumps
uintptr_t HoldToMash::jmp_return{NULL};
bool HoldToMash::cheaton{NULL};
//variables
// clang-format off
// only in clang/icl mode on x64, sorry
std::string types = "Hold to Mash\0Double Press To Mash\0";
int activation_type = 0;
struct renda {
    float threshhold = 0.0f;
    float frequency = 0.0f;
};
renda sword_renda;
renda style_renda;
renda ebony_ivory;
renda summon_sword;
//Issue with this right now is the renda check on the rising dragon is a single button, making it way too easy to get dragon punch.
//actual field is app.RendaChecker.<count>k__BackingField at 0x14, set to 0.
//Easiest solution is just requiring the hold button be held a few extra frames before copying to press and starting the recka,
//but that might mess with other stuff. 
//Also, I want to redo the mash stuff for ebony and ivory, so it might be worth redoing this as lua or with SDK.
  static naked void newmem_detour() {
__asm {
  validation:
    //cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
    //jne code
    cmp byte ptr [HoldToMash::cheaton], 1
    je cheatcode
    jmp code
  code:
    test [r8+0x48], eax
    seta al
    jmp qword ptr [HoldToMash::jmp_return]

  cheatcode:
    test [r8+0x3C], eax
    seta al
    jmp qword ptr [HoldToMash::jmp_return]
  }
}

// clang-format on

void HoldToMash::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> HoldToMash::on_initialize() {
  init_check_box_info();

  auto base              = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled           = &HoldToMash::cheaton;
  m_on_page              = Page_QOL;
  //m_depends_on           = { "PlayerTracker" };
  m_scripts = { "HoldToMash.lua" };//scripts here
  m_full_name_string     = "Hold To Mash";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Hold button for inputs like twosometime, million stab, rising dragon, etc.";

  set_up_hotkey();
  verify_scripts();
  //auto holdtomash_addr = m_patterns_cache->find_addr(base, "41 85 40 48 0F 97 C0");
  //
  //if (!holdtomash_addr) {
  //  return "Unable to find holdtomash pattern.";
  //}
  //if (!install_new_detour(holdtomash_addr.value(), m_holdtomash_detour, &newmem_detour, &jmp_return, 7)) {
  //  //return a error string in case something goes wrong
  //  spdlog::error("[{}] failed to initialize", get_name());
  //  return "Failed to initialize holdtomash";
  //}

  return Mod::on_initialize();
}


void HoldToMash::on_load_lua_mod()
{
    API::LuaLock _{};
    load_scripts();
    //running lua on load here
}

void HoldToMash::on_unload_lua_mod()
{
    API::LuaLock _{};
}

void HoldToMash::on_lua_mod_update()
{
    API::LuaLock _{};
    sol::state_view mod_state_view{ m_mod_state };
    sol::optional<float> threshold_check = mod_state_view["dante_ebony_ivory"]["threshhold"];
    if (!threshold_check.has_value())
        return;
    mod_state_view["sword_renda"]["type"] = activation_type + 1;
    mod_state_view["style_renda"]["type"] = activation_type + 1;
    mod_state_view["dante_ebony_ivory"]["type"] = activation_type + 1;
    mod_state_view["vergil_summoned_swords"]["type"] = activation_type + 1;

    mod_state_view["dante_ebony_ivory"]["threshhold"] = ebony_ivory.threshhold;
    mod_state_view["dante_ebony_ivory"]["mash_frequency"] = ebony_ivory.frequency;

    mod_state_view["sword_renda"]["threshhold"] = sword_renda.threshhold;
    mod_state_view["sword_renda"]["mash_frequency"] = sword_renda.frequency;

    mod_state_view["style_renda"]["threshhold"] = style_renda.threshhold;
    mod_state_view["style_renda"]["mash_frequency"] = style_renda.frequency;

    mod_state_view["vergil_summoned_swords"]["threshhold"] = summon_sword.threshhold;
    mod_state_view["vergil_summoned_swords"]["mash_frequency"] = summon_sword.frequency;
    //lua actions on update here
}

void draw_threshhold(std::string label, float& threshhold, float& frequency) {
    ImGui::Text(label.c_str());
    ImGui::SliderFloat(("Startup Window##" + label).c_str(), &threshhold, 0.0, 120.0);
    ImGui::SliderFloat(("Frequency##" + label).c_str(), &frequency, 0.0, 10.0);
    ImGui::Separator();
}

void HoldToMash::on_draw_ui()
{
    auto combostring = "Hold to Mash\0Double Press To Mash\0";
    UI::Combo("Activation Type", (int*)&activation_type, combostring);
    ImGui::Separator();
    draw_threshhold("Ebony & Ivory Rapid-fire", ebony_ivory.threshhold, ebony_ivory.frequency);
    draw_threshhold("Vergil Summon Sword Rapid-fire", summon_sword.threshhold, summon_sword.frequency);
    draw_threshhold("Melee Mash", sword_renda.threshhold, sword_renda.frequency);
    draw_threshhold("Dante Style Mash", style_renda.threshhold, style_renda.frequency);
   
    draw_lua_ui();
}



// during load
void HoldToMash::on_config_load(const utility::Config &cfg) {
    sword_renda.threshhold = cfg.get<float>("sword_renda_threshhold").value_or(12.0);
    sword_renda.frequency = cfg.get<float>("sword_renda_frequency").value_or(5.0);

    style_renda.threshhold = cfg.get<float>("style_renda_threshhold").value_or(12.0);
    style_renda.frequency = cfg.get<float>("style_renda_frequency").value_or(5.0);

    ebony_ivory.threshhold = cfg.get<float>("ebony_ivory_threshhold").value_or(12.0);
    ebony_ivory.frequency = cfg.get<float>("ebony_ivory_frequency").value_or(5.0);

    summon_sword.threshhold = cfg.get<float>("summon_sword_threshhold").value_or(12.0);
    summon_sword.frequency = cfg.get<float>("summon_sword_frequency").value_or(5.0);

    activation_type = cfg.get<int>("activation_type").value_or(1);
    
}
// during save
void HoldToMash::on_config_save(utility::Config &cfg) {
    cfg.set<float>("sword_renda_threshhold", sword_renda.threshhold);
    cfg.set<float>("sword_renda_frequency", sword_renda.frequency);

    cfg.set<float>("style_renda_threshhold", style_renda.threshhold);
    cfg.set<float>("style_renda_frequency", style_renda.frequency);

    cfg.set<float>("ebony_ivory_threshhold", ebony_ivory.threshhold);
    cfg.set<float>("ebony_ivory_frequency", ebony_ivory.frequency);

    cfg.set<float>("summon_sword_threshhold", summon_sword.threshhold);
    cfg.set<float>("summon_sword_frequency", summon_sword.frequency);

    cfg.set<int>("activation_type", activation_type);
}
// do something every frame
//void HoldToMash::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void HoldToMash::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void HoldToMash::on_draw_ui() {}