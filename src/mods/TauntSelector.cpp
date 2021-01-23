#include "TauntSelector.hpp"
#include "mods/PlayerTracker.hpp"
#include "mods/StyleRank.hpp"
uintptr_t TauntSelector::jmp_ret{NULL};
uintptr_t TauntSelector::cheaton{NULL};
//Taunt order: Air, Lock-on, S-Rank Air, S-Rank Lockon, BP taunt, EX Taunt
uint32_t TauntSelector::nerotaunts[6]{0,0,0,0,0,0};
uint32_t TauntSelector::dantetaunts[6]{0,0,0,0,0,0};
uint32_t TauntSelector::vtaunts[6]{0,0,0,0,0,0};
uint32_t TauntSelector::vergiltaunts[6]{0,0,0,0,0,0};
std::map<std::string, uint32_t> vergiltauntmap;
std::string vergiltauntstrings[6] = {"Default", "Default", "Default",
                                     "Default", "Default", "Default"};
    // clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	validation:
        push rax
        mov rax, [TauntSelector::cheaton]
        cmp byte ptr [rax], 1
        pop rax
        jne code
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        je vergilcode
        jmp code
    code:
        mov r8d,[rdi+0x10]
        setne r9b
        jmp qword ptr [TauntSelector::jmp_ret]        

    vergilcode:
		mov r8d,[rdi+0x10]
        setne r9b

        push r10
        push r11
        lea r11, [TauntSelector::vergiltaunts]
        mov r10, 5
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
        jae vergilsrankcode
        sub r10, 2
        //lock-on 1
        cmp r8d, 0x7C
        je overridetaunt
        dec r10
        //air taunt 0
        cmp r8d, 0x79
        je overridetaunt
        jmp ret_jmp
    vergilsrankcode:
        //lock-on s-rank 3
        cmp r8d, 0x7C
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
  ischecked = false;
  onpage    = commonpage;
  full_name_string     = "Taunt Selector";
  author_string        = "The Hitchhiker";
  description_string   = "Allows you to specify what taunts will be used by each character.";
  TauntSelector::cheaton = (uintptr_t)&ischecked;

  auto addr = utility::scan(base, "44 8B 47 10 41 0F 95 C1 4C");
  if (!addr) {
    return "Unable to find TauntSelector pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 8)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize TauntSelector";
  }
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
void TauntSelector::on_config_load(const utility::Config &cfg) {}
// during save
void TauntSelector::on_config_save(utility::Config &cfg) {}
// do something every frame
void TauntSelector::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void TauntSelector::on_draw_debug_ui() {
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
    ImGui::Text("Vergil Taunts");
    draw_combo_box(vergiltauntmap, "Air Taunt", vergiltaunts[0],vergiltauntstrings[0]);
    //ImGui::SameLine();
    draw_combo_box(vergiltauntmap, "Lock-on Taunt", vergiltaunts[1], vergiltauntstrings[1]);
    //ImGui::SameLine();
    
    draw_combo_box(vergiltauntmap, "S-Rank Air Taunt", vergiltaunts[2], vergiltauntstrings[2]);
    //ImGui::SameLine();
    draw_combo_box(vergiltauntmap, "S-Rank Lock-on Taunt", vergiltaunts[3], vergiltauntstrings[3]);
    //ImGui::SameLine();
    draw_combo_box(vergiltauntmap, "Bloody Palace Taunt", vergiltaunts[4], vergiltauntstrings[4]);
    draw_combo_box(vergiltauntmap, "EX Taunt", vergiltaunts[5], vergiltauntstrings[5]);

}


