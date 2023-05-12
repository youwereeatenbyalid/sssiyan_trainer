#include "VergilDisableSDTAccumulate.hpp"
//clang-format off
bool VergilDisableSDTAccumulate::cheaton{NULL};
uintptr_t VergilDisableSDTAccumulate::jmp_ret{NULL};
uintptr_t VergilDisableSDTAccumulate::func_addr{NULL};

static naked void accumfunc_detour() {
	__asm {
		cmp byte ptr [VergilDisableSDTAccumulate::cheaton], 0
		je originalcode
		jmp skip

		originalcode:
		call qword ptr [VergilDisableSDTAccumulate::func_addr]
		jmp skip

		skip:
		//movzx edx,al
		jmp qword ptr [VergilDisableSDTAccumulate::jmp_ret]
  }


}

//clang-format on
void VergilDisableSDTAccumulate::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> VergilDisableSDTAccumulate::on_initialize() {
  init_check_box_info();
  m_is_enabled          = &VergilDisableSDTAccumulate::cheaton;
  m_on_page             = Page_VergilSDT;
  m_full_name_string   = "Disable SDT Accumulate";
  m_author_string      = "V.P.Zadov";
  m_description_string = "Disable the \"accumulate\" skill. Works best with the "
                       "accumulate system rework.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  auto init_addr = m_patterns_cache->find_addr(base, "E8 ? ? ? ? 0F B6 D0 48 8B 43 50 48 8B 48 18 48 85 C9 0F 85 ? ? ? ? 85 D2 0F 85 ? ? ? ? 4C 8B 05 ? ? ? ? 48 8B D7");
  if (!init_addr) {
    return "Unanable to find VergilDisableSDTAccumulate::init_addr pattern.";
  }

  //app.GameModel.isTag(System.UInt32, System.UInt32)
  //tu6 aob C3 CC CC CC 48 89 6C 24 18 57 41 +0x4
  //tu7 aob 5F C3 CC 48 89 6C 24 18 57 41 +0x3
  auto func_addr_temp = m_patterns_cache->find_addr(base, "5F C3 CC 48 89 6C 24 18 57 41");
  if (!func_addr_temp) {
    return "Unanable to find VergilDisableSDTAccumulate::func_addr pattern.";
  }
  VergilDisableSDTAccumulate::func_addr = func_addr_temp.value() + 0x3;

  if (!install_new_detour(init_addr.value(), m_accumulatefunc_detour, &accumfunc_detour, &jmp_ret, 5)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilDisableSDTAccumulate"; 
  }

  return Mod::on_initialize();
}

// void VergilDisableSDTAccumulate::on_config_load(const utility::Config& cfg) {}

// void VergilDisableSDTAccumulate::on_config_save(utility::Config& cfg) {}

// void VergilDisableSDTAccumulate::on_draw_ui() {}

// void VergilDisableSDTAccumulate::on_draw_debug_ui() {}
