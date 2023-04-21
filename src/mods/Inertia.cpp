
#include "Inertia.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t Inertia::jmp_ret{NULL};
bool Inertia::cheaton{NULL};
float backupxinertia = 0.0f;
float backupzinertia = 0.0f;
float backupinertia  = 0.0f;
float inertiamult    = 0.8f;
uint32_t Inertia::airhiketimer = 0;
// clang-format off
// only in clang/icl mode on x64, sorry
static naked void detour() {
	__asm {
        movss dword ptr [backupinertia], xmm13
        movss dword ptr [backupxinertia], xmm13
        movss dword ptr [backupzinertia], xmm13
        code:
        mov rax,[rcx+0x08]
        mov r8d,[rax+0x78]
        jmp qword ptr [Inertia::jmp_ret]        
	}
}

naked void Inertia::store_detour() {
	__asm {
           //move the x inertia into XMM13
           mov r8, [PlayerTracker::playerinertiax]

           test r8, r8
           je inertiastoresafety

           movss xmm13, dword ptr [r8]
           //move the z inertia into XMM14
           mov r8, [PlayerTracker::playerinertiaz]
           movss xmm14, dword ptr [r8]


               mulss xmm13, [inertiamult] // I was thinking about making this an option but might just force it sue me
               mulss xmm14, [inertiamult]
           movss [backupxinertia], xmm13
           movss [backupzinertia], xmm14

           //square x and z, sum them, and move the square root of the sum into XMM14
           mulss xmm13, xmm13
           mulss xmm14, xmm14
           addss xmm13, xmm14
           sqrtss xmm14, xmm13
           //move the composite inertia into xmm14
           movss dword ptr [backupinertia], xmm14
        inertiastoresafety:
           //clear xmm13 and xmm14
           movss xmm13, xmm15
           movss xmm14, xmm15
        ret
	}
}

naked void Inertia::preserve_detour() {
	__asm {
inertiapreserve:
           //move the x and z values into xmm13 and xmm14
           movss xmm13, [backupxinertia]
           movss xmm14, [backupzinertia]
           //move them into their respective addresses
           mov r8, [PlayerTracker::playerinertiax]
           test r8, r8
           je inertiapreservesafety

           movss dword ptr [r8],xmm13
           mov r8, [PlayerTracker::playerinertiaz]
           movss dword ptr [r8],xmm14

        inertiapreservesafety:
           //clear the xmm13 and xmm14 registers
           movss xmm13, xmm15
           movss xmm14, xmm15
           ret
	}
}

naked void Inertia::redirect_detour() {
	__asm {
inertiaredirect:
           //move the composite inertia into xmm14
           movss xmm14, [backupinertia]
           //move the sin value into xmm13
           movss xmm13, dword ptr [PlayerTracker::sinvalue]
           //multiply the two together to get the new X inertia
           mulss xmm13, xmm14
           //move the new value into the X inertia address
           mov r8, [PlayerTracker::playerinertiax]
           test r8, r8
           je inertiaredirectsafety

           movss dword ptr [r8],xmm13
           //move the cos value into xmm13
           movss xmm13, [PlayerTracker::cosvalue]
           //multiply the two together to get the new X inertia
           mulss xmm13, xmm14
           //move the new value into the X inertia address
           mov r8, [PlayerTracker::playerinertiaz]
           movss dword ptr [r8],xmm13

        inertiaredirectsafety:
           //clear the xmm13 and xmm14 registers
           movss xmm13, xmm15
           movss xmm14, xmm15
           ret
	}
}

