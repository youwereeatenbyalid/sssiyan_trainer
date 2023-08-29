#include "TauntSelector.hpp"
#include "mods/PlayerTracker.hpp"
#include "mods/StyleRank.hpp"
uintptr_t TauntSelector::jmp_ret{NULL};
bool TauntSelector::cheaton{NULL};
bool use_dev_names{ false };
//Taunt order: Air, Lock-on, S-Rank Air, S-Rank Lockon, BP taunt, EX Taunt
uint32_t TauntSelector::nerotaunts[6]{0,0,0,0,0,0};
uint32_t TauntSelector::dantetaunts[6]{0,0,0,0,0,0};
uint32_t TauntSelector::vtaunts[6]{0,0,0,0,0,0};
uint32_t TauntSelector::vergiltaunts[6]{0,0,0,0,0,0};
std::map<std::string, uint32_t> nerotauntmap;
std::map<std::string, uint32_t> dantetauntmap;
std::map<std::string, uint32_t> vtauntmap;
std::map<std::string, uint32_t> vergiltauntmap;

std::map<std::string, uint32_t> nerotauntmap_dev;
std::map<std::string, uint32_t> dantetauntmap_dev;
std::map<std::string, uint32_t> vtauntmap_dev;
std::map<std::string, uint32_t> vergiltauntmap_dev;

std::string nerotauntstrings[6]   = { "Default", "Default", "Default",
                                      "Default", "Default", "Default" };
std::string dantetauntstrings[6]  = { "Default", "Default", "Default",
                                      "Default", "Default", "Default" };
std::string vtauntstrings[6]      = { "Default", "Default", "Default",
                                      "Default", "Default", "Default" };
std::string vergiltauntstrings[6] = { "Default", "Default", "Default",
                                      "Default", "Default", "Default" };
    // clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	validation:
        cmp byte ptr [TauntSelector::cheaton], 1
        jne code
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        je nerocode
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        je dantecode
        cmp [PlayerTracker::playerid], 2 //change this to the char number obviously
        je vcode
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        je vergilcode
        jmp code

    code:
        cmp byte ptr [rdi+20h], 00  // new
        mov rcx,rbx                 // new
        mov r8d, [rdi+0x10]
        setne r9b
        jmp qword ptr [TauntSelector::jmp_ret] 

    nerocode:
        push r10
        push r11
        lea r11, [TauntSelector::nerotaunts]
        mov r10, 5
        jmp tauntcode

    dantecode:
        push r10
        push r11
        lea r11, [TauntSelector::dantetaunts]
        mov r10, 5
        jmp tauntcode

    vcode:
        push r10
        push r11
        lea r11, [TauntSelector::vtaunts]
        mov r10, 5
        jmp tauntcode

    vergilcode:
        push r10
        push r11
        lea r11, [TauntSelector::vergiltaunts]
        mov r10, 5
        jmp tauntcode

    tauntcode:
        cmp byte ptr [rdi+20h], 00  // new
        mov rcx,rbx                 // new
		mov r8d,[rdi+0x10]
        setne r9b

        //ex taunt 5
        cmp r8d, 0x7F
        je overridetaunt
        dec r10
        //bp taunt 4
        cmp r8d, 0x82
        je overridetaunt
        dec r10
        //s rank 3
        cmp byte ptr [StyleRank::rank], 5
        jae srankcode
        sub r10, 2
        //lock-on 1
        cmp r8d, 0x7C
        je overridetaunt
        cmp r8d, 0x7D
        je overridetaunt
        dec r10
        //air taunt 0
        cmp r8d, 0x7A
        je overridetaunt
        cmp r8d, 0x79
        je overridetaunt
        jmp ret_jmp

    srankcode:
        //lock-on s-rank 3
        cmp r8d, 0x7C
        je overridetaunt
        cmp r8d, 0x7D
        je overridetaunt
        dec r10
        //air taunt s-rank 2
        cmp r8d, 0x7A
        je overridetaunt
        //air taunt s-rank 2
        cmp r8d, 0x79
        je overridetaunt
        jmp ret_jmp

    overridetaunt:
        mov r11, [r11+r10*4]
        cmp r11d, 0
        je ret_jmp
        mov r8d, r11d
        jmp ret_jmp

    ret_jmp:
        pop r11
        pop r10
        jmp qword ptr [TauntSelector::jmp_ret]
	}
}

