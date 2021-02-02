
#include "SpardaWorkshop.hpp"
#include "mods/PlayerTracker.hpp"
uintptr_t SpardaWorkshop::sceneplacer_jmp_ret{NULL};
uintptr_t SpardaWorkshop::enemyplacer_jmp_ret{NULL};
bool SpardaWorkshop::cheaton{NULL};
static glm::vec3 barrelcoords{0.0,0.0,15.0};
static glm::vec3 playercoords{0.0,0.0,0};
static glm::vec3 enemycoords{0.0,0.0,7.0};
bool custombarrel = false;
bool customenemy = false;
bool customplayer = false;
glm::vec3 SpardaWorkshop::custombarrelcoords{ 0.0,0.0,15.0 };
glm::vec3 SpardaWorkshop::customplayercoords{ 0.0,0.0,0 };
glm::vec3 SpardaWorkshop::customenemycoords{ 0.0,0.0,7.0 };
std::vector<glm::vec3> barrelcoordsvector;
std::vector<glm::vec3> playercoordsvector;
std::vector<glm::vec3> enemycoordsvector;

int barrelindex = 0;
int playerindex = 0;
int enemyindex = 0;
// clang-format off
// only in clang/icl mode on x64, sorry

static naked void sceneplacer_detour() {
	__asm {
	    validation:
            //cmp byte ptr [SpardaWorkshop::cheaton], 1
            //je cheatcode
            jmp cheatcode
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
            cmp byte ptr [custombarrel], 1
            je custombarrelcode
            mov eax, [barrelcoords.x]
            mov [rcx+0x30], eax
            mov eax, [barrelcoords.y]
            mov [rcx+0x34], eax
            mov eax, [barrelcoords.z]
            mov [rcx+0x38], eax
            jmp qword ptr [SpardaWorkshop::sceneplacer_jmp_ret]


        custombarrelcode:
            mov eax, [SpardaWorkshop::custombarrelcoords.x]
            mov [rcx+0x30], eax
            mov eax, [SpardaWorkshop::custombarrelcoords.y]
            mov [rcx+0x34], eax
            mov eax, [SpardaWorkshop::custombarrelcoords.z]
            mov [rcx+0x38], eax
            jmp qword ptr[SpardaWorkshop::sceneplacer_jmp_ret]

        playercode:
            cmp byte ptr [customplayer], 1
            je customplayercode
            mov eax, [playercoords.x]
            mov [rcx+0x30], eax
            mov eax, [playercoords.y]
            mov [rcx+0x34], eax
            mov eax, [playercoords.z]
            mov [rcx+0x38], eax
            jmp qword ptr [SpardaWorkshop::sceneplacer_jmp_ret]

        customplayercode:
            mov eax, [SpardaWorkshop::customplayercoords.x]
            mov [rcx+0x30], eax
            mov eax, [SpardaWorkshop::customplayercoords.y]
            mov [rcx+0x34], eax
            mov eax, [SpardaWorkshop::customplayercoords.z]
            mov [rcx+0x38], eax
            jmp qword ptr [SpardaWorkshop::sceneplacer_jmp_ret]
	}
}

static naked void enemyplacer_detour() {
    __asm {
        validation:
            //cmp byte ptr [SpardaWorkshop::cheaton], 1
            //je cheatcode
            jmp cheatcode
        code:
            movss xmm0,[rsi+0x80]
            movss xmm1,[rsi+0x84]
            movss xmm2,[rsi+0x88]
            jmp qword ptr [SpardaWorkshop::enemyplacer_jmp_ret]

        cheatcode:
            cmp byte ptr [customenemy], 1
            je customcheatcode
            movss xmm0,[enemycoords.x]
            movss xmm1,[enemycoords.y]
            movss xmm2,[enemycoords.z]
            jmp qword ptr [SpardaWorkshop::enemyplacer_jmp_ret]

        customcheatcode:
            movss xmm0, [SpardaWorkshop::customenemycoords.x]
            movss xmm1, [SpardaWorkshop::customenemycoords.y]
            movss xmm2, [SpardaWorkshop::customenemycoords.z]
            jmp qword ptr [SpardaWorkshop::enemyplacer_jmp_ret]
    }
}

