
#include "SpardaWorkshop.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t SpardaWorkshop::sceneplacer_jmp_ret{NULL};
uintptr_t SpardaWorkshop::enemyplacer_jmp_ret{NULL};
bool SpardaWorkshop::cheaton{NULL};
static glm::vec3 barrelcoords{0.0,0.0,15.0};
static glm::vec3 playercoords{2.0,0.0,7.0};
static glm::vec3 enemycoords{3.0,4.0,-3.0};
std::vector<glm::vec3> barrelcoordsvector;
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void sceneplacer_detour() {
	__asm {
	    validation:
            cmp byte ptr [SpardaWorkshop::cheaton], 1
            je cheatcode
            jmp code
        code:
            mov eax, [rdx+0x8]
            mov [rcx+0x38], eax
            jmp qword ptr [SpardaWorkshop::sceneplacer_jmp_ret]
        cheatcode:
            mov eax, [rdx+0x8]
            cmp eax, 0x4AA3056A
            je barrelcode
            cmp eax, 0x42883333
            je playercode
            jmp code
        barrelcode:
            mov eax, [barrelcoords.x]
            mov [rcx+0x30], eax
            mov eax, [barrelcoords.y]
            mov [rcx+0x34], eax
            mov eax, [barrelcoords.z]
            mov [rcx+0x38], eax
            jmp qword ptr [SpardaWorkshop::sceneplacer_jmp_ret]
        playercode:
            mov eax, [playercoords.x]
            mov [rcx+0x30], eax
            mov eax, [playercoords.y]
            mov [rcx+0x34], eax
            mov eax, [playercoords.z]
            mov [rcx+0x38], eax
            jmp qword ptr [SpardaWorkshop::sceneplacer_jmp_ret]
	}
}


static naked void enemyplacer_detour() {
    __asm {
        validation:
            cmp byte ptr [SpardaWorkshop::cheaton], 1
            je cheatcode
            jmp code
        code:
            movss xmm0,[rsi+0x80]
            movss xmm1,[rsi+0x84]
            movss xmm2,[rsi+0x88]
            jmp qword ptr [SpardaWorkshop::enemyplacer_jmp_ret]

        cheatcode:
            movss xmm0,[enemycoords.x]
            movss xmm1,[enemycoords.y]
            movss xmm2,[enemycoords.z]
            jmp qword ptr [SpardaWorkshop::enemyplacer_jmp_ret]
    }
}

// clang-format on

std::optional<std::string> SpardaWorkshop::on_initialize() {
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  ischecked = &SpardaWorkshop::cheaton;
  onpage    = commonpage;
  full_name_string     = "Sparda's Workshop";
  author_string        = "Lidemi";
  description_string   = "Integration with the Sparda's Workshop mod."
                         "Set player, barrel & enemy positions.";

  auto sceneplacer_addr = utility::scan(base, "8B 42 08 89 41 38 80");
  if (!sceneplacer_addr) {
    return "Unable to find sceneplacer pattern.";
  }
  auto enemyplacer_addr = utility::scan(base, "01 00 00 F3 0F 10 86 80 00 00 00 F3");
  if (!enemyplacer_addr) {
      return "Unable to find enemyplacer pattern.";
  }
  if (!install_hook_absolute(sceneplacer_addr.value(), m_sceneplacer_hook, &sceneplacer_detour, &sceneplacer_jmp_ret, 6)) {
  //  return a error string in case something goes wrong
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize scene placer";
  }
  if (!install_hook_absolute(enemyplacer_addr.value()+3, m_enemyplacer_hook, &enemyplacer_detour, &enemyplacer_jmp_ret, 24)) {
      //  return a error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize enemy placer";
  }

  //coordinate setup for barrel;
  barrelcoordsvector.push_back({0.0,0.0,15.0 });
  barrelcoordsvector.push_back({33.75, 7.8, 34.0});
  barrelcoordsvector.push_back({-3, 0, 44});
  barrelcoordsvector.push_back({-5.7, 2.8, 22.3});

  return Mod::on_initialize();
}

// during load
void SpardaWorkshop::on_config_load(const utility::Config &cfg) {}
// during save
void SpardaWorkshop::on_config_save(utility::Config &cfg) {}
// do something every frame
void SpardaWorkshop::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void SpardaWorkshop::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here


void SpardaWorkshop::on_draw_ui() {
    //ImGui::Text("Here for a quick check");
    int barrelindex = 0;
    int playerindex = 0;
    int enemyindex = 0;

    if(ImGui::Combo("Barrel Positions", &barrelindex, "Position 0\0Position 1\0Position 2\0Position 3\0")){
        barrelcoords = barrelcoordsvector.at(barrelindex);
    }
}
