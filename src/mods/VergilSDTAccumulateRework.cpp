#include "VergilSDTAccumulateRework.hpp"

//clang-format off
float VergilSDTAccumulateRework::sdtPointsToAdd{55.0};
float VergilSDTAccumulateRework::curDtValue{0.0};
float VergilSDTAccumulateRework::prevDtValue{0.0};
bool VergilSDTAccumulateRework::isNeedToAddStdPoints{NULL};
bool VergilSDTAccumulateRework::cheaton{NULL};

uintptr_t VergilSDTAccumulateRework::dtchange_jmp_ret{NULL};
uintptr_t VergilSDTAccumulateRework::sdtchange_jmp_ret{NULL};

static naked void dtchange_detour() {
    __asm {
        cmp byte ptr [VergilSDTAccumulateRework::cheaton], 1
        je cheat

        originalcode:
        movss [rbp + 0x00001110], xmm4
        jmp qword ptr [VergilSDTAccumulateRework::dtchange_jmp_ret]

        cheat:
        cmp byte ptr [DMC3JCE::isJceRunning], 1
        je originalcode
        movss xmm5, [VergilSDTAccumulateRework::maxSdt]
        comiss xmm5, dword ptr [rbp + 0x00001B20]
        jbe originalcode
        mov ebx, dword ptr [rbp + 0x00001110]
        mov dword ptr [VergilSDTAccumulateRework::prevDtValue], ebx
        movss [VergilSDTAccumulateRework::curDtValue], xmm4
        movss [rbp + 0x00001110], xmm4

        cmpdtvals:
        comiss xmm4, [VergilSDTAccumulateRework::prevDtValue]
        ja shouldincsdt
        jmp qword ptr [VergilSDTAccumulateRework::dtchange_jmp_ret]
        
        shouldincsdt:
        cmp byte ptr [VergilSDTAccumulateRework::isConstInc], 1
        je const_inc
        movss xmm5, dword ptr [VergilSDTAccumulateRework::prevDtValue]
        subss xmm4, xmm5
        addss xmm4, dword ptr [rbp + 0x00001B20]
        movss dword ptr [rbp+0x00001B20], xmm4
        movss xmm4, dword ptr [rbp+0x00001110]
        jmp qword ptr [VergilSDTAccumulateRework::dtchange_jmp_ret]

        const_inc:
        movss xmm4, [rbp+0x00001B20]
        addss xmm4, dword ptr [VergilSDTAccumulateRework::sdtPointsToAdd]
        movss dword ptr [rbp+0x00001B20], xmm4
        movss xmm4, dword ptr [rbp+0x00001110]

        jmp qword ptr [VergilSDTAccumulateRework::dtchange_jmp_ret]
  }
}
//clang-format on

void VergilSDTAccumulateRework::on_config_load(const utility::Config &cfg) {
  isConstInc = cfg.get<bool>("VergilSDTAccumulateRework.isConstInc").value_or(false);
  sdtPointsToAdd = cfg.get<float>("vergilSdtPointsToAdd").value_or(55.0);
}

void VergilSDTAccumulateRework::on_config_save(utility::Config &cfg) {
  cfg.set<float>("vergilSdtPointsToAdd", sdtPointsToAdd);
  cfg.set<bool>("VergilSDTAccumulateRework.isConstInc", isConstInc);
}

std::optional<std::string> VergilSDTAccumulateRework::on_initialize() {
  init_check_box_info();
  ischecked = &VergilSDTAccumulateRework::cheaton;
  onpage = vergilsdt;
  full_name_string = "SDT accumulate system rework(+)";
  author_string = "VPZadov";
  description_string = "Vergil will gain SDT points when he gains DT points.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  auto dtchange_addr = utility::scan(base, "F3 0F 11 A5 10 11 00 00");
  if (!dtchange_addr) {
    return "Unanable to find VergilSDTAccum::dtchange_addr pattern.";
  }

  if (!install_hook_absolute(dtchange_addr.value(), m_dtchange_hook, &dtchange_detour, &dtchange_jmp_ret, 8)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilSdtAccumulateRework::dtchange_detour()"; 
  }

  return Mod::on_initialize();
}

void VergilSDTAccumulateRework::on_draw_ui() {
  ImGui::Checkbox("Add const custom amount of sdt", &isConstInc);
  if (isConstInc)
  {
      ImGui::TextWrapped("Set num of SDT points, that Vergil will get with DT points:");
      UI::SliderFloat("##SDT Points Slider", &sdtPointsToAdd, 50.0, 180.0, "%.2f");
  }
  if (VergilInfSDT::cheaton|| InfDT::cheaton)
    VergilSDTAccumulateRework::cheaton = false;
}

void VergilSDTAccumulateRework::on_draw_debug_ui() {}

void VergilSDTAccumulateRework::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
