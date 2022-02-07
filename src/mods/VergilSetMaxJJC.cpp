#include "VergilSetMaxJJC.hpp"
#include "VergilInfJdCs.hpp"

uint32_t VergilSetMaxJJC::max_jjc{3};
uintptr_t VergilSetMaxJJC::jmp_ret{NULL};
bool VergilSetMaxJJC::cheaton{NULL};

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
}

void VergilSetMaxJJC::on_config_save(utility::Config& cfg) {
  cfg.set<uint32_t>("vergil_max_jjc", max_jjc);
}

std::optional<std::string> VergilSetMaxJJC::on_initialize() {
  init_check_box_info();
  m_is_enabled        = &VergilSetMaxJJC::cheaton;
  m_on_page           = vergilcheat;
  m_full_name_string = "Set maximum JJC in a row (+)";
  m_author_string    = "VPZadov";
  m_description_string = "Set the maximum number of Just Judgement Cuts Vergil can perform in a row.";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto init_addr = utility::scan(base, "8B 47 48 39 81 E0 18 00 00");
  if (!init_addr)
    return "Unable to find VergilSetMaxJJC pattern.";

  if (!install_hook_absolute(init_addr.value(), m_setmaxjjc_hook, &max_jjc_detour, &jmp_ret, 9)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize VergilSetMaxJJC";
  }
  return Mod::on_initialize();
}

void VergilSetMaxJJC::on_draw_ui() {
  if (cheaton) {
    VergilInfJdCs::cheaton = false;
  }
  ImGui::TextWrapped("\"Infinite Just Judgement Cuts\" mod will be disabled if this mod active.");
  ImGui::TextWrapped("Set maximum jjc:");
  UI::SliderInt("##Set max jjc slider", (int*)&max_jjc, 1, 20);
}

void VergilSetMaxJJC::on_draw_debug_ui() {}


