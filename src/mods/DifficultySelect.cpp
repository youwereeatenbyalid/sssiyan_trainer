#include "DifficultySelect.hpp"
#include "mods/PlayerTracker.hpp"
//jumps
uintptr_t DifficultySelect::jmp_return1{NULL};
uintptr_t DifficultySelect::jmp_return2{NULL};
bool DifficultySelect::cheaton{NULL};
//variables
uintptr_t DifficultySelect::bpdifficulty{0};
uintptr_t DifficultySelect::missiondifficulty{0};
// clang-format off
// only in clang/icl mode on x64, sorry

  static naked void newmem1_detour() {
__asm {
  validation:
    cmp byte ptr [DifficultySelect::cheaton], 1
    je cheatcode
    jmp code
  code:
    mov [rax+0x0000008C],edi
    jmp qword ptr [DifficultySelect::jmp_return1]

  cheatcode:
    mov edi, [DifficultySelect::missiondifficulty]
    mov [rax+0x0000008C],edi
    jmp qword ptr [DifficultySelect::jmp_return1]

  }
}
  static naked void newmem2_detour() {
__asm {
  validation:
    cmp byte ptr [DifficultySelect::cheaton], 1
    je cheatcode
    jmp code
  code:
    mov [rcx+0x0000008C],edx
    jmp qword ptr [DifficultySelect::jmp_return2]

  cheatcode:
    mov edx, [DifficultySelect::bpdifficulty]
    mov [rcx+0x0000008C],edx
    jmp qword ptr [DifficultySelect::jmp_return2]

  }
}


// clang-format on

void DifficultySelect::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DifficultySelect::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  m_is_enabled = &DifficultySelect::cheaton;
  m_on_page    = Page_Enemies;
  m_full_name_string     = "Select Enemy Difficulty (+)";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Set the difficulty of enemies in combat.";

  set_up_hotkey();
  auto missionenemydifficulty_addr = m_patterns_cache->find_addr(base, "89 B8 8C 00 00 00");

  if (!missionenemydifficulty_addr) {
    return "Unable to find missionenemydifficulty pattern.";
  }
  if (!install_hook_absolute(missionenemydifficulty_addr.value(), m_missionenemydifficulty_hook, &newmem1_detour, &jmp_return1, 6)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize missionenemydifficulty";
  }
  auto bpenemydifficulty_addr = m_patterns_cache->find_addr(base, "89 91 8C 00 00 00 48 8B");

  if (!bpenemydifficulty_addr) {
    return "Unable to find bpenemydifficulty pattern.";
  }
  if (!install_hook_absolute(bpenemydifficulty_addr.value(), m_bpenemydifficulty_hook, &newmem2_detour, &jmp_return2, 6)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize bpenemydifficulty";
  }

  return Mod::on_initialize();
}

// during load
void DifficultySelect::on_config_load(const utility::Config &cfg) 
{
        bpdifficulty = cfg.get<int>("bloody palace difficulty").value_or(0);
        missiondifficulty = cfg.get<int>("mission difficulty").value_or(0);
}
// during save
void DifficultySelect::on_config_save(utility::Config &cfg)
{
    cfg.set<int>("bloody palace difficulty", bpdifficulty);
    cfg.set<int>("mission difficulty", missiondifficulty);
}
// do something every frame
// void DifficultySelect::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void DifficultySelect::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void DifficultySelect::on_draw_ui() {
  auto difficultystring =
      "Human\0Devil Hunter\0Son of Sparda\0Dante Must Die\0Heaven or Hell\0Hell "
      "and Hell\0";
  ImGui::Combo("Mission Difficulty", (int*)&DifficultySelect::missiondifficulty, difficultystring);
  ImGui::Combo("Bloody Palace Difficulty", (int*)&DifficultySelect::bpdifficulty, difficultystring);
}
