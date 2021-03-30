#include "VergilNoAfterimages.hpp"
#include "mods\VergilSDTFormTracker.hpp"

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

        cheat:
        cmp [VergilNoAfterimages::vergilafterimage_state], 0
        je humanonly
        cmp [VergilNoAfterimages::vergilafterimage_state], 1
        je sdtonly
        cmp [VergilNoAfterimages::vergilafterimage_state], 2
        je all

        humanonly:
        cmp [VergilSDTFormTracker::vergilform_state], 2
        jne ret_jne
        jmp originalcode

        sdtonly:
        cmp [VergilSDTFormTracker::vergilform_state], 2
        je ret_jne
        jmp originalcode

        all:
        jmp ret_jne

        ret_jne:
        jmp qword ptr [VergilNoAfterimages::afterimages_jne]
  }
}

void VergilNoAfterimages::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> VergilNoAfterimages::on_initialize() {
  init_check_box_info();
  ischecked        = &VergilNoAfterimages::cheaton;
  onpage           = vergilafterimages;
  full_name_string = "Disable Vergil's afterimages (+)";
  author_string    = "VPZadov";
  description_string = "Disable afterimages, that Vergil leaves after almost every move.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  auto init_addr = utility::scan(base, "4C 39 78 18 0F 85 03 03 00 00 48");
  if (!init_addr) {
    return "Unanable to find VergilNoAfterimages pattern.";
  }

  VergilNoAfterimages::afterimages_jne = init_addr.value() + 0x30D;

  if (!install_hook_absolute(init_addr.value(), m_vergilafterimages_hook, afterimages_detour, &afterimages_jmp_ret, 0xD)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilNoAfterimages"; 
  }
  return Mod::on_initialize();
}

void VergilNoAfterimages::on_config_load(const utility::Config& cfg) {
  vergilafterimage_state = cfg.get<uint32_t>("Vergil_Afterimage_State").value_or<uint32_t>(0);
}

void VergilNoAfterimages::on_config_save(utility::Config& cfg) {
  cfg.set<uint32_t>("Vergil_Afterimage_State", vergilafterimage_state);
}

void VergilNoAfterimages::on_draw_ui() {
  ImGui::Text("Set value to disable afterimages: 0 - only in human form, \n1 - only in SDT, 2 - in all forms.");
  ImGui::SliderInt("##Afterimages state slider", (int*)&vergilafterimage_state, 0, 2);
}

void VergilNoAfterimages::on_draw_debug_ui() {}
