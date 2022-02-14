#include "VergilSetMaxJJC.hpp"
uint32_t VergilSetMaxJJC::max_jjc{3};
uintptr_t VergilSetMaxJJC::jmp_ret{NULL};
bool VergilSetMaxJJC::cheaton{NULL};
bool VergilSetMaxJJC::sdtincrease{ false };
bool VergilSetMaxJJC::infinitejjdc{ false };
//clang-format off
static naked void max_jjc_detour() {
	__asm {
		cmp byte ptr [VergilSetMaxJJC::cheaton], 0
		je originalcode
		jmp cheat

		originalcode:
		mov eax, [rdi+0x48]
		jmp ret_jmp

		cheat:
		mov eax, dword ptr [VergilSetMaxJJC::max_jjc]
		cmp [VergilSetMaxJJC::sdtincrease], 1
		jne ret_jmp
		inc eax
		jmp ret_jmp

		ret_jmp://note: don't name labels as "exit" :D
		cmp [rcx+0x000018E0],eax
		jmp [VergilSetMaxJJC::jmp_ret]
  }
}
//clang-format on

void VergilSetMaxJJC::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

void VergilSetMaxJJC::on_config_load(const utility::Config& cfg) {
  max_jjc = cfg.get<uint32_t>("vergil_max_jjc").value_or(3);
  sdtincrease == cfg.get<bool>("vergil_sdt_jjdcincrease").value_or(false);
  infinitejjdc == cfg.get<bool>("vergil_infinite_jjdc").value_or(false);
}

void VergilSetMaxJJC::on_config_save(utility::Config& cfg) {
  cfg.set<uint32_t>("vergil_max_jjc", max_jjc);
  cfg.set<bool>("vergil_sdt_jjdcincrease", sdtincrease);
  cfg.set<bool>("vergil_infinite_jjdc", infinitejjdc);
}

std::optional<std::string> VergilSetMaxJJC::on_initialize() {
  init_check_box_info();
  m_is_enabled        = &VergilSetMaxJJC::cheaton;
  m_on_page           = vergilcheat;
  m_full_name_string = "Set Just Judgment Cut Limit(+)";
  m_author_string    = "VPZadov";
  m_description_string = "Set the maximum number of Just Judgment Cuts Vergil can perform in a row.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto init_addr = patterns->find_addr(base, "8B 47 48 39 81 E0 18 00 00");
  if (!init_addr)
    return "Unable to find VergilSetMaxJJC pattern.";

  if (!install_hook_absolute(init_addr.value(), m_setmaxjjc_hook, &max_jjc_detour, &jmp_ret, 9)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilSetMaxJJC";
  }
  return Mod::on_initialize();
}

void VergilSetMaxJJC::on_draw_ui() {
  ImGui::TextWrapped("JJdC limit:");
  UI::SliderInt("##Set max jjc slider", (int*)&max_jjc, 1, 20);
  ImGui::Checkbox("+1 to limit in SDT", (bool*)&sdtincrease);
  ImGui::Checkbox("Infinite JJdC's", (bool*)&infinitejjdc);
}

void VergilSetMaxJJC::on_draw_debug_ui() {}


