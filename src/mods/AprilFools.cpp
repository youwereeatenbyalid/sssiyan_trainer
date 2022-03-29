#include "AprilFools.hpp"

uintptr_t AprilFools::jmp_ret{NULL};
uintptr_t AprilFools::jmp_ret2{ NULL };
uintptr_t AprilFools::jmp_ret3{ NULL };
bool AprilFools::cheaton{NULL};


// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() { // force v for player select in normal menus.
	__asm {
		cmp byte ptr [AprilFools::cheaton], 1
		je cheatcode
		jmp code

	cheatcode:
		mov r8d, 2
		mov esi,r8d
		mov rdi,rdx
		jmp qword ptr [AprilFools::jmp_ret]

	code:
		mov esi,r8d
		mov rdi,rdx
		jmp qword ptr [AprilFools::jmp_ret]
	}
}

static naked void detour2() { // force v for player select in vergil mode menus.
	__asm {
		cmp byte ptr [AprilFools::cheaton], 1
		je cheatcode
		jmp code

	cheatcode:
		mov ecx, 0
		mov r12d, 2
		mov rdx,[rax+ 0x18]
		test rdx,rdx
		jmp qword ptr [AprilFools::jmp_ret2]

	code:
		mov rdx,[rax+ 0x18]
		test rdx,rdx
		jmp qword ptr [AprilFools::jmp_ret2]

	}
}

static naked void detour3() { // force v in actual player load
	__asm {
		cmp byte ptr [AprilFools::cheaton], 1
		je cheatcode
		jmp code

	cheatcode:
		mov r8d, 2
		mov [rax+0x10], r8d
		mov r9d,[rax+ 0x14]
		mov ecx,[rax+ 0x18]
		jmp qword ptr [AprilFools::jmp_ret3]
	code:
		mov r9d,[rax+ 0x14]
		mov ecx,[rax+ 0x18]
		jmp qword ptr [AprilFools::jmp_ret3]
	}
}


// clang-format on

void AprilFools::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> AprilFools::on_initialize() {
  init_check_box_info();
  
  m_is_enabled               = &AprilFools::cheaton;
  m_on_page                  = Page_None;
  m_full_name_string         = "AprilFools (+)";
  m_author_string            = "The Hitchhiker";
  m_description_string		 = "if you're seeing this we fucked up. forces character selection to always be v.";
  set_up_hotkey();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  uintptr_t staticbase = g_framework->get_module().as<uintptr_t>();
  auto addr = staticbase+0x422299;
  if (!addr) {
    return "Unable to find AprilFools pattern.";
  }
  if (!install_hook_absolute(addr, m_function_hook, &detour, &jmp_ret, 6)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize AprilFools";
  }

  auto addr2 = m_patterns_cache->find_addr(base, "48 8B 50 18 48 85 D2 0F 85 93 01 00 00 85");
  if (!addr2) {
    return "Unable to find AprilFools vergil pattern.";
  }
 
  if (!install_hook_absolute(addr2.value(), m_function_hook2, &detour2, &jmp_ret2, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize AprilFools";
  } 
 
  auto addr3 = m_patterns_cache->find_addr(base, "44 8B 48 14 8B 48 18 48 85 ED");

  if (!addr3) {
    return "Unable to find AprilFools mission pattern.";
  }

  if (!install_hook_absolute(addr3.value(), m_function_hook3, &detour3, &jmp_ret3, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize  AprilFools mission pattern";
  }

  return Mod::on_initialize();
}

void AprilFools::on_frame()
{
	{ // God does not have mercy on this date
		time_t now = time(0);
		tm* ltm = localtime(&now);

		if (ltm->tm_year == 2022 - 1900 && ltm->tm_mon == 3 && ltm->tm_mday == 1) {
			cheaton = true;
		}
	}
}

