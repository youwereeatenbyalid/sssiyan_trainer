#include "InfiniteTrickUp.hpp"

bool InfiniteTrickUp::cheaton{NULL};
uintptr_t InfiniteTrickUp::trickupRet{NULL};
uintptr_t InfiniteTrickUp::trickupJneRet{NULL};

static naked uintptr_t trickup_detour() {
	__asm {
		cmp byte ptr [InfiniteTrickUp::cheaton], 01
		je ret_jne

		originalcode:
		cmp byte ptr[rdi + 0x000019BC], 00
		jne ret_jne
		jmp qword ptr[InfiniteTrickUp::trickupRet]

		ret_jne:
		jmp qword ptr [InfiniteTrickUp::trickupJneRet]
	}
}

std::optional<std::string> InfiniteTrickUp::on_initialize()
{
	init_check_box_info();
	auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
	m_is_enabled = &cheaton;
	m_on_page = Page_VergilCheat;
	m_full_name_string = "Infinite Trick Up";
	m_author_string = "V.P.Zadov";
	m_description_string = "Allow vergil to infinitely trick up.";

  set_up_hotkey();

	auto trickUpAddr = m_patterns_cache->find_addr(base, "80 BF BC 19 00 00 00"); //DevilMayCry5.exe+550BA5
	if (!trickUpAddr) {
		return "Unanable to find InfiniteTrickUp.trickUpAddr pattern.";
	}

	trickupJneRet = trickUpAddr.value() + 0xD;

	if (!install_hook_absolute(trickUpAddr.value(), m_trickup_hook, &trickup_detour, &trickupRet, 0x7)) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize InfiniteTrickUp.trickUp";
	}

    return Mod::on_initialize();
}

// void InfiniteTrickUp::on_config_load(const utility::Config& cfg){}

// void InfiniteTrickUp::on_config_save(utility::Config& cfg){}

// void InfiniteTrickUp::on_frame(){}

// void InfiniteTrickUp::on_draw_ui(){}

// void InfiniteTrickUp::on_draw_debug_ui(){}

void InfiniteTrickUp::init_check_box_info() {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}
