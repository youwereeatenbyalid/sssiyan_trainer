
#include "WeightReset.hpp"
#include "PlayerTracker.hpp"
uintptr_t WeightReset::jmp_ret{NULL};
bool WeightReset::cheaton{NULL};

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        // cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        // jne code

        cmp byte ptr [WeightReset::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        // Full House
        cmp byte ptr [rbx+110h], 50
        je write1FullHouse // 0

        // No Balrog
        cmp byte ptr [rbx+110h], 4
        je write1AirHike // 5
        cmp byte ptr [rbx+110h], 1
        je write1AirHikeForwards // 1
        cmp byte ptr [rbx+110h], 1
        je write1AirHikeBackwards // 1
        cmp byte ptr [rbx+110h], 1
        je write1AirHikeLeft // 1
        cmp byte ptr [rbx+110h], 1
        je write1AirHikeRight // 1

        // Balrog
        cmp byte ptr [rbx+110h], 5
        je write1AirHike // 5
        cmp byte ptr [rbx+110h], 1
        je write1AirHikeForwards // 1
        cmp byte ptr [rbx+110h], 0
        je write1AirHikeBackwards // 1
        cmp byte ptr [rbx+110h], 1
        je write1AirHikeLeft // 1
        cmp byte ptr [rbx+110h], 1
        je write1AirHikeRight // 1

        // DT
        cmp byte ptr [rbx+110h], 5
        je write1ThirdJump // 5
        cmp byte ptr [rbx+110h], 1
        je write1ThirdJumpForwards // 1
        cmp byte ptr [rbx+110h], 0
        je write1ThirdJumpBackwards // 1
        cmp byte ptr [rbx+110h], 0
        je write1ThirdJumpLeft // 1
        cmp byte ptr [rbx+110h], 0
        je write1ThirdJumpRight // 1
        jmp code

        write1FullHouse:
        cmp byte ptr [rbx+10h], 0 // Compare ResourceID, just in case
        je write1
        jmp code       // Write 1

        write1AirHike:
        cmp byte ptr [rbx+10h], 5
        je write1
        jmp code

        write1AirHikeForwards:
        cmp byte ptr [rbx+10h], 1
        je write1
        jmp code

        write1AirHikeBackwards:
        cmp byte ptr [rbx+10h], 1
        je write1
        jmp code

        write1AirHikeLeft:
        cmp byte ptr [rbx+10h], 1
        je write1
        jmp code

        write1AirHikeRight:
        cmp byte ptr [rbx+10h], 1
        je write1
        jmp code

        write1ThirdJump:
        cmp byte ptr [rbx+10h], 5
        je write1
        jmp code

        write1ThirdJumpForwards:
        cmp byte ptr [rbx+10h], 1
        je write1
        jmp code

        write1ThirdJumpBackwards:
        cmp byte ptr [rbx+10h], 1
        je write1
        jmp code

        write1ThirdJumpLeft:
        cmp byte ptr [rbx+10h], 1
        je write1
        jmp code

        write1ThirdJumpRight:
        cmp byte ptr [rbx+10h], 1
        je write1
        jmp code

        write1:
        mov byte ptr [rbx+24h], 1
        jmp cont
        jmp qword ptr [WeightReset::jmp_ret]

    code:
        mov [rbx+24h], al
    cont:
        mov rax, [rdx+38h]
		jmp qword ptr [WeightReset::jmp_ret]
	}
}

// clang-format on

void WeightReset::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> WeightReset::on_initialize() {
  init_check_box_info();

  m_is_enabled            = &WeightReset::cheaton;
  m_on_page               = mechanics;

  m_full_name_string     = "Reset Weight";
  m_author_string        = "SSSiyan";
  m_description_string   = "Air Hikes and Divekick will reset your weight";

  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "88 43 24 48 8B 42 38");
  if (!addr) {
    return "Unable to find WeightReset pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize WeightReset";
  }
  return Mod::on_initialize();
}

void WeightReset::on_config_load(const utility::Config& cfg) {
}

void WeightReset::on_config_save(utility::Config& cfg) {
}
