#include "LandCancels.hpp"
#include "PlayerTracker.hpp"
//#include "ImGuiExtensions/ImGuiExtensions.h"

uintptr_t LandCancels::jmp_ret{NULL};
bool LandCancels::cheaton{NULL};
bool landCancelTestWeightToggle{ FALSE };
float desiredWeight{ 30.0f };
bool landCancelTestToggle{ FALSE };
uint32_t landCancelTestMove{ 8008 };

// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
    __asm {
        cmp byte ptr [LandCancels::cheaton], 1
        jne code
        //cmp byte ptr [landCancelTestWeightToggle], 1
        //push r11
        //push r12
        //jne cheatcode
        //mov r11, [PlayerTracker::playerentity]
        //mov r11, [r11+0x2C4]
        //mov r12, [desiredWeight]
        //mov [r11], r12d
        //pop r12
        //pop r11
        jmp cheatcode

    cheatcode:
        cmp byte ptr [landCancelTestToggle], 1
        jne cheatcodecont
        push r11
        mov r11d, [landCancelTestMove]
        cmp r11d, [PlayerTracker::playermoveid]
        pop r11
        je forceland
    cheatcodecont:
        cmp [PlayerTracker::playerid], 0
        je neromoves
        cmp [PlayerTracker::playerid], 1
        je dantemoves
        cmp byte ptr [rdx+0x8], 2
        je retcode
        jmp code

    neromoves:
        cmp [PlayerTracker::playermoveid], 0x03F20028 // Air Rave 1
        je forceland                                     
        cmp [PlayerTracker::playermoveid], 0x03FC0028 // Air Rave 2
        je forceland                                     
        cmp [PlayerTracker::playermoveid], 0x04060028 // Air Rave 3
        je forceland
        cmp [PlayerTracker::playermoveid], 0x02C6003C // Charge Shot Looking Up
        je chargeshottimer
        cmp [PlayerTracker::playermoveid], 0x02A8003C // Charge Shot Level
        je chargeshottimer
        cmp [PlayerTracker::playermoveid], 0x02D0003C // Charge Shot Looking Down
        je chargeshottimer
        cmp byte ptr [rdx+0x8], 2
        je retcode
        jmp code

    chargeshottimer:
        push r11
        push r12
        push r9
        // some kind of action and grounded check? only shows 1-4 when rbp = treelayer
        cmp rsi,rdi
        jne posttimer
        // hitch stuff: get layer offset
        mov r9, [rbp+0x1F80]
        and r9, 1
        imul r9, r9, 0xFB0
        // cmp dword ptr [rbp+r9+0xA8], 60 // Motion Bank
        // jne groundedcheck
        // cmp dword ptr [rbp+r9+0xB0], 700 // Motion ID
        // jne groundedcheck
        cmp dword ptr [rbp+r9+0xC8], 0x41600000 // PrevFrame // 14.0f
        ja popforceland
        jmp posttimer

    dantemoves:
        // cmp [PlayerTracker::playermoveid], 0x04B00002 // Ecstasy, works without this
        // je forceland
    // swords
        cmp [PlayerTracker::playermoveid], 0x157C00C9 // Air Rave 1 Reb
        je forceland
        cmp [PlayerTracker::playermoveid], 0x158600C9 // Air Rave 2 Reb
        je forceland
        cmp [PlayerTracker::playermoveid], 0x159000C9 // Air Rave 3 Reb
        je forceland
        cmp [PlayerTracker::playermoveid], 0x159A00C9 // Air Rave 4 Reb
        je forceland
        cmp [PlayerTracker::playermoveid], 0x145000FA // Air Rave 1 Reb
        je forceland
        cmp [PlayerTracker::playermoveid], 0x145A00FA // Air Rave 2 Reb
        je forceland
        cmp [PlayerTracker::playermoveid], 0x146400FA // Air Rave 3 Reb
        je forceland
        cmp [PlayerTracker::playermoveid], 0x146E00FA // Air Rave 4 Reb
        je forceland
        cmp [PlayerTracker::playermoveid], 0x157C00F1 // Air Rave 1 Reb
        je forceland
        cmp [PlayerTracker::playermoveid], 0x158600F1 // Air Rave 2 Reb
        je forceland
        cmp [PlayerTracker::playermoveid], 0x159000F1 // Air Rave 3 Reb
        je forceland
        cmp [PlayerTracker::playermoveid], 0x159A00F1 // Air Rave 4 Reb
        je forceland
    // guns
        cmp [PlayerTracker::playermoveid], 0x03F2012C // Air Shot
        je forceland
        cmp [PlayerTracker::playermoveid], 0x157C012C // Rainstorm
        je forceland
        cmp [PlayerTracker::playermoveid], 0x1586012C // Rainstorm Loop
        je forceland
        cmp [PlayerTracker::playermoveid], 0x159A012C // Rainstorm End
        je forceland
        cmp [PlayerTracker::playermoveid], 0x159A012C // Low Rainstorm End
        je forceland
        cmp byte ptr [rdx+0x8], 2
        je retcode
        jmp code

    posttimer:
        cmp byte ptr [rdx+0x8], 2
        je popret
        jmp popcode

    popcode:
        pop r9
        pop r12
        pop r11
    code:
        mov dword ptr [rdx+34h], 0 // can't land
    retcode:
        jmp qword ptr [LandCancels::jmp_ret]

    popret:
        pop r9
        pop r12
        pop r11
        jmp qword ptr [LandCancels::jmp_ret]

    popforceland:
        pop r9
        pop r12
        pop r11
    forceland:
        mov dword ptr [rdx+34h], 2 // can land
        jmp qword ptr [LandCancels::jmp_ret]
    }
}