// clang-format on

void SpardaWorkshop::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> SpardaWorkshop::on_initialize() {
  init_check_box_info();

  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  ischecked = &SpardaWorkshop::cheaton;
  onpage    = gamemode;
  full_name_string     = "Sparda's Workshop";
  author_string        = "Lidemi";
  description_string   = "Integration with the Sparda's Workshop mod.\n"
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
  barrelcoordsvector.push_back({ -3, 0, 80 });
  barrelcoordsvector.push_back({ 2.5, 11.25, 53.5 });
  barrelcoordsvector.push_back({ -25, 8.5, 6 });
  barrelcoordsvector.push_back({ -31, 5, 12.5 });
  barrelcoordsvector.push_back({ -31, 6, -36.5 });
  barrelcoordsvector.push_back({ -33.6, 13.8, -10 });
  barrelcoordsvector.push_back({ -28, 0 , -10 });
  barrelcoordsvector.push_back({ -27, 3.5, -2 });
  barrelcoordsvector.push_back({ -27, 2.0, - 15 });
  barrelcoordsvector.push_back({ 26, 0, -42.7 });
  barrelcoordsvector.push_back({ 44, 3.3, -18.7 });
  barrelcoordsvector.push_back({ 35.35, 0, -57.6 });

  enemycoordsvector.push_back({ 0,0,7 });
  enemycoordsvector.push_back({ 36.5, 5, 35 });
  enemycoordsvector.push_back({ 32, 20, 40 });
  enemycoordsvector.push_back({ 41, 20, 40 });
  enemycoordsvector.push_back({ 19, 30, 50 });
  enemycoordsvector.push_back({ -1.5, 12, 53.5 });
  enemycoordsvector.push_back({ -15.67, 28, 48.5 });
  enemycoordsvector.push_back({ -10.5, 28, 51.5 });
  enemycoordsvector.push_back({ -5, 20, 47 });
  enemycoordsvector.push_back({ -12, 3, 24 });
  enemycoordsvector.push_back({ -3, 20, 43 });
  enemycoordsvector.push_back({ -18, 25, 46 });
  enemycoordsvector.push_back({ 2, 20, 62 });
  enemycoordsvector.push_back({ -7.94, 0, 77 });
  enemycoordsvector.push_back({ 19, 25, 46 });
  enemycoordsvector.push_back({ -18, 15, 26 });
  enemycoordsvector.push_back({ -27, 25, -10 });
  enemycoordsvector.push_back({ -18, 0, 1.5 });
  enemycoordsvector.push_back({ -27, 15, -17 });
  enemycoordsvector.push_back({ -27, 25, -3 });
  enemycoordsvector.push_back({ -29, 25, 8 });
  enemycoordsvector.push_back({ -29, 25, -29 });
  enemycoordsvector.push_back({ -9, 25, -29 });
  enemycoordsvector.push_back({ -9, 25, 9 });
  enemycoordsvector.push_back({ -31, 25, 18 });
  enemycoordsvector.push_back({ -31, 25, -45 });
  enemycoordsvector.push_back({ -36.7, 25, -13 });
  enemycoordsvector.push_back({ 37.8, 0, -25.5 });
  enemycoordsvector.push_back({ 30.9, 2.1, -17.9 });
  enemycoordsvector.push_back({ 43.7, 0, -47 });
  enemycoordsvector.push_back({ 41.2, 18, -57.5 });
  enemycoordsvector.push_back({ 35.35, 0, -57.6 });
  enemycoordsvector.push_back({ 27.5, 18, -60 });
  enemycoordsvector.push_back({ 36.5, 18, -34 });
  enemycoordsvector.push_back({ 54.45, 20, -17.8 });
  enemycoordsvector.push_back({ 35.35, 0, -57.6 });

  enemycoordsvector.push_back({ 35.35, 0, -57.6 });

  playercoordsvector.push_back({0,0,0 });
  playercoordsvector.push_back({ 36.5, 0, 27 });
  playercoordsvector.push_back({ 34, 8, 40 });
  playercoordsvector.push_back({ 21, 16, 50 });
  playercoordsvector.push_back({ 1.2, 12, 53.5 });
  playercoordsvector.push_back({ -10.5, 12, 51.5 });
  playercoordsvector.push_back({ -15.67, 10, 48.5 });
  playercoordsvector.push_back({ 0, 2, 36.2 });
  playercoordsvector.push_back({ -5, 6, 47 });
  playercoordsvector.push_back({ -8, 3, 24 });
  playercoordsvector.push_back({ 0, 3, 36.5 });
  playercoordsvector.push_back({ -25, 0, 45 });
  playercoordsvector.push_back({ -4, 4, 60 });
  playercoordsvector.push_back({ -7.94, 0, 70 });
  playercoordsvector.push_back({ 16, 10, 46 });
  playercoordsvector.push_back({ -17, 0, 26 });
  playercoordsvector.push_back({ -22, 0, -10 });
  playercoordsvector.push_back({ -15, 0, 1.5 });
  playercoordsvector.push_back({ -27, 5, 0 });
  playercoordsvector.push_back({ -26, 5, -17 });
  playercoordsvector.push_back({ -23, 7, 8 });
  playercoordsvector.push_back({ -23, 7, -29 });
  playercoordsvector.push_back({ -6, 0, -29 });
  return Mod::on_initialize();
}

