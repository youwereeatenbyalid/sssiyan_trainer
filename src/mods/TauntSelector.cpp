#include "TauntSelector.hpp"
#include "mods/PlayerTracker.hpp"
#include "mods/StyleRank.hpp"
uintptr_t TauntSelector::jmp_ret{NULL};
bool TauntSelector::cheaton{NULL};
//Taunt order: Air, Lock-on, S-Rank Air, S-Rank Lockon, BP taunt, EX Taunt
uint32_t TauntSelector::nerotaunts[6]{0,0,0,0,0,0};
uint32_t TauntSelector::dantetaunts[6]{0,0,0,0,0,0};
uint32_t TauntSelector::vtaunts[6]{0,0,0,0,0,0};
uint32_t TauntSelector::vergiltaunts[6]{0,0,0,0,0,0};
std::map<std::string, uint32_t> nerotauntmap;
std::map<std::string, uint32_t> dantetauntmap;
std::map<std::string, uint32_t> vtauntmap;
std::map<std::string, uint32_t> vergiltauntmap;

std::string nerotauntstrings[6] = { "Default", "Default", "Default",
                                     "Default", "Default", "Default" };
std::string dantetauntstrings[6] = { "Default", "Default", "Default",
                                     "Default", "Default", "Default" };
std::string vtauntstrings[6] = { "Default", "Default", "Default",
                                     "Default", "Default", "Default" };
std::string vergiltauntstrings[6] = {"Default", "Default", "Default",
                                     "Default", "Default", "Default"};
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
        mov r8d,[rdi+0x10]
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

std::optional<std::string> TauntSelector::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  ischecked = &TauntSelector::cheaton;
  onpage    = commonpage;

  full_name_string     = "Taunt Selector (+)";
  author_string        = "The Hitchhiker";
  description_string   = "Allows you to specify what taunts will be used by each character.";

  auto addr = utility::scan(base, "80 7F 20 00 48 8B CB 44");
  if (!addr) {
    return "Unable to find TauntSelector pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 15)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize TauntSelector";
  }
  nerotauntmap["Default"] = 0x0;
  nerotauntmap["Throat cut"] = 0x6A;
  nerotauntmap["\"What'd you say?\""] = 0x6B;
  nerotauntmap["\"Come on!\""] = 0x6C;
  nerotauntmap["Gun fakeout"] = 0x6D;
  nerotauntmap["Sword Rev"] = 0x6E;
  nerotauntmap["\"I'll play with you\""] = 0x6F;
  //nerotauntmap["Sword Plant - Force Edge"] = 0x70;
  nerotauntmap["Air Guitar"] = 0x71;
  nerotauntmap["Clapping"] = 0x72;
  nerotauntmap["Well! What is it!"] = 0x73;
  nerotauntmap["Kneel"] = 0x74;
  nerotauntmap["Instant reload"] = 0x75;
  nerotauntmap["Breaker Taunt (unused)"] = 0x76;
  nerotauntmap["Hood Taunt"] = 0x77;
  nerotauntmap["Camera Taunt"] = 0x78;
  nerotauntmap["Air taunt"] = 0x79;
  nerotauntmap["S-Rank Lock-On provocation"] = 0x7C;
  nerotauntmap["Lock-On provocation"] = 0x7D;
  nerotauntmap["EX provocation"] = 0x7F;
  nerotauntmap["BP provocation"] = 0x83;
  nerotauntmap["BP provocation (Hoodless)"] = 0x82;
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
  vtauntmap["Mini pet"] = 0x6F;
  vtauntmap["\"You make me laugh\""] = 0x70;
  vtauntmap["Caprice 24 - Violin"] = 0x71;
  vtauntmap["Tap-Dance"] = 0x72;
  //vtauntmap["\"Man, I'm bored\""] = 0x73;
  vtauntmap["Pet Summon"] = 0x74;
  vtauntmap["Turn away"] = 0x75;
  vtauntmap["Air taunt"] = 0x79;
  vtauntmap["Lock-On provocation"] = 0x7C;
  vtauntmap["EX provocation"] = 0x7F;
  vtauntmap["BP provocation"] = 0x82;

  vergiltauntmap["Default"]                           = 0x0;
  vergiltauntmap["\"Show me your motivation!\""]      = 0x6A;
  vergiltauntmap["\"You think you stand a chance?\""] = 0x6B;
  vergiltauntmap["\"You're wasting my time.\""]       = 0x6C;
  vergiltauntmap["\"Disappointing.\""]                = 0x6D;
  vergiltauntmap["\"Let's have some fun!\""]          = 0x6E;
  vergiltauntmap["Sword Plant - Yamato"]              = 0x6F;
  vergiltauntmap["Sword Plant - Force Edge"]          = 0x70;
  vergiltauntmap["Book"]                              = 0x71;
  vergiltauntmap["Bringer"]                           = 0x72;
  vergiltauntmap["Warm up"]                           = 0x73;
  vergiltauntmap["Force Edge"]                        = 0x74;
  vergiltauntmap["\"Try Me.\""]                       = 0x75;
  vergiltauntmap["Kata"]                              = 0x76;
  vergiltauntmap["Summoned Sword Air Throw"]          = 0x79;
  vergiltauntmap["Yamato Air Rave"]                   = 0x7A;
  vergiltauntmap["Lock-On provocation"]               = 0x7C;
  vergiltauntmap["EX provocation"]                    = 0x7F;
  vergiltauntmap["BP provocation"]                    = 0x82;
  return Mod::on_initialize();
}

