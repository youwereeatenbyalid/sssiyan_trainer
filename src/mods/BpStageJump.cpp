
#include "BpStageJump.hpp"
#include "PlayerTracker.hpp"

uintptr_t BpStageJump::jmp_ret{NULL};
uintptr_t BpStageJump::jmp_jne{NULL};
bool BpStageJump::cheaton{NULL};
int bpstage = 1;
bool bossrush;
bool retrystage;

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
        // cmp byte ptr [PlayerTracker::playerid], 1 //change this to the char number obviously
        // jne code
		cmp byte ptr [BpStageJump::cheaton], 1
        je cheatcode
        jmp code

    cheatcode:
        cmp byte ptr [bossrush], 1
        je bossrushstart
        push rax
        mov eax, [bpstage]
        mov [rbx+68h], eax
        mov rax, [rdi+50h]
        pop rax
        // cmp byte ptr [retrystage], 1
        // je retcode
        // mov byte ptr [BpStageJump::cheaton], 00
        jmp retcode

    bossrushstart:
        cmp byte ptr [rbx+68h], 00
        je stage20
        cmp byte ptr [rbx+68h], 20
        je stage40
        cmp byte ptr [rbx+68h], 40
        je stage60
        cmp byte ptr [rbx+68h], 60
        je stage80
        cmp byte ptr [rbx+68h], 80
        je stage90
        cmp byte ptr [rbx+68h], 90
        je stage98
        jmp code

    stage20:
        mov byte ptr [rbx+68h], 20
        jmp retcode
    stage40:
        mov byte ptr [rbx+68h], 40
        jmp retcode
    stage60:
        mov byte ptr [rbx+68h], 60
        jmp retcode
    stage80:
        mov byte ptr [rbx+68h], 80
        jmp retcode
    stage90:
        mov byte ptr [rbx+68h], 90
        jmp retcode
    stage98:
        mov byte ptr [rbx+68h], 98
        jmp retcode

    code:
        cmp [rbx+79h], cl
        jne jmp_jne
        inc byte ptr [rbx+68h]
    retcode:
        mov rax, [rdi+50h]
        jmp qword ptr [BpStageJump::jmp_ret]

    jmp_jne:
        jmp qword ptr [BpStageJump::jmp_jne]
	}
}

// clang-format on

std::optional<std::string> BpStageJump::on_initialize() {
  ischecked               = &BpStageJump::cheaton;
  onpage                  = commonpage;
  full_name_string        = "Bp Stage Jump (+)";
  author_string           = "SSSiyan";
  description_string      = "Allows you to skip to a BP stage of your choosing.";

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = utility::scan(base, "38 4B 79 75 03");
  if (!addr) {
    return "Unable to find BpStageJump pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 12)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize BpStageJump";
  }

  BpStageJump::jmp_jne = addr.value() + 8;

  return Mod::on_initialize();
}

void BpStageJump::on_config_load(const utility::Config& cfg) {
  bossrush   = cfg.get<bool>("bp_jump_boss_rush").value_or(false);
  // retrystage = cfg.get<bool>("bp_jump_retry_stage").value_or(false);
}
void BpStageJump::on_config_save(utility::Config& cfg) {
  cfg.set<bool>("bp_jump_boss_rush", bossrush);
  // cfg.set<bool>("bp_jump_retry_stage", retrystage);
}

void BpStageJump::on_draw_ui() {
    ImGui::Checkbox("Boss Rush", &bossrush);
    // ImGui::Checkbox("Retry Stage", &retrystage);
    ImGui::SliderInt("BP Stage", &bpstage, 1, 101);
}