// clang-format on

void TauntSelector::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}


void initialize_dev_taunt_map(){
    nerotauntmap_dev["Default"] = 0x0;
    nerotauntmap_dev["Provoke.Provoke01.Start"] = 0x6A;
    nerotauntmap_dev["Provoke.Provoke02.Start"] = 0x6B;
    nerotauntmap_dev["Provoke.Provoke03.Start"] = 0x6C;
    nerotauntmap_dev["Provoke.Provoke04.Start"] = 0x6D;
    nerotauntmap_dev["Provoke.Provoke05.Start"] = 0x6E;
    nerotauntmap_dev["Provoke.Provoke06.Start"] = 0x6F;
    nerotauntmap_dev["Provoke.Provoke07.Start"] = 0x70;
    nerotauntmap_dev["Provoke.Provoke08.Start"] = 0x71;
    nerotauntmap_dev["Provoke.Provoke09.Start"] = 0x72;
    nerotauntmap_dev["Provoke.Provoke10.Start"] = 0x73;
    nerotauntmap_dev["Provoke.Provoke11.Start"] = 0x74;
    nerotauntmap_dev["Provoke.Provoke12.Start"] = 0x75;
    nerotauntmap_dev["Provoke.Provoke13.Start (unused)"] = 0x76;
    nerotauntmap_dev["Provoke.Provoke14.Start"] = 0x77;
    nerotauntmap_dev["Provoke.Provoke15.Start"] = 0x78;
    nerotauntmap_dev["Provoke.AirProvoke01.Start"] = 0x79;
    nerotauntmap_dev["Provoke.LockOnProvoke01.Start"] = 0x7C;
    nerotauntmap_dev["Provoke.LockOnProvoke02.Start"] = 0x7D;
    nerotauntmap_dev["Provoke.ShopProvoke01.Start"] = 0x7F;
    nerotauntmap_dev["Provoke.BPProvoke01.Start"] = 0x82;
    nerotauntmap_dev["Provoke.BPProvoke02.Start"] = 0x83;
    nerotauntmap_dev["Provoke.SpecialProvoke01.Start"] = 0x85;

    dantetauntmap_dev["Default"] = 0x0;
    dantetauntmap_dev["Provoke.Provoke01.Start"] = 0x6A;
    dantetauntmap_dev["Provoke.Provoke02.Start"] = 0x6B;
    dantetauntmap_dev["Provoke.Provoke03.Start"] = 0x6C;
    dantetauntmap_dev["Provoke.Provoke04.Start"] = 0x6D;
    dantetauntmap_dev["Provoke.Provoke05.Start"] = 0x6E;
    dantetauntmap_dev["Provoke.Provoke06.Start"] = 0x6F;
    dantetauntmap_dev["Provoke.Provoke07.Start"] = 0x70;
    dantetauntmap_dev["Provoke.Provoke08.Start"] = 0x71;
    dantetauntmap_dev["Provoke.Provoke09.Start"] = 0x72;
    dantetauntmap_dev["Provoke.Provoke10.Start"] = 0x73;
    dantetauntmap_dev["Provoke.Provoke11.Start"] = 0x74;
    dantetauntmap_dev["Provoke.Provoke12.Start"] = 0x75;
    dantetauntmap_dev["Provoke.AirProvoke01.Start"] = 0x79;
    dantetauntmap_dev["Provoke.LockOnProvoke01.Start"] = 0x7C;
    dantetauntmap_dev["Provoke.LockOnProvoke02.Start"] = 0x7D;
    dantetauntmap_dev["Provoke.ShopProvoke01.Start"] = 0x7F;
    dantetauntmap_dev["Provoke.BPProvoke01.Start"] = 0x82;

    vtauntmap_dev["Default"] = 0x0;
    vtauntmap_dev["Provoke.Provoke01.Start"] = 0x6A;
    vtauntmap_dev["Provoke.Provoke02.Start"] = 0x6B;
    vtauntmap_dev["Provoke.Provoke03.Start"] = 0x6C;
    vtauntmap_dev["Provoke.Provoke04.Start"] = 0x6D; //6D - Provoke04.End
    vtauntmap_dev["Provoke.Provoke05.Start"] = 0x6E; //6E - Provoke05.End
    vtauntmap_dev["Provoke.Provoke06.Start"] = 0x6F;
    vtauntmap_dev["Provoke.Provoke07.Start"] = 0x70;
    vtauntmap_dev["Provoke.Provoke08.Start"] = 0x71;
    vtauntmap_dev["Provoke.Provoke09.Start"] = 0x72;
    vtauntmap_dev["Provoke.Provoke10.Start"] = 0x73;
    vtauntmap_dev["Provoke.Provoke11.Start"] = 0x74;
    vtauntmap_dev["Provoke.Provoke12.Start"] = 0x75;
    vtauntmap_dev["Provoke.AirProvoke01.Start"] = 0x79;
    vtauntmap_dev["Provoke.AirProvoke02.Start"] = 0x7A;
    vtauntmap_dev["Provoke.AirProvoke03.Start (Unused)"] = 0x7B;
    vtauntmap_dev["Provoke.LockOnProvoke01.Start"] = 0x7C;
    vtauntmap_dev["Provoke.ShopProvoke01.Start"] = 0x7F;
    vtauntmap_dev["Provoke.BPProvoke01.Start"] = 0x82;

    vergiltauntmap_dev["Default"] = 0x0;
    vergiltauntmap_dev["Provoke.Provoke01.Start"] = 0x6A;
    vergiltauntmap_dev["Provoke.Provoke02.Start"] = 0x6B;
    vergiltauntmap_dev["Provoke.Provoke03.Start"] = 0x6C;
    vergiltauntmap_dev["Provoke.Provoke04.Start"] = 0x6D;
    vergiltauntmap_dev["Provoke.Provoke05.Start"] = 0x6E;
    vergiltauntmap_dev["Provoke.Provoke06.Start"] = 0x6F;
    vergiltauntmap_dev["Provoke.Provoke07.Start"] = 0x70;
    vergiltauntmap_dev["Provoke.Provoke08.Start"] = 0x71;
    vergiltauntmap_dev["Provoke.Provoke09.Start"] = 0x72;
    vergiltauntmap_dev["Provoke.Provoke10.Start"] = 0x73;
    vergiltauntmap_dev["Provoke.Provoke11.Start"] = 0x74;
    vergiltauntmap_dev["Provoke.Provoke12.Start"] = 0x75;
    vergiltauntmap_dev["Provoke.Provoke13.Start"] = 0x76;
    vergiltauntmap_dev["Provoke.AirProvoke01.Start"] = 0x79;
    vergiltauntmap_dev["Provoke.AirProvoke02.Start"] = 0x7A;
    vergiltauntmap_dev["Provoke.LockOnProvoke01.Start"] = 0x7C;
    vergiltauntmap_dev["Provoke.ShopProvoke01.Start"] = 0x7F;
    vergiltauntmap_dev["Provoke.BPProvoke01.Start"] = 0x82;
}
void initialize_user_taunt_map() {
    nerotauntmap["Default"] = 0x0;
    nerotauntmap["Throat cut"] = 0x6A;
    nerotauntmap["\"What'd you say?\""] = 0x6B;
    nerotauntmap["\"Come on!\""] = 0x6C;
    nerotauntmap["Gun fakeout"] = 0x6D;
    nerotauntmap["Sword Rev"] = 0x6E;
    nerotauntmap["\"I'll play with you\""] = 0x6F;
    nerotauntmap["\"Shall we dance?\""] = 0x70;
    nerotauntmap["Air Guitar"] = 0x71;
    nerotauntmap["Clapping"] = 0x72;
    nerotauntmap["Well! What is it!"] = 0x73;
    nerotauntmap["Kneel"] = 0x74;
    nerotauntmap["Instant reload"] = 0x75;
    nerotauntmap["Breaker Taunt (unused)"] = 0x76;
    nerotauntmap["Hood Taunt"] = 0x77;
    nerotauntmap["Camera Taunt"] = 0x78;
    nerotauntmap["Air taunt"] = 0x79;
    nerotauntmap["Lock-On provocation"] = 0x7C;
    nerotauntmap["S-Rank Lock-On provocation"] = 0x7D;
    nerotauntmap["EX provocation"] = 0x7F;
    nerotauntmap["BP provocation (Hoodless)"] = 0x82;
    nerotauntmap["BP provocation"] = 0x83;
    nerotauntmap["Gum taunt"] = 0x85;

    dantetauntmap["Default"] = 0x0;
    dantetauntmap["One arm beckon"] = 0x6A;
    dantetauntmap["Both arm Beckon"] = 0x6B;
    dantetauntmap["Clap Beckon"] = 0x6C;
    dantetauntmap["Shrug"] = 0x6D;
    dantetauntmap["Backup Beckon"] = 0x6E;
    dantetauntmap["One arm Beckon extented"] = 0x6F;
    dantetauntmap["Bend-over beckon"] = 0x70;
    dantetauntmap["Guts and honor"] = 0x71;
    dantetauntmap["\"come and get me!\""] = 0x72;
    dantetauntmap["\"Man, I'm bored\""] = 0x73;
    dantetauntmap["\"You look like you just had your ass handed to you\""] = 0x74;
    dantetauntmap["DSD Taunt"] = 0x75;
    dantetauntmap["Air taunt"] = 0x79;
    dantetauntmap["S-Rank Lock-On provocation"] = 0x7C;
    dantetauntmap["Lock-On provocation"] = 0x7D;
    dantetauntmap["EX provocation"] = 0x7F;
    dantetauntmap["BP provocation"] = 0x82;

    vtauntmap["Default"] = 0x0;
    vtauntmap["Slow Clap"] = 0x6A;
    vtauntmap["Ride of the Valkyries"] = 0x6B;
    vtauntmap["\"I need more strength\""] = 0x6C;
    //6D - Provoke04.End
    //6E - Provoke05.End
    vtauntmap["Mini Griffon"] = 0x6D;
    vtauntmap["Mini Shadow"] = 0x6E;
    vtauntmap["Mini Nightmare"] = 0x6F;
    vtauntmap["\"You make me laugh\""] = 0x70;
    vtauntmap["Caprice 24 - Violin"] = 0x71;
    vtauntmap["Tap-Dance"] = 0x72;
    vtauntmap["Coughing Fit"] = 0x73;
    vtauntmap["Pet Summon"] = 0x74;
    vtauntmap["Turn away"] = 0x75;
    vtauntmap["Air taunt"] = 0x79;
    vtauntmap["S-Rank Air taunt"] = 0x7A;
    //vtauntmap["AirProvoke03 (Unused)"] = 0x7B;
    vtauntmap["Lock-On provocation"] = 0x7C;
    vtauntmap["EX provocation"] = 0x7F;
    vtauntmap["BP provocation"] = 0x82;

    vergiltauntmap["Default"] = 0x0;
    vergiltauntmap["\"Show me your motivation!\""] = 0x6A;
    vergiltauntmap["\"You think you stand a chance?\""] = 0x6B;
    vergiltauntmap["\"You're wasting my time.\""] = 0x6C;
    vergiltauntmap["\"Disappointing.\""] = 0x6D;
    vergiltauntmap["\"Let's have some fun!\""] = 0x6E;
    vergiltauntmap["Sword Plant - Yamato"] = 0x6F;
    vergiltauntmap["Sword Plant - Force Edge"] = 0x70;
    vergiltauntmap["Book"] = 0x71;
    vergiltauntmap["Bringer"] = 0x72;
    vergiltauntmap["Warm up"] = 0x73;
    vergiltauntmap["Force Edge"] = 0x74;
    vergiltauntmap["\"Try Me.\""] = 0x75;
    vergiltauntmap["Kata"] = 0x76;
    vergiltauntmap["Summoned Sword Air Throw"] = 0x79;
    vergiltauntmap["Yamato Air Rave"] = 0x7A;
    vergiltauntmap["Lock-On provocation"] = 0x7C;
    vergiltauntmap["EX provocation"] = 0x7F;
    vergiltauntmap["BP provocation"] = 0x82;
}
std::optional<std::string> TauntSelector::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &TauntSelector::cheaton;
  m_on_page    = Page_Taunt;
  m_depends_on = { "PlayerTracker","StyleRank" };
  m_full_name_string     = "Taunt Selector (+)";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Allows you to specify what taunts will be used by each character.";

  set_up_hotkey();

  auto addr = m_patterns_cache->find_addr(base, "80 7F 20 00 48 8B CB 44");
  if (!addr) {
    return "Unable to find TauntSelector pattern.";
  }
  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 15)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize TauntSelector";
  }

  initialize_user_taunt_map();
  initialize_dev_taunt_map();
  return Mod::on_initialize();
}

