#include "VergilNoAfterimages.hpp"
#include "mods\VergilSDTFormTracker.hpp"
//clang-format off

uintptr_t VergilNoAfterimages::afterimages_jmp_ret{NULL};
uintptr_t VergilNoAfterimages::afterimages_jne{NULL};

uint32_t VergilNoAfterimages::vergilafterimage_state{0};

bool VergilNoAfterimages::cheaton{NULL};

static naked void afterimages_detour() {
    __asm {
        cmp byte ptr [VergilNoAfterimages::cheaton], 0
        je originalcode
        jmp cheat

        originalcode:
        cmp [rax+0x18],r15
        jne ret_jne
        mov rcx,rbx
        jmp qword ptr [VergilNoAfterimages::afterimages_jmp_ret]

        checkhideret:
        pop rax
        jmp checkstate

        cheat:
        cmp byte ptr [VergilNoAfterimages::isNoDrawIfObjHidden], 1
        je checkhide
        checkstate:
        cmp dword ptr [VergilNoAfterimages::vergilafterimage_state], 1
        je humanonly
        cmp dword ptr [VergilNoAfterimages::vergilafterimage_state], 2
        je sdtonly
        cmp dword ptr [VergilNoAfterimages::vergilafterimage_state], 3
        je all
        cmp dword ptr [VergilNoAfterimages::vergilafterimage_state], 0
        je originalcode

        humanonly:
        cmp dword ptr [VergilSDTFormTracker::vergilform_state], 2
        jne ret_jne
        jmp originalcode

        sdtonly:
        cmp dword ptr [VergilSDTFormTracker::vergilform_state], 2
        je ret_jne
        jmp originalcode

        checkhide:
        cmp qword ptr [PlayerTracker::vergilentity], 0
        je checkstate
        push rax
        mov rax, [PlayerTracker::vergilentity]
        mov rax, [rax+0x10]//GameObj
        test rax, rax
        je checkhideret
        mov al, [rax + 0x15]//DrawSelf
        cmp al, 0
        pop rax
        je ret_jne
        jmp checkstate

        all:
        jmp ret_jne

        ret_jne:
        jmp qword ptr [VergilNoAfterimages::afterimages_jne]
  }
}
//clang-format on

void VergilNoAfterimages::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> VergilNoAfterimages::on_initialize() {
  init_check_box_info();
  m_is_enabled        = &VergilNoAfterimages::cheaton;
  m_on_page           = Page_VergilVFXSettings;
  m_full_name_string = "Disable Afterimages (+)";
  m_author_string    = "V.P.Zadov";
  m_description_string = "Disable the afterimages Vergil leaves.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  auto init_addr = m_patterns_cache->find_addr(base, "4C 39 78 18 0F 85 03 03 00 00 48");//DevilMayCry5.exe+58B450 
  if (!init_addr) {
    return "Unanable to find VergilNoAfterimages pattern.";
  }

  VergilNoAfterimages::afterimages_jne = init_addr.value() + 0x30D;

  if (!install_hook_absolute(init_addr.value(), m_vergilafterimages_hook, &afterimages_detour, &afterimages_jmp_ret, 0xD)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilNoAfterimages"; 
  }
  return Mod::on_initialize();
}

void VergilNoAfterimages::on_config_load(const utility::Config& cfg) {
  vergilafterimage_state = cfg.get<uint32_t>("Vergil_Afterimage_State").value_or<uint32_t>(0);
  isNoDrawIfObjHidden = cfg.get<bool>("VergilNoAfterimages.isNoDrawIfObjHidden").value_or(true);
}

void VergilNoAfterimages::on_config_save(utility::Config& cfg) {
  cfg.set<uint32_t>("Vergil_Afterimage_State", vergilafterimage_state);
  cfg.set<bool>("VergilNoAfterimages.isNoDrawIfObjHidden", isNoDrawIfObjHidden);
}

void VergilNoAfterimages::on_draw_ui() {
  ImGui::Text("Set value to disable afterimages: 0 - only in human form, \n1 - only in SDT, 2 - in all forms; 3 - default.");

  ImGui::Combo("Disable Type", (int*)&vergilafterimage_state, "Default afterimages\0Only in Human Form\0Only in SDT\0All Forms\0");
  ImGui::Separator();
  ImGui::TextWrapped("Disable afterimages during tricks, dodges, jce etc. Use with \"Default afterimages\" if you want this by itself");
  ImGui::Checkbox("Disable trick/dodge afterimages", &isNoDrawIfObjHidden);
}

// void VergilNoAfterimages::on_draw_debug_ui() {}
