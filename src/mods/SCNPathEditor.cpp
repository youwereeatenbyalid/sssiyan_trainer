
#include "SCNPathEditor.hpp"
#include "mods/PlayerTracker.hpp"
#include "mods/SpardaWorkshop.hpp"
#include "mods/LDK.hpp"
uintptr_t SCNPathEditor::jmp_ret{NULL};
uintptr_t SCNPathEditor::jmp_jne{ NULL };
bool SCNPathEditor::cheaton{NULL};
const char16_t SCNPathEditor::aimap[] = u"Scene/Location/Location**/Location22_AIMap.scn";
const char16_t SCNPathEditor::doujyo[] = u"Scene/Location/Location**/Environments/l22_01_doujyo.scn";
const char16_t SCNPathEditor::doujyoprop[] = u"Scene/Location/Location**/Environments/Props/l22_01_doujyo_Props*****.scn";
const char16_t SCNPathEditor::enemy[] = u"Scene/LevelDesign/Mission/Miss*****/M**_Enemy.scn";
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	    validation:
		jne jne_jmp
		add rcx, 0x68
		cmp r8, 0x2E
		je aimap
		cmp r8, 0x38
		je doujyo
		cmp r8, 0x49
		je doujyoprop
		cmp r8, 0x31
		je enemy
		jmp qword ptr [SCNPathEditor::jmp_ret]
 
        aimap:
			push rcx
			push r9
			lea rcx, [SCNPathEditor::aimap]
			lea r9, SCNPathEditor::stringcompare_detour
			call r9
			cmp rcx, 1
			pop r9
			pop rcx
			
			je workshopchoice
			jmp qword ptr [SCNPathEditor::jmp_ret]  

		doujyo:
			push rcx
			push r9
			lea rcx, [SCNPathEditor::doujyo]
			lea r9, SCNPathEditor::stringcompare_detour
			call r9
			cmp rcx, 1
			pop r9
			pop rcx
			je workshopchoice
			jmp qword ptr [SCNPathEditor::jmp_ret]

		doujyoprop:
			push rcx
			push r9
			lea rcx, [SCNPathEditor::doujyoprop]
			lea r9, SCNPathEditor::stringcompare_detour
			call r9
			cmp rcx, 1
			pop r9
			pop rcx
			je workshopchoice
			jmp qword ptr [SCNPathEditor::jmp_ret]

		enemy:
			push rcx
			push r9
			lea rcx, [SCNPathEditor::enemy]
			lea r9, SCNPathEditor::stringcompare_detour
			call r9
			cmp rcx, 1
			pop r9
			pop rcx
			je ldkchoice
			jmp qword ptr[SCNPathEditor::jmp_ret]


		workshopchoice:
			mov dword ptr [rdx+0x2E], 0x00320032
			cmp byte ptr [SpardaWorkshop::cheaton], 1
			jne ret_jmp
			mov dword ptr [rdx+0x2E], 0x00570053
			jmp qword ptr[SCNPathEditor::jmp_ret]

		ldkchoice:
			mov dword ptr [rdx+0x3E], 0x006E006F
			mov dword ptr [rdx+0x3A], 0x00690073 //sion
			cmp byte ptr [LDK::cheaton], 1
			jne ret_jmp
			mov dword ptr [rdx+0x3E], 0x004B0044
			mov dword ptr [rdx+0x3A], 0x004C0073 //sLDK
			jmp qword ptr [SCNPathEditor::jmp_ret]

		ret_jmp:
			jmp qword ptr [SCNPathEditor::jmp_ret]

		jne_jmp:
			jmp qword ptr [SCNPathEditor::jmp_jne]
	}
}
/// <summary>
/// How to call this thing: 
/// Maximum value is in r8
/// String address in rcx
/// Game address in rdx
/// 
/// iterator stored in rax
/// value stored in rbx
/// </summary>
/// <returns></returns>
naked void SCNPathEditor::stringcompare_detour(){
	__asm {
	start:
		push rax
		push rbx
		mov rax, 0xFFFFFFFFFFFFFFFF
		xor rbx, rbx

	loop:
		inc rax
		mov bx, [rcx+rax*2]
		cmp word ptr [rdx+rax*2], bx
		jne loopwildcard
		jmp loopcontinue

	loopwildcard:
		cmp rbx, 0x2A
		je loopcontinue
		jmp fail

	loopcontinue:
		cmp rax, r8
		jb loop
	success:
		mov rcx, 1
		pop rbx
		pop rax
		ret
	
	fail:
		mov rcx, 0
		pop rbx
		pop rax
		ret
	}
}
// clang-format on

void SCNPathEditor::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> SCNPathEditor::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &SCNPathEditor::cheaton;
  //onpage    = gamemode;
  m_full_name_string     = "SCNPathEditor Full Name";
  m_author_string        = "Author";
  m_description_string   = "This is the description of SCNPathEditor.";

  set_up_hotkey();

  auto addr = patterns->find_addr(base, "75 F5 48 83 C1 68");
  if (!addr) {
    return "Unable to find SCNPathEditor pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 6)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize SCNPathEditor";
  }
  SCNPathEditor::jmp_jne = addr.value()-9;
  return Mod::on_initialize();
}

// during load
void SCNPathEditor::on_config_load(const utility::Config &cfg) {}
// during save
void SCNPathEditor::on_config_save(utility::Config &cfg) {}
// do something every frame
void SCNPathEditor::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void SCNPathEditor::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void SCNPathEditor::on_draw_ui() {}