// during load
void TauntSelector::on_config_load(const utility::Config &cfg) {
    use_dev_names = cfg.get<bool>("use_dev_names").value_or(false);
    for (int i = 0; i < 6; i++) {
        TauntSelector::nerotaunts[i] = cfg.get<int>("nerotaunts_" + std::to_string(i)).value_or(0);
        TauntSelector::dantetaunts[i] = cfg.get<int>("dantetaunts_" + std::to_string(i)).value_or(0);
        TauntSelector::vtaunts[i] = cfg.get<int>("vtaunts_" + std::to_string(i)).value_or(0);
        TauntSelector::vergiltaunts[i] = cfg.get<int>("vergiltaunts_" + std::to_string(i)).value_or(0);
        //this seems really dumb actually
        for (std::map<std::string, uint32_t>::iterator it = nerotauntmap.begin(); it != nerotauntmap.end(); ++it) {
            if (TauntSelector::nerotaunts[i] == it->second) {
                nerotauntstrings[i] = it->first;
            }
        }
        for (std::map<std::string, uint32_t>::iterator it = dantetauntmap.begin(); it != dantetauntmap.end(); ++it) {
            if (TauntSelector::dantetaunts[i] == it->second) {
                dantetauntstrings[i] = it->first;
            }
        }
        for (std::map<std::string, uint32_t>::iterator it = vtauntmap.begin(); it != vtauntmap.end(); ++it) {
            if (TauntSelector::vtaunts[i] == it->second) {
                vtauntstrings[i] = it->first;
            }
        }
        for (std::map<std::string, uint32_t>::iterator it = vergiltauntmap.begin(); it != vergiltauntmap.end(); ++it) {
            if (TauntSelector::vergiltaunts[i] == it->second) {
                vergiltauntstrings[i] = it->first;
            }
        }
    }
    
}
// during save
void TauntSelector::on_config_save(utility::Config &cfg) {
    cfg.set<bool>("use_dev_names",use_dev_names);
    for (int i = 0; i < 6; i++) {
        cfg.set<int>("nerotaunts_" + std::to_string(i), TauntSelector::nerotaunts[i]);
        cfg.set<int>("dantetaunts_" + std::to_string(i), TauntSelector::dantetaunts[i]);
        cfg.set<int>("vtaunts_" + std::to_string(i), TauntSelector::vtaunts[i]);
        cfg.set<int>("vergiltaunts_" + std::to_string(i), TauntSelector::vergiltaunts[i]);
        //cfg.set<const std::string&>("vergiltauntstrings_" + std::to_string(i), vergiltauntstrings[i]);
    }
    
}
// do something every frame
// void TauntSelector::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void TauntSelector::on_draw_debug_ui() {
    ImGui::Text("nero Air Taunt: %X", TauntSelector::nerotaunts[0]);
    ImGui::Text("nero Lock-on Taunt: %X", TauntSelector::nerotaunts[1]);
    ImGui::Text("nero S-rank Air Taunt: %X", TauntSelector::nerotaunts[2]);
    ImGui::Text("nero S-rank Lock-on Taunt: %X", TauntSelector::nerotaunts[3]);
    ImGui::Text("nero BP Taunt: %X", TauntSelector::nerotaunts[4]);
    ImGui::Text("nero BP Taunt: %X", TauntSelector::nerotaunts[5]);

  ImGui::Text("Vergil Air Taunt: %X", TauntSelector::vergiltaunts[0]);
  ImGui::Text("Vergil Lock-on Taunt: %X",TauntSelector::vergiltaunts[1]);
  ImGui::Text("Vergil S-rank Air Taunt: %X",TauntSelector::vergiltaunts[2]);
  ImGui::Text("Vergil S-rank Lock-on Taunt: %X",TauntSelector::vergiltaunts[3]);
  ImGui::Text("Vergil BP Taunt: %X", TauntSelector::vergiltaunts[4]);
  ImGui::Text("Vergil BP Taunt: %X", TauntSelector::vergiltaunts[5]);
}

