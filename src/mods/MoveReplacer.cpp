
#include "MoveReplacer.hpp"
#include "mods/PlayerTracker.hpp"
#include "mods/Inertia.hpp"
uintptr_t MoveReplacer::jmp_ret{NULL};
bool MoveReplacer::cheaton{NULL};
uintptr_t MoveReplacer::filtercall{NULL};
uintptr_t MoveReplacer::startmovecall{NULL};
uintptr_t MoveReplacer::endmovecall{NULL};
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void detour() {
	__asm {
	validation:
        cmp [PlayerTracker::playerid], 0 //change this to the char number obviously
        je nerocode
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        je dantecode
        cmp [PlayerTracker::playerid], 1 //change this to the char number obviously
        je vcode
        cmp [PlayerTracker::playerid], 4 //change this to the char number obviously
        je vergilcode
        jmp moveswaporiginalcode
    nerocode:
        //neutral enemy step
        cmp dword ptr [r14], 0x522
        je enemystep

        //forward enemy step
        cmp dword ptr [r14], 0x4A3
        je enemystep

        //backward enemy step
        cmp dword ptr [r14], 0x53C
        je enemystep

        //left enemy step
        cmp dword ptr [r14], 0x535
        je enemystep

        //right enemy step
        cmp dword ptr [r14], 0x549
        je enemystep
        jmp moveswaporiginalcode
    dantecode:
        //jmp moveswaporiginalcode
        push rax
        mov rax, [MoveReplacer::filtercall]
        cmp [rsp+0x160], rax
        pop rax
        je moveswaporiginalcode

        //neutral enemy step
        cmp dword ptr [r14], 0x106
        je enemystep

        //forward enemy step
        cmp dword ptr [r14], 0x10D
        je enemystep

        //backward enemy step
        cmp dword ptr [r14], 0x148
        je enemystep

        //left enemy step
        cmp dword ptr [r14], 0x173
        je enemystep

        //right enemy step
        cmp dword ptr [r14], 0x15E
        je enemystep

        //Air Hike Back
        cmp dword ptr [r14], 0x12A
        je airhike
        //Air Hike Forward
        cmp dword ptr [r14], 0x136
        je airhike
        //Air Hike Left
        cmp dword ptr [r14], 0x153
        je airhike
        //Air Hike Right
        cmp dword ptr [r14], 0x167
        je airhike

        //Balrog Air Hike Back
        cmp dword ptr [r14], 0x12D
        je airhike
        //Balrog Air Hike Forward
        cmp dword ptr [r14], 0x0EC
        je airhike
        //Balrog Air Hike Left
        cmp dword ptr [r14], 0x139
        je airhike
        //Balrog Air Hike Right
        cmp dword ptr [r14], 0x156
        je airhike

        //cmp [r14], CDA
        //je guardfly
        jmp moveswaporiginalcode
    vcode:
        jmp moveswaporiginalcode
    vergilcode:
        //neutral enemy step
        cmp dword ptr [r14], 0x68
        je enemystep

        //forward enemy step
        cmp dword ptr [r14], 0x6E
        je enemystep

        //backward enemy step
        cmp dword ptr [r14], 0x8F
        je enemystep

        //left enemy step
        cmp dword ptr [r14], 0xA1
        je enemystep

        //right enemy step
        cmp dword ptr [r14], 0x98
        je enemystep
        jmp moveswaporiginalcode
    enemystep:
        //cmp [inertiatoggle], 1
        cmp byte ptr [Inertia::cheaton], 1
        jne moveswaporiginalcode

        push rax
        mov rax, [MoveReplacer::startmovecall]
        cmp [rsp+0x60], rax
        pop rax
        je inertiastore
        push rax
        mov rax, [MoveReplacer::endmovecall]
        cmp [rsp+0x60], rax
        pop rax
        je inertiawrite
        jmp moveswaporiginalcode


        airhike:        


        cmp byte ptr [Inertia::cheaton], 1
        jne moveswaporiginalcode

        push rax
        mov rax, [MoveReplacer::startmovecall]
        cmp [rsp+0x60], rax
        pop rax
        je startairhiketimer
        push rax
        mov rax, [MoveReplacer::endmovecall]
        cmp [rsp+0x60], rax
        pop rax
        je endairhiketimer
        jmp moveswaporiginalcode


        startairhiketimer:
        mov [Inertia::airhiketimer], 800
        jmp inertiastore

        endairhiketimer:
        mov [Inertia::airhiketimer], 0
        jmp inertiawrite

        

        inertiawrite:
        cmp [PlayerTracker::redirect], 1
        je inertiaredirect
        jmp inertiapreserve

        inertiastore:
            push r8
            call Inertia::store_detour
            pop r8
            jmp moveswaporiginalcode

        inertiapreserve:
            push r8
            call Inertia::preserve_detour
            pop r8
            jmp moveswaporiginalcode

        inertiaredirect:
            push r8
            call Inertia::redirect_detour
            pop r8
            jmp moveswaporiginalcode


    moveswaporiginalcode:
        mov ebx,[r14]
        mov rcx,r13
        jmp qword ptr [MoveReplacer::jmp_ret]
	}
}

// clang-format on

void MoveReplacer::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> MoveReplacer::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &MoveReplacer::cheaton;
  m_on_page    = -1;
  m_full_name_string     = "MoveReplacer";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Framework for animation replacement + inertia. Should be hidden in release.";

  set_up_hotkey();

  auto addr = m_patterns_cache->find_addr(base, "41 8B 1E 49 8B CD");
  auto filtercalladdr = m_patterns_cache->find_addr(base, "FE FE FF FF 48 83 C3 04 48 FF C7");
  auto startmovecalladdr = m_patterns_cache->find_addr(base, "49 8B 46 20 41 8B CC");
  auto endmovecalladdr   = m_patterns_cache->find_addr(base, "66 44 21 73 1A");
  MoveReplacer::filtercall = filtercalladdr.value();
  MoveReplacer::startmovecall = startmovecalladdr.value();
  MoveReplacer::endmovecall   = endmovecalladdr.value();
  if (!addr) {
    return "Unable to find MoveReplacer pattern.";
  }
  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 6)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize MoveReplacer";
  }
  return Mod::on_initialize();
}

// during load
// void MoveReplacer::on_config_load(const utility::Config &cfg) {}
// during save
// void MoveReplacer::on_config_save(utility::Config &cfg) {}
// do something every frame
// void MoveReplacer::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void MoveReplacer::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void MoveReplacer::on_draw_ui() {}