// during load
void SpardaWorkshop::on_config_load(const utility::Config &cfg) {
     SpardaWorkshop::custombarrelcoords.x = cfg.get<float>("barrel X").value_or(0.0);
     SpardaWorkshop::custombarrelcoords.y = cfg.get<float>("barrel Y").value_or(0.0);
     SpardaWorkshop::custombarrelcoords.z = cfg.get<float>("barrel Z").value_or(15.0);
     custombarrel = cfg.get<float>("custom barrel").value_or(false);
     customplayer = cfg.get<float>("custom player").value_or(false);
     customenemy = cfg.get<float>("custom enemy").value_or(false);

     barrelindex = cfg.get<int>("barrel index").value_or(0);
     playerindex = cfg.get<int>("player index").value_or(0);
     enemyindex = cfg.get<int>("enemy index").value_or(0);
     SpardaWorkshop::customplayercoords.x = cfg.get<float>("player X").value_or(0.0);
     SpardaWorkshop::customplayercoords.y = cfg.get<float>("player Y").value_or(0.0);
     SpardaWorkshop::customplayercoords.z = cfg.get<float>("player Z").value_or(0.0);

     SpardaWorkshop::customenemycoords.x = cfg.get<float>("enemy X").value_or(0.0);
     SpardaWorkshop::customenemycoords.y = cfg.get<float>("enemy Y").value_or(0.0);
     SpardaWorkshop::customenemycoords.z = cfg.get<float>("enemy Z").value_or(7.0);
}
// during save
void SpardaWorkshop::on_config_save(utility::Config &cfg) {
    cfg.set<float>("barrel X", custombarrelcoords.x);
    cfg.set<float>("barrel Y", custombarrelcoords.y);
    cfg.set<float>("barrel Z", custombarrelcoords.z);

    cfg.set<float>("player X", customplayercoords.x);
    cfg.set<float>("player Y", customplayercoords.y);
    cfg.set<float>("player Z", customplayercoords.z);

    cfg.set<float>("enemy X", customenemycoords.x);
    cfg.set<float>("enemy Y", customenemycoords.y);
    cfg.set<float>("enemy Z", customenemycoords.z);

    cfg.set<float>("custom barrel",custombarrel);
    cfg.set<float>("custom player", customplayer);
    cfg.set<float>("custom enemy", customenemy);

    cfg.set<int>("barrel index", barrelindex);
    cfg.set<int>("player index", playerindex);
    cfg.set<int>("enemy index", enemyindex);
}
// do something every frame
void SpardaWorkshop::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void SpardaWorkshop::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here