// will show up in main window, dump ImGui widgets you want here

void TauntSelector::draw_combo_box(std::map<std::string, uint32_t> map, const char* label, uint32_t &value, std::string &combolabel) {
  float cursorX = ImGui::GetCursorPosX();
  
  size_t label_len = strlen(label);

  if (label[0] != '#' && label[1] != '#') {
	  char label_to_show[255];
	  memset(&label_to_show, '\0', sizeof(label_to_show));

	  for (size_t i = 0; i < label_len && !(label[i] == '#' && label[i + 1] == '#'); i++)
		  label_to_show[i] = label[i];

	  ImGui::Text(label_to_show);
	  cursorX += ImGui::CalcTextSize("X").x;
	  ImGui::SetCursorPos(ImVec2(cursorX, ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y));
  }
    
  auto def_text_col = ImGui::GetColorU32(ImGuiCol_Text);
  
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
  ImGui::PushStyleColor(ImGuiCol_Border, OUTLINE_GRAY);
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.93f, 0.93f, 1.00f));
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.07f, 0.07f, 0.07f, 1.00f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.05f, 0.11f, 0.20f, 1.00f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.05f, 0.11f, 0.20f, 1.00f));
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.05f, 0.11f, 0.20f, 1.00f));
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.05f, 0.11f, 0.20f, 1.00f));
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.26f, 0.59f, 0.98f, 0.00f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.05f, 0.11f, 0.20f, 1.00f));
  ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.07f, 0.07f, 0.07f, 1.00f));

  char internal_label[255];
  memset(&internal_label, '\0', sizeof(internal_label));
  memset(&internal_label, '#', 2);
  if (size_t len = label_len; len < (sizeof(internal_label) - 3))
	  memcpy(&internal_label[2], label, len);

  if (ImGui::BeginCombo(internal_label, combolabel.c_str()))
  {
    for (std::map<std::string, uint32_t>::iterator it = map.begin(); it != map.end(); ++it) {
      bool is_selected = (value == it->second); // You can store your selection however you
                        // want, outside or inside your objects

      bool pushed_def_text_color = false;
	  if (!is_selected) {
		  ImGui::PushStyleColor(ImGuiCol_Text, def_text_col);
		  pushed_def_text_color = true;
	  }

      if (ImGui::Selectable(it->first.c_str(), is_selected)) {
        value               = it->second;
        combolabel          = it->first;
      }

      if (is_selected) {
        ImGui::SetItemDefaultFocus(); // Set the initial focus when opening
                                      // the combo (scrolling + for keyboard
                                      // navigation support in the upcoming
                                      // navigation branch)
      }

	  if (pushed_def_text_color) {
		  ImGui::PopStyleColor();
	  }
    }
    ImGui::EndCombo();
  }

  ImGui::PopStyleColor(11);
  ImGui::PopStyleVar(1);
} 
void TauntSelector::on_draw_ui() { 
    ImGui::Checkbox("Use developer names", &use_dev_names);
    ImGui::ShowHelpMarker("Show internal names of taunts. Useful if you're making a taunt animation mod.");
    if (ImGui::CollapsingHeader("Nero Taunts"))
    {
        if (use_dev_names) {
            draw_combo_box(nerotauntmap_dev, "Air Taunt##nero", nerotaunts[0], nerotauntstrings[0]);
            draw_combo_box(nerotauntmap_dev, "Lock-on Taunt##nero", nerotaunts[1], nerotauntstrings[1]);
            draw_combo_box(nerotauntmap_dev, "S-Rank Air Taunt##nero", nerotaunts[2], nerotauntstrings[2]);
            draw_combo_box(nerotauntmap_dev, "S-Rank Lock-on Taunt##nero", nerotaunts[3], nerotauntstrings[3]);
            draw_combo_box(nerotauntmap_dev, "Bloody Palace Taunt##nero", nerotaunts[4], nerotauntstrings[4]);
            draw_combo_box(nerotauntmap_dev, "EX Taunt##nero", nerotaunts[5], nerotauntstrings[5]);
        }
        else {
            draw_combo_box(nerotauntmap, "Air Taunt##nero", nerotaunts[0], nerotauntstrings[0]);
            draw_combo_box(nerotauntmap, "Lock-on Taunt##nero", nerotaunts[1], nerotauntstrings[1]);
            draw_combo_box(nerotauntmap, "S-Rank Air Taunt##nero", nerotaunts[2], nerotauntstrings[2]);
            draw_combo_box(nerotauntmap, "S-Rank Lock-on Taunt##nero", nerotaunts[3], nerotauntstrings[3]);
            draw_combo_box(nerotauntmap, "Bloody Palace Taunt##nero", nerotaunts[4], nerotauntstrings[4]);
            draw_combo_box(nerotauntmap, "EX Taunt##nero", nerotaunts[5], nerotauntstrings[5]);
        }
        
    }
    if (ImGui::CollapsingHeader("Dante Taunts"))
    {
        if (use_dev_names) {
            draw_combo_box(dantetauntmap_dev, "Air Taunt##dante", dantetaunts[0], dantetauntstrings[0]);
            draw_combo_box(dantetauntmap_dev, "Lock-on Taunt##dante", dantetaunts[1], dantetauntstrings[1]);
            draw_combo_box(dantetauntmap_dev, "S-Rank Air Taunt##dante", dantetaunts[2], dantetauntstrings[2]);
            draw_combo_box(dantetauntmap_dev, "S-Rank Lock-on Taunt##dante", dantetaunts[3], dantetauntstrings[3]);
            draw_combo_box(dantetauntmap_dev, "Bloody Palace Taunt##dante", dantetaunts[4], dantetauntstrings[4]);
            draw_combo_box(dantetauntmap_dev, "EX Taunt##dante", dantetaunts[5], dantetauntstrings[5]);
        }else{
            draw_combo_box(dantetauntmap, "Air Taunt##dante", dantetaunts[0], dantetauntstrings[0]);
            draw_combo_box(dantetauntmap, "Lock-on Taunt##dante", dantetaunts[1], dantetauntstrings[1]);
            draw_combo_box(dantetauntmap, "S-Rank Air Taunt##dante", dantetaunts[2], dantetauntstrings[2]);
            draw_combo_box(dantetauntmap, "S-Rank Lock-on Taunt##dante", dantetaunts[3], dantetauntstrings[3]);
            draw_combo_box(dantetauntmap, "Bloody Palace Taunt##dante", dantetaunts[4], dantetauntstrings[4]);
            draw_combo_box(dantetauntmap, "EX Taunt##dante", dantetaunts[5], dantetauntstrings[5]);
        }
    }
    if (ImGui::CollapsingHeader("V Taunts"))
    {
        if (use_dev_names) {
            draw_combo_box(vtauntmap_dev, "Air Taunt##v", vtaunts[0], vtauntstrings[0]);
            draw_combo_box(vtauntmap_dev, "Lock-on Taunt##v", vtaunts[1], vtauntstrings[1]);
            draw_combo_box(vtauntmap_dev, "S-Rank Air Taunt##v", vtaunts[2], vtauntstrings[2]);
            draw_combo_box(vtauntmap_dev, "S-Rank Lock-on Taunt##v", vtaunts[3], vtauntstrings[3]);
            draw_combo_box(vtauntmap_dev, "Bloody Palace Taunt##v", vtaunts[4], vtauntstrings[4]);
            draw_combo_box(vtauntmap_dev, "EX Taunt##v", vtaunts[5], vtauntstrings[5]);
        }
        else {
            draw_combo_box(vtauntmap, "Air Taunt##v", vtaunts[0], vtauntstrings[0]);
            draw_combo_box(vtauntmap, "Lock-on Taunt##v", vtaunts[1], vtauntstrings[1]);
            draw_combo_box(vtauntmap, "S-Rank Air Taunt##v", vtaunts[2], vtauntstrings[2]);
            draw_combo_box(vtauntmap, "S-Rank Lock-on Taunt##v", vtaunts[3], vtauntstrings[3]);
            draw_combo_box(vtauntmap, "Bloody Palace Taunt##v", vtaunts[4], vtauntstrings[4]);
            draw_combo_box(vtauntmap, "EX Taunt##v", vtaunts[5], vtauntstrings[5]);
        }
        
    }
    if (ImGui::CollapsingHeader("Vergil Taunts"))
    {
        if (use_dev_names) {
            draw_combo_box(vergiltauntmap_dev, "Air Taunt##vergil", vergiltaunts[0], vergiltauntstrings[0]);
            draw_combo_box(vergiltauntmap_dev, "Lock-on Taunt##vergil", vergiltaunts[1], vergiltauntstrings[1]);
            draw_combo_box(vergiltauntmap_dev, "S-Rank Air Taunt##vergil", vergiltaunts[2], vergiltauntstrings[2]);
            draw_combo_box(vergiltauntmap_dev, "S-Rank Lock-on Taunt##vergil", vergiltaunts[3], vergiltauntstrings[3]);
            draw_combo_box(vergiltauntmap_dev, "Bloody Palace Taunt##vergil", vergiltaunts[4], vergiltauntstrings[4]);
            draw_combo_box(vergiltauntmap_dev, "EX Taunt##vergil", vergiltaunts[5], vergiltauntstrings[5]);
        }
        else {
            draw_combo_box(vergiltauntmap, "Air Taunt##vergil", vergiltaunts[0], vergiltauntstrings[0]);
            draw_combo_box(vergiltauntmap, "Lock-on Taunt##vergil", vergiltaunts[1], vergiltauntstrings[1]);
            draw_combo_box(vergiltauntmap, "S-Rank Air Taunt##vergil", vergiltaunts[2], vergiltauntstrings[2]);
            draw_combo_box(vergiltauntmap, "S-Rank Lock-on Taunt##vergil", vergiltaunts[3], vergiltauntstrings[3]);
            draw_combo_box(vergiltauntmap, "Bloody Palace Taunt##vergil", vergiltaunts[4], vergiltauntstrings[4]);
            draw_combo_box(vergiltauntmap, "EX Taunt##vergil", vergiltaunts[5], vergiltauntstrings[5]);
        }
    }
}


