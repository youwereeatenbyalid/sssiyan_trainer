
#include "MovingTargetSwitch.hpp"

uintptr_t MovingTargetSwitch::jmp_ret{NULL};
bool MovingTargetSwitch::cheaton{NULL};
uintptr_t MovingTargetSwitch::jmp_jae{NULL};

float comissvalue = 0.75f;
float fixvalue = 2.0f;
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        cmp byte ptr [MovingTargetSwitch::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
		comiss xmm0, [fixvalue]
		jmp qword ptr [MovingTargetSwitch::jmp_ret]

    code:
        comiss xmm0, [comissvalue]
        jae jaejmp
        jmp qword ptr [MovingTargetSwitch::jmp_ret]

    jaejmp:
        jmp qword ptr [MovingTargetSwitch::jmp_jae]
	}
}

// clang-format on

void MovingTargetSwitch::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

//trying something out
void MovingTargetSwitch::on_sdk_init(){
    auto injecting_method =  sdk::find_method_definition("app.Player", "updateManualLockOn(System.Boolean)");
    uintptr_t method_address = injecting_method->get_function_t<uintptr_t>();
    uintptr_t addr = method_address + 0x1A8;
    uintptr_t addr_jae = method_address + 0x365;
    //auto addr_jae = addr.value() + 0x1BD;
    MovingTargetSwitch::jmp_jae = addr_jae /*.value() + 3*/;
    if (!install_new_detour(addr, m_detour, &detour, &jmp_ret, 7)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      //return "Failed to initialize MovingTargetSwitch";
    }
}

std::optional<std::string> MovingTargetSwitch::on_initialize() {
  init_check_box_info();

  m_is_enabled            = &MovingTargetSwitch::cheaton;
  m_on_page               = Page_Mechanics;

  m_full_name_string     = "Moving Target Switch";
  m_author_string        = "SSSiyan";
  m_description_string   = "Allows you to switch targets while moving the left stick.";

  //auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  ////
  //// 
  ////0F 2F 05 B1 DF F6 02
  ////app_Player__updateManualLockOn171428+0x1AF-0x7
  ////app_Player__updateManualLockOn171428+0x1A8
  //auto addr = m_patterns_cache->find_addr(base, "0F 2F 05 ? ? ? ? 0F 83 ? ? ? ? 48 83 7A ? ?");
  //

  ////app_Player__updateManualLockOn171428 + 0x368-0x3
  ////app_Player__updateManualLockOn171428 + 0x365
  ////addr + 1BD
  ////auto addr_jae = m_patterns_cache->find_addr(base, "3E 00 00 48 85 C9 75 12");
  ////if (!addr_jae) {
  ////    return "Unable to find MovingTargetSwitch jae pattern.";
  ////}
  //if (!addr) {
  //    return "Unable to find MovingTargetSwitch pattern.";
  //}

  //auto addr_jae = addr.value() + 0x1BD;



  //MovingTargetSwitch::jmp_jae = addr_jae /*.value() + 3*/;

  //if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 7)) {
  //  //  return a error string in case something goes wrong
  //  spdlog::error("[{}] failed to initialize", get_name());
  //  return "Failed to initialize MovingTargetSwitch";
  //}
  return Mod::on_initialize();
}

// void MovingTargetSwitch::on_draw_ui(){}