// during load
void TauntSelector::on_config_load(const utility::Config &cfg) {
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
    for (int i = 0; i < 6; i++) {
        cfg.set<int>("nerotaunts_" + std::to_string(i), TauntSelector::nerotaunts[i]);
        cfg.set<int>("dantetaunts_" + std::to_string(i), TauntSelector::dantetaunts[i]);
        cfg.set<int>("vtaunts_" + std::to_string(i), TauntSelector::vtaunts[i]);
        cfg.set<int>("vergiltaunts_" + std::to_string(i), TauntSelector::vergiltaunts[i]);
        //cfg.set<const std::string&>("vergiltauntstrings_" + std::to_string(i), vergiltauntstrings[i]);
    }
    
}
// do something every frame
void TauntSelector::on_frame() {}
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

void TauntSelector::draw_combo_box(std::map<std::string, uint32_t> map, const char* title, uint32_t &value, std::string &combolabel) {
  if (ImGui::BeginCombo(title, combolabel.c_str()))
  {
    for (std::map<std::string, uint32_t>::iterator it = map.begin(); it != map.end(); ++it) {
      bool is_selected = (value == it->second); // You can store your selection however you
                        // want, outside or inside your objects
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
    }
    ImGui::EndCombo();
  }
} 
void TauntSelector::on_draw_ui() { 
    if (ImGui::CollapsingHeader("Nero Taunts"))
    {
        draw_combo_box(nerotauntmap, "Air Taunt##nero", nerotaunts[0], nerotauntstrings[0]);
        draw_combo_box(nerotauntmap, "Lock-on Taunt##nero", nerotaunts[1], nerotauntstrings[1]);
        draw_combo_box(nerotauntmap, "S-Rank Air Taunt##nero", nerotaunts[2], nerotauntstrings[2]);
        draw_combo_box(nerotauntmap, "S-Rank Lock-on Taunt##nero", nerotaunts[3], nerotauntstrings[3]);
        draw_combo_box(nerotauntmap, "Bloody Palace Taunt##nero", nerotaunts[4], nerotauntstrings[4]);
        draw_combo_box(nerotauntmap, "EX Taunt##nero", nerotaunts[5], nerotauntstrings[5]);
    }
    if (ImGui::CollapsingHeader("Dante Taunts"))
    {
        draw_combo_box(dantetauntmap, "Air Taunt##dante", dantetaunts[0], dantetauntstrings[0]);
        draw_combo_box(dantetauntmap, "Lock-on Taunt##dante", dantetaunts[1], dantetauntstrings[1]);
        draw_combo_box(dantetauntmap, "S-Rank Air Taunt##dante", dantetaunts[2], dantetauntstrings[2]);
        draw_combo_box(dantetauntmap, "S-Rank Lock-on Taunt##dante", dantetaunts[3], dantetauntstrings[3]);
        draw_combo_box(dantetauntmap, "Bloody Palace Taunt##dante", dantetaunts[4], dantetauntstrings[4]);
        draw_combo_box(dantetauntmap, "EX Taunt##dante", dantetaunts[5], dantetauntstrings[5]);
    }
    if (ImGui::CollapsingHeader("V Taunts"))
    {
        draw_combo_box(vtauntmap, "Air Taunt##v", vtaunts[0], vtauntstrings[0]);
        draw_combo_box(vtauntmap, "Lock-on Taunt##v", vtaunts[1], vtauntstrings[1]);
        draw_combo_box(vtauntmap, "S-Rank Air Taunt##v", vtaunts[2], vtauntstrings[2]);
        draw_combo_box(vtauntmap, "S-Rank Lock-on Taunt##v", vtaunts[3], vtauntstrings[3]);
        draw_combo_box(vtauntmap, "Bloody Palace Taunt##v", vtaunts[4], vtauntstrings[4]);
        draw_combo_box(vtauntmap, "EX Taunt##v", vtaunts[5], vtauntstrings[5]);
    }
    if (ImGui::CollapsingHeader("Vergil Taunts"))
    {
    draw_combo_box(vergiltauntmap, "Air Taunt##vergil", vergiltaunts[0],vergiltauntstrings[0]);
    draw_combo_box(vergiltauntmap, "Lock-on Taunt##vergil", vergiltaunts[1], vergiltauntstrings[1]);
    draw_combo_box(vergiltauntmap, "S-Rank Air Taunt##vergil", vergiltaunts[2], vergiltauntstrings[2]);
    draw_combo_box(vergiltauntmap, "S-Rank Lock-on Taunt##vergil", vergiltaunts[3], vergiltauntstrings[3]);
    draw_combo_box(vergiltauntmap, "Bloody Palace Taunt##vergil", vergiltaunts[4], vergiltauntstrings[4]);
    draw_combo_box(vergiltauntmap, "EX Taunt##vergil", vergiltaunts[5], vergiltauntstrings[5]);
    }
}