void SpardaWorkshop::on_draw_ui() {
    //ImGui::Text("Here for a quick check");

    ImGui::Text("Preset Barrel Positions");
    if(ImGui::Combo("##Barrel Positions", &barrelindex, 
        "Position 0\0Position 1\0Position 2\0Position 3\0"
        "Position 4\0Position 5\0Position 6\0Position 7\0"
        "Position 8\0Position 9\0Position 10\0Position 11\0"
        "Position 12\0Position 13\0Position 14\0Position 15\0"
        "Position 16\0")){
        barrelcoords = barrelcoordsvector.at(barrelindex);
    }
    ImGui::Spacing();
    ImGui::Checkbox("Use Custom Barrel Position",&custombarrel);
    ImGui::Text("Custom Barrel Position");
    ImGui::DragFloat3("##Custom Barrel Position", (float*)&custombarrelcoords);
    if(ImGui::Button("Set to player coordinates##barrel")&& PlayerTracker::playertransform != NULL){
        float* x = (float*)PlayerTracker::playertransform + 0xC;
        float* y = (float*)PlayerTracker::playertransform + 0xD;
        float* z = (float*)PlayerTracker::playertransform + 0xE;
        custombarrelcoords = {*x,*y,*z};
    }
    ImGui::Separator();
    ImGui::Text("Preset Enemy Positions");
    if (ImGui::Combo("##Enemy Positions", &enemyindex,
        "Position 0\0Position 1\0Position 2\0Position 3\0"
        "Position 4\0Position 5\0Position 6\0Position 7\0"
        "Position 8\0Position 9\0Position 10\0Position 11\0"
        "Position 12\0Position 13\0Position 14\0Position 15\0"
        "Position 16\0Position 17\0Position 18\0Position 19\0"
        "Position 20\0Position 21\0Position 22\0Position 23\0"
        "Position 24\0Position 25\0Position 26\0Position 27\0"
        "Position 28\0Position 29\0Position 30\0Position 31\0"
        "Position 32\0Position 33\0")) {
        enemycoords = enemycoordsvector.at(enemyindex);
    }
    ImGui::Spacing();
    ImGui::Checkbox("Use Custom Enemy Position", &customenemy);
    ImGui::Text("Custom Enemy Position");
    ImGui::DragFloat3("##Custom Enemy Position", (float*)&customenemycoords);
    if (ImGui::Button("Set to player coordinates##enemy") && PlayerTracker::playertransform != NULL) {
        float* x = (float*)PlayerTracker::playertransform + 0xC;
        float* y = (float*)PlayerTracker::playertransform + 0xD;
        float* z = (float*)PlayerTracker::playertransform + 0xE;
        customenemycoords = { *x,*y,*z };
    }
    ImGui::Separator();
    ImGui::Text("Preset Player Positions");
    if (ImGui::Combo("##Player Positions", &playerindex,
        "Position 0\0Position 1\0Position 2\0Position 3\0"
        "Position 4\0Position 5\0Position 6\0Position 7\0"
        "Position 8\0Position 9\0Position 10\0Position 11\0"
        "Position 12\0Position 13\0Position 14\0Position 15\0"
        "Position 16\0Position 17\0Position 18\0Position 19\0"
        "Position 20\0Position 21\0Position 22\0")) {
        playercoords = playercoordsvector.at(playerindex);
    }
    ImGui::Spacing();
    ImGui::Checkbox("Use Custom Player Position", &customplayer);
    ImGui::Text("Custom Player Position");
    ImGui::DragFloat3("##Custom Player Position", (float*)&customplayercoords);
    if (ImGui::Button("Set to player coordinates##player") && PlayerTracker::playertransform != NULL) {
        float* x = (float*)PlayerTracker::playertransform + 0xC;
        float* y = (float*)PlayerTracker::playertransform + 0xD;
        float* z = (float*)PlayerTracker::playertransform + 0xE;
        customplayercoords = { *x,*y,*z };
    }
}
