#include "LDK.hpp"

// clang-format off
// only in clang/icl mode on x64, sorry
uintptr_t LDK::enemynumber_jmp_ret{NULL};
uintptr_t LDK::capbypass_jmp_ret1{NULL};
uintptr_t LDK::capbypass_jmp_ret2{NULL};
uintptr_t LDK::capbypass_jmp_jnl{NULL};
uintptr_t LDK::capbypass_jmp_jle{NULL};
uintptr_t LDK::cheaton{NULL};
uint32_t LDK::number{0};
uint32_t LDK::hardlimit{30};
uint32_t LDK::softlimit{20};
uint32_t LDK::limittype{0};
static naked void enemynumber_detour() {
	__asm {
		originalcode:
		mov eax,[rax+0x70]
		mov [rdi+0x00000750],eax
		mov [LDK::number], eax


		hardcheck:
		cmp eax, [LDK::hardlimit]
		jb softcheck

		mov [LDK::limittype], 2
		jmp ret_jmp

		softcheck:
		cmp eax, [LDK::softlimit]
		jb belowsoftcapacity

		mov [LDK::limittype], 1
		jmp ret_jmp

		belowsoftcapacity:
		mov [LDK::limittype], 0
		ret_jmp:
		jmp qword ptr[LDK::enemynumber_jmp_ret]
	}
}
// clang-format on
static naked void capbypass_detour1() {
__asm {
originalcode:
    push rax
    mov rax,[LDK::cheaton]
    cmp byte ptr [rax], 1
    pop rax
	je cheatcode
	jmp code
cheatcode:
	cmp byte ptr [LDK::limittype], 2
	je ret_jnl
	jmp ret_jmp
code:
	cmp eax,[rcx+0x30]
	jnl ret_jnl
	jmp ret_jmp
ret_jnl:
	jmp qword ptr [LDK::capbypass_jmp_jnl]
ret_jmp:
	jmp qword ptr [LDK::capbypass_jmp_ret1]
	}
}
static naked void capbypass_detour2() {
  __asm {
originalcode:
    push rax
    mov rax,[LDK::cheaton]
    cmp byte ptr [rax], 1
    pop rax
	je cheatcode
	jmp code
cheatcode:
	cmp byte ptr [LDK::limittype], 2
	je ret_jle
	jmp ret_jmp
code:
	cmp r14d,eax
	jle ret_jle
	jmp ret_jmp
ret_jle:
	jmp qword ptr [LDK::capbypass_jmp_jle]
ret_jmp:
	jmp qword ptr [LDK::capbypass_jmp_ret2]
  }
}
std::optional<std::string> LDK::on_initialize() {
  ischecked            = false;
  onpage               = commonpage;
  full_name_string     = "Legendary Dark Knights";
  author_string        = "The HitchHiker, Dr. Penguin, DeepDarkKapusta";
  description_string   = "Enables the Legendary Dark Knights Gamemode.";
  LDK::cheaton         = (uintptr_t)&ischecked;
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto enemynumber_addr = utility::scan(base, "8B 40 70 89 87 50 07 00 00");
  if (!enemynumber_addr) {
    return "Unable to find Enemy Number pattern.";
  }
  auto capbypass_addr1 = utility::scan(base, "3B 41 30 7D 12");
  if (!capbypass_addr1) {
    return "Unable to find Cap bypass 1 pattern.";
  }
  auto capbypass_addr2 = utility::scan(base, "44 3B F0 7E 52");
  if (!capbypass_addr2) {
    return "Unable to find Cap bypass 2 pattern.";
  }
  LDK::capbypass_jmp_jnl = capbypass_addr1.value() + 0x17;
  LDK::capbypass_jmp_jle = capbypass_addr2.value() + 0x57;


  if (!install_hook_absolute(enemynumber_addr.value(), m_enemynumber_hook, &enemynumber_detour,
                             &enemynumber_jmp_ret, 9)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize Enemy Number";
  }
  if (!install_hook_absolute(capbypass_addr1.value(), m_capbypass_hook1,
                             &capbypass_detour1, &capbypass_jmp_ret1, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize Cap bypass 1";
  }
  if (!install_hook_absolute(capbypass_addr2.value(), m_capbypass_hook2,
                             &capbypass_detour2, &capbypass_jmp_ret2, 5)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize Cap bypass 2";
  }
  return Mod::on_initialize();
}
// during load
void LDK::on_config_load(const utility::Config &cfg) {}
// during save
void LDK::on_config_save(utility::Config &cfg) {}
// do something every frame
//void LDK::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void LDK::on_draw_debug_ui() {
  ImGui::Text("Enemy Limit type is currently %X", LDK::limittype);
  ImGui::Text("Hard limit is: %X", LDK::hardlimit);
  ImGui::Text("Soft limit is: %X", LDK::softlimit);
  ImGui::Selectable("Select me?? IDK lol", true);
}

// will show up in main window, dump ImGui widgets you want here
void LDK::on_draw_ui() {
  ImGui::SliderInt("Enemy Hard Limit", (int*)&LDK::hardlimit, 1, 50);
  ImGui::SliderInt("Enemy Soft Limit", (int*)&LDK::softlimit, 1, 50);
}
