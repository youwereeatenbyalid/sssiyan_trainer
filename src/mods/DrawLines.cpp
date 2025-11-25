#include "DrawLines.hpp"
#include "utility/WorldVisualizer.hpp"
#include "ImGuiExtensions/ImGuiExtensions.h" // for helpmarker
#include "CameraSettings.hpp" // for fov

uintptr_t DrawLines::jmp_ret{ NULL };
uintptr_t DrawLines::baseAddr{ NULL };
bool DrawLines::cheaton{ false };
bool DrawLines::playerPosDisplay{ false };
bool DrawLines::jcSpheresDisplay{ false };
bool DrawLines::attackSpheresDisplay{ false };
static HitData* currentHitDataPtr{ nullptr };

struct HitboxSnapshot {
    glm::vec3 pos;
    float radius;
};
static std::vector<HitboxSnapshot> hitDataList;

static void AddHitDataPtr(void* ptr) {
    auto* hitData = (HitData*)(ptr);
    hitDataList.push_back({ hitData->pos, hitData->radius }); // this line crashes in levels or vergil idk how fix
}

// clang-format off
static naked void detour() {
    __asm {
        movss [rdi+0x58], xmm0
        cmp byte ptr [DrawLines::cheaton], 0
        je retcode
        cmp byte ptr [DrawLines::attackSpheresDisplay], 0
        je retcode
        
		// filter out some always active hitbox
        cmp dword ptr [rdi-0x340], 0x00740053 // 'S' 't'
        je check2
        jmp savePtr

        check2:
        cmp dword ptr [rdi-0x33C], 0x00790061 // 'a' 'y'
        je retcode
        
        savePtr: // god i love not having pushad
        push rax
        push rbx
        push rcx
        push rdx
        push rsi
        push rdi
        push rbp
        push r8
        push r9
        push r10
        push r11
        push r12
        push r13
        push r14
        push r15

        sub rsp, 8 // align stack to 16
        mov rcx, rdi
        call AddHitDataPtr
        add rsp, 8

        pop r15
        pop r14
        pop r13
        pop r12
        pop r11
        pop r10
        pop r9
        pop r8
        pop rbp
        pop rdi
        pop rsi
        pop rdx
        pop rcx
        pop rbx
        pop rax
        
        retcode:
        jmp qword ptr [DrawLines::jmp_ret]
    }
}
// clang-format on

void DrawLines::init_check_box_info() {
    m_check_box_name = m_prefix_check_box_name + std::string(get_name());
    m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> DrawLines::on_initialize() {
    m_is_enabled = &enabled;
    init_check_box_info();

    m_is_enabled = &DrawLines::cheaton;
    m_on_page = Page_Mechanics;
    m_depends_on = {};
    m_full_name_string = "Draw Debug Lines";
    m_author_string = "Siyan";
    m_description_string = "View lines in 3d space that visualize the sizes of collisions etc.";

    set_up_hotkey();

    baseAddr = g_framework->get_module().as<uintptr_t>();
    HMODULE base = (HMODULE)baseAddr;
    auto addr = m_patterns_cache->find_addr(base, "F3 0F 11 47 58 48 83 C4 70"); // DevilMayCry5.exe+45D26FA
    if (!addr) {
        return "Unable to find DrawLines pattern.";
    }

    if (!install_new_detour(addr.value(), m_detour, &detour, &jmp_ret, 5)) {
        //  return a error string in case something goes wrong
        spdlog::error("[{}] failed to initialize", get_name());
        return "Failed to initialize DrawLines";
    }
    return Mod::on_initialize();
}

void DrawLines::Draw3dShapes() {
    if (!g_framework) { return; }
    if (!DrawLines::baseAddr) { return; }
    player* ply = GetPlayer();
    if (!ply) { return; }
    camera* cam = ply->cam;
    if (!cam) { return; }
    enemy* em = GetVoidEnemy();
    if (!em) { return; }

    ImVec2 displaySize = ImGui::GetIO().DisplaySize; // wrong if windowed, should get game res
    float aspectRatio = displaySize.x / displaySize.y;

    // should get live fov because some moves change it, for now we use user set fov or default
    float fov = 65.0f;
    if (CameraSettings::cheaton) {
        fov = CameraSettings::fov;
    }
    // we make our own because I couldn't find the actual proj
    glm::mat4x4 viewProj = CalculateViewProjectionMatrix(cam, fov, aspectRatio);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(displaySize);
    ImGui::Begin("WorldViz", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus);

    WorldVisualizer::SetDrawList(ImGui::GetWindowDrawList());
    WorldVisualizer::SetViewProjectionMatrix(viewProj);

    if (DrawLines::playerPosDisplay) {
        glm::vec3 playerPos = ply->pos;
        glm::vec3 camPos = cam->pos;
        glm::vec3 camTarget = cam->target;

        // ImGui::Begin("Debug");
        // ImGui::InputFloat3("Player Pos", &playerPos.x);
        // ImGui::InputFloat3("Camera Pos, &camPos.x);
        // ImGui::InputFloat3("Target Pos, &camTarget.x);
        // ImGui::End();

        glm::mat3x3 playerRot = WorldVisualizer::CreateRotationMatrix(ply->rot.x, ply->rot.y, ply->rot.z); // this is the wrong offset
        WorldVisualizer::DrawWorldSphere(playerPos, 0.1f, IM_COL32(0, 0, 255, 255), 32, 1.0f, &playerRot);
    }

    if (DrawLines::jcSpheresDisplay) {
        /*if (em->hip && em->chest) { // idk these offsets
            glm::vec3 hipPos = em->pos + em->hip->pos;
            glm::vec3 chestPos = em->pos + em->chest->pos;
            ImGui::Text("hip pos: %.2f, %.2f, %.2f", hipPos.x, hipPos.y, hipPos.z);
            WorldVisualizer::DrawWorldCapsule(hipPos, chestPos, 2.0f, IM_COL32(255, 0, 0, 255), 32, 1.0f);
        }*/
    }

    if (DrawLines::attackSpheresDisplay) {
        for (const HitboxSnapshot& snapshot : hitDataList) {
            WorldVisualizer::DrawWorldSphere(snapshot.pos, snapshot.radius, IM_COL32(255, 0, 0, 255), 32, 1.0f);
        }
        hitDataList.clear();
    }

    ImGui::End();
}

void DrawLines::on_draw_ui() {
    ImGui::Checkbox("Display Player Position", &DrawLines::playerPosDisplay);
    ImGui::ShowHelpMarker("Debug option to check it's displaying correctly");
    // ImGui::Checkbox("Display JC Spheres", &DrawLines::jcSpheresDisplay); // does nothing atm
    ImGui::Checkbox("Display Attack Spheres", &DrawLines::attackSpheresDisplay);
    ImGui::ShowHelpMarker("This crashes in missions or with Vergil, need help from someone clevererer");
}

void DrawLines::on_frame() {
    if (DrawLines::cheaton) { DrawLines::Draw3dShapes(); }
}

void DrawLines::on_config_load(const utility::Config& cfg) {
    playerPosDisplay = cfg.get<bool>("playerPosDisplay").value_or(true);
    jcSpheresDisplay = cfg.get<bool>("jcSpheresDisplay").value_or(false);
    attackSpheresDisplay = cfg.get<bool>("attackSpheresDisplay").value_or(false);
}

void DrawLines::on_config_save(utility::Config& cfg) {
    cfg.set<bool>("playerPosDisplay", playerPosDisplay);
    cfg.set<bool>("jcSpheresDisplay", jcSpheresDisplay);
    cfg.set<bool>("attackSpheresDisplay", attackSpheresDisplay);
}