// siy stuff
/*
uintptr_t Inertia::reduce_rate_jmp_ret{NULL};
float reduceRate_FullPercent = 1.0f;
float reduceRate_SeventyPercent = 0.7f;
float reduceRate_FiftyPercent = 0.5f;
float reduceRate_ThirtyPercent = 0.3f;
bool Inertia::custom_inertia = false;
// inertiaReduceRate takes existing inertia and multiplies it by x
// dmc4 clamps the first move you do (other than guard), which this should also ideally do if we have air attack count
// "DevilMayCry5.exe"+51821D
naked void Inertia::inertia_reduce_rate_detour() {
    __asm {
        cmp byte ptr [Inertia::custom_inertia], 0
        je code

        cmp dword ptr [rdi+0xE64], 1
        je playerDante
        jmp code

    playerDante:
        cmp dword ptr [rdi+0x2C4], 0 // weight, see if this is first attack
        je code // clamps first attack to intended value

        // cmp dword ptr [rdi+18B0], 0 // sword id
        // je rebellionAttacks
        // 2 seems to be used for freefall too :(
        cmp r8d, 0x3 // rave 2
        je fullInertia
        cmp r8d, 0x4 // rave 3
        je fullInertia
        // cmp dword ptr [rdi+18D4], 0 // gun id
        // je ebonyIvoryAttacks
        cmp r8d, 0x18 // normal shot
        je fiftyInertia
        cmp r8d, 0x19 // gunslinger shot
        je fiftyInertia
        // cmp dword ptr [rdi+18D4], 1 // gun id
        // je shotgunAttacks
        cmp r8d, 0x1C // shotty shot
        je fiftyInertia
        // cmp dword ptr [rdi+18D4], 5 // gun id
        // je hatAttacks
        cmp r8d, 0x22 // hatGatling1
        je fiftyInertia
        jmp code

    fullInertia:
        mov eax, [reduceRate_FullPercent]
        jmp code

    seventyInertia:
        mov eax, [reduceRate_SeventyPercent]
        jmp code

    fiftyInertia:
        mov eax, [reduceRate_FiftyPercent]
        jmp code

    thirtyInertia:
        mov eax, [reduceRate_ThirtyPercent]
        jmp code

    code:
        mov [rbx+0x18],eax
        mov eax,[rdx+0x30]
        jmp qword ptr [Inertia::reduce_rate_jmp_ret]
    }
}
*/
// siy stuff over

// clang-format on

void Inertia::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> Inertia::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &Inertia::cheaton;
  m_on_page    = Page_Mechanics;

  m_full_name_string     = "Inertia Redirection";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Redirect inertia through air hikes & enemy steps.";

  set_up_hotkey();

  auto addr = m_patterns_cache->find_addr(base, "48 8B 41 08 44 8B 40 78");
  if (!addr) {
    return "Unable to find Inertia pattern.";
  }
  if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 8)) {
    //return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize Inertia";
  }

  // siy stuff
  /*
  auto addrReduceRate = m_patterns_cache->find_addr(base, "89 43 18 8B 42 30");
  if (!addrReduceRate) {
      return "Unable to find Inertia Reduce Rate pattern.";
  }
  if (!install_hook_absolute(addrReduceRate.value(), m_detour2, &inertia_reduce_rate_detour, &reduce_rate_jmp_ret, 6)) {
      //return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize Inertia Reduce Rate";
  }*/
  // siy stuff over

  return Mod::on_initialize();
}

// do something every frame
void Inertia::on_frame() { 
  if (airhiketimer > 1) {
    Inertia::airhiketimer--;
  }
  if (airhiketimer < 10 && airhiketimer > 1) {
    backupxinertia = 0.0;
    backupzinertia = 0.0;
    backupinertia  = 0.0;
  }
}
// will show up in debug window, dump ImGui widgets you want here
void Inertia::on_draw_debug_ui() { 
    ImGui::Text("[Inertia] X inertia: %.2f", &backupxinertia);
    ImGui::Text("[Inertia] Z inertia: %.2f", &backupzinertia);
    ImGui::Text("[Inertia] Composite inertia: %.2f", &backupinertia);
    ImGui::Text("[Inertia] Air Hike Timer: %d", (int*)&Inertia::airhiketimer);
}

// siy stuff
// will show up in main window, dump ImGui widgets you want here
void Inertia::on_draw_ui() {
    // ImGui::Text("This checkbox changes how much inertia is taken by different moves. Work In Progress.");
    // ImGui::Checkbox("Edited Inertia Values", &Inertia::custom_inertia);
}

// during load
void Inertia::on_config_load(const utility::Config &cfg) {
    // Inertia::custom_inertia = cfg.get<bool>("custom_inertia").value_or(false);
}

// during save
void Inertia::on_config_save(utility::Config &cfg) {
    // cfg.set<bool>("custom_inertia", Inertia::custom_inertia);
}
// siy stuff over