// clang-format on

void LandCancels::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> LandCancels::on_initialize() {
  init_check_box_info();

  m_is_enabled = &LandCancels::cheaton;
  m_on_page    = mechanics;

  m_full_name_string = "Land Cancels";
  m_author_string    = "SSSiyan";
  m_description_string = "Touching the floor will cancel your current aerial attack.\n\n"
      "Only certain attacks are set to be land cancellable and this list will expand with time. Feel free to @ me with ideas!";

    auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  auto addr = patterns->find_addr(base, "C7 42 34 00 00 00 00 C3 CC CC 48");
  if (!addr) {
    return "Unable to find LandCancels pattern.";
  }

  if (!install_hook_absolute(addr.value(), m_function_hook, &detour, &jmp_ret, 7)) {
    //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize LandCancels";
  }
  return Mod::on_initialize();
}

void LandCancels::on_frame() {
    if (PlayerTracker::ingameplay) {
        if (landCancelTestWeightToggle) {
            (*(float*)(PlayerTracker::playerentity + 0x2C4)) = desiredWeight;
        }
    }
}

void LandCancels::on_draw_ui() {
    if (ImGui::CollapsingHeader("Debug")) {
        ImGui::TextWrapped("This will freeze your weight to your choice to make it easier to test land cancels.");
        ImGui::Checkbox("Land Cancel Weight Assist", &landCancelTestWeightToggle);
        if (landCancelTestWeightToggle) {
            ImGui::InputFloat("Desired Weight", &desiredWeight);
        }
        ImGui::Separator();
        ImGui::TextWrapped("This allows you test to see what a move would be like with land cancelling applied.");
        ImGui::Checkbox("Land Cancel Test", &landCancelTestToggle);
        if (landCancelTestToggle) {
            ImGui::TextWrapped("Pause the game while doing the move you want to see land cancellable, then press this button.");
            ImGui::TextWrapped("If the results are good, send me a message and I'll look at adding it to the list!");
            if (ImGui::Button("Enable Land Cancel For Current Move")) {
                landCancelTestMove = PlayerTracker::playermoveid;
            }
            ImGui::Text("Current MoveID: %i", PlayerTracker::playermoveid);
        }
    }
}
