
#include "DisableBreakaway.hpp"
#include "mods/PlayerTracker.hpp"
#include "mods/GameInput.hpp"
uintptr_t DisableBreakaway::jmp_ret{NULL};
bool DisableBreakaway::cheaton{NULL};
uint32_t inputs[] = {Mod::sword,Mod::gun,Mod::jump,Mod::tauntinput,Mod::lockon,Mod::changetarget,Mod::dpad,Mod::deviltrigger,Mod::dpadup,Mod::dpaddown,Mod::dpadleft,Mod::dpadright,Mod::style,Mod::righttrigger,Mod::lefttrigger,Mod::resetcamera,Mod::SDT};
// clang-format off
// only in clang/icl mode on x64, sorry
uint32_t DisableBreakaway::overrideinput{0x1000};
int inputindex{0};
static naked void detour() {
	__asm {
	    validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        jne code
        cmp byte ptr [DisableBreakaway::cheaton], 1
        je cheatcode
        jmp code
     code:
        cmp qword ptr [rax+0x18],00
        jmp qword ptr [DisableBreakaway::jmp_ret]        
     cheatcode:
        push r8
        mov r8, GameInput::holdframes[0*8]
        or r8, GameInput::holdframes[1*8]
        or r8, GameInput::holdframes[2*8]
        or r8, GameInput::holdframes[3*8]
        or r8, GameInput::holdframes[4*8]
        or r8, GameInput::holdframes[5*8]
        test r8, [DisableBreakaway::overrideinput]
        pop r8
        ja code
		cmp rcx,00
        jmp qword ptr [DisableBreakaway::jmp_ret]
	}
}

// clang-format on

void DisableBreakaway::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DisableBreakaway::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  ischecked = &DisableBreakaway::cheaton;
  onpage    = breaker;

  full_name_string     = "Disable Breakaway";
  author_string        = "Lidemi & The Hitchhiker";
  description_string   = "Disables the Breakaway animation."
                         "Hold the assigned input while pressing breakaway to force a breakaway.";

  auto addr = utility::scan(base, "48 83 78 18 00 0F 85 7E 01 00 00 48 B8");
  if (!addr) {
    return "Unable to find DisableBreakaway pattern.";
  }
  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 5)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize DisableBreakaway";
  }
  return Mod::on_initialize();
}

// during load
void DisableBreakaway::on_config_load(const utility::Config &cfg) {
    inputindex = cfg.get<int>("breakaway index").value_or(0);
}
// during save
void DisableBreakaway::on_config_save(utility::Config &cfg) {
    cfg.set<int>("breakaway index",inputindex);
}
// do something every frame
//void DisableBreakaway::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void DisableBreakaway::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void DisableBreakaway::on_draw_ui() {
    if (ImGui::Combo("Override button", &inputindex,
        "Sword\0Gun\0Jump\0Taunt\0"
        "Lock-on\0Change Target\0Breakaway\0Devil Trigger\0"
        "Dpad Up\0Dpad Down\0Dpad Left\0Dpad Right\0"
        "Breaker Action\0Buster\0Exceed\0Reset Camera\0")) {
        overrideinput = inputs[inputindex];
    }
}
