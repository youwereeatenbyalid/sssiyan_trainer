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
        movss [rbp+0x00001110],xmm4
        cmp byte ptr [VergilSDTAccumulateRework::cheaton], 1
        je cheat
        jmp qword ptr [VergilSDTAccumulateRework::dtchange_jmp_ret]

        cheat:
        movss [VergilSDTAccumulateRework::curDtValue], xmm4
        jmp cmpdtvals

        cmpdtvals:
        comiss xmm4, [VergilSDTAccumulateRework::prevDtValue]
        ja shouldincsdt
        movss [VergilSDTAccumulateRework::prevDtValue], xmm4
        mov byte ptr [VergilSDTAccumulateRework::isNeedToAddStdPoints], 0
        jmp qword ptr [VergilSDTAccumulateRework::dtchange_jmp_ret]
        
        shouldincsdt:
        mov byte ptr [VergilSDTAccumulateRework::isNeedToAddStdPoints], 1
        movss [VergilSDTAccumulateRework::prevDtValue], xmm4
        jmp qword ptr [VergilSDTAccumulateRework::dtchange_jmp_ret]
  }
}

static naked void sdtchange_detour() {
    __asm {
        cmp byte ptr [VergilSDTAccumulateRework::cheaton], 1
        je cheat
        mov edx, 0x00000002
        jmp qword ptr [VergilSDTAccumulateRework::sdtchange_jmp_ret]

        cheat:
        push rax
        mov rax, qword ptr [VergilSDTAccumulateRework::maxSdt]
        cmp [rbx+0x00001B20], rax
        jae setmaxsdt
        cmp byte ptr [VergilSDTAccumulateRework::isNeedToAddStdPoints], 1
        je sdtmaxcheck
        jmp ret_jmp

        setmaxsdt:
        mov [rbx+0x00001B20], rax
        jmp ret_jmp

        sdtmaxcheck:
        cmp [rbx+0x00001B20], rax
        jl incsdt
        jmp ret_jmp

        incsdt:
        movss xmm1, [rbx+0x00001B20]
        addss xmm1, [VergilSDTAccumulateRework::sdtPointsToAdd]
        movss [rbx+0x00001B20], xmm1
        jmp ret_jmp

        ret_jmp:
        pop rax
        mov byte ptr [VergilSDTAccumulateRework::isNeedToAddStdPoints], 0
        jmp qword ptr [VergilSDTAccumulateRework::sdtchange_jmp_ret]

  }
}
//clang-format on

void VergilSDTAccumulateRework::on_config_load(const utility::Config &cfg) {
  sdtPointsToAdd = cfg.get<float>("vergilSdtPointsToAdd").value_or(55.0);
}

void VergilSDTAccumulateRework::on_config_save(utility::Config &cfg) {
  cfg.set<float>("vergilSdtPointsToAdd", sdtPointsToAdd);
}

std::optional<std::string> VergilSDTAccumulateRework::on_initialize() {
  init_check_box_info();
  ischecked = &VergilSDTAccumulateRework::cheaton;
  onpage = vergilsdt;
  full_name_string = "SDT accumulate system rework (WIP?)(+)";
  author_string = "VPZadov";
  description_string = "Vergil will gain SDT points when he gains DT points.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  auto dtchange_addr = utility::scan(base, "F3 0F 11 A5 10 11 00 00");
  if (!dtchange_addr) {
    return "Unanable to find VergilSDTAccum::dtchange_addr pattern.";
  }

  auto sdtchange_addr = utility::scan(base, "BA 02 00 00 00 0F 5A");
  if (!sdtchange_addr) {
    return "Unanable to find VergilSDTAccum::sdtchange_addr pattern.";
  }

  if (!install_hook_absolute(dtchange_addr.value(), m_dtchange_hook, &dtchange_detour, &dtchange_jmp_ret, 8)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilSdtAccumulateRework::dtchange_detour()"; 
  }

  if (!install_hook_absolute(sdtchange_addr.value(), m_sdtchange_hook, &sdtchange_detour, &sdtchange_jmp_ret, 5)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilSdtAccumulateRework::sdtchange_detour()"; 
  }

  return Mod::on_initialize();
}

void VergilSDTAccumulateRework::on_draw_ui() {
  ImGui::TextWrapped("A completely buggy mod, mb "
                     "someday I'll fix this, IDK.\nDo not use this mod with "
                     "\"Infinite DT\" and \"Infinite SDT\" mods.");
  ImGui::Separator();
  ImGui::TextWrapped("Set num of SDT points, that Vergil will get with DT points:");
  ImGui::SliderFloat("##SDT Points Slider", &sdtPointsToAdd, 50.0, 180.0, "%.2f");

  if (VergilInfSDT::cheaton|| InfDT::cheaton)
    VergilSDTAccumulateRework::cheaton = false;
}

void VergilSDTAccumulateRework::on_draw_debug_ui() {}

void VergilSDTAccumulateRework::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
