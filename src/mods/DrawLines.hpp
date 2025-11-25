#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class DrawLines : public Mod {
public:
	DrawLines() = default;
	// mod name string for config
	std::string_view get_name() const override { return "DrawLines"; }
	std::string get_checkbox_name() override { return m_check_box_name; };
	std::string get_hotkey_name() override { return m_hot_key_name; };
	// called by m_mods->init() you'd want to override this
	std::optional<std::string> on_initialize() override;

	static inline bool enabled{ false };

	static uintptr_t jmp_ret;
	static uintptr_t baseAddr;
	static bool cheaton;
	static bool playerPosDisplay;
	static bool jcSpheresDisplay;
	static bool attackSpheresDisplay;

	static void Draw3dShapes();
	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override;
	void on_config_save(utility::Config& cfg) override;

	// on_frame() is called every frame regardless whether the gui shows up.
	void on_frame() override;
	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override;
	// on_draw_debug_ui() is called when debug window shows up
	// void on_draw_debug_ui() override;

private:
	// function hook instance for our detour, convinient wrapper
	// around minhook
	void init_check_box_info() override;

	std::shared_ptr<Detour_t> m_detour;
};

// surely ref can dump this stuff but idk how
class camera {
public:
	char pad_0[0x6C0];
	glm::vec3 target; // 0x6C0
	char pad_6CC[0x4];
	char pad_6D0[0xC0];
	glm::vec3 pos; // 0x790
};

class bone {
public:
	char pad_0[0x20];
	glm::vec3 pos; // 0x20
};

class player {
public:
	char pad_0[0x130];
	glm::vec3 pos; // 0x130
	char pad_13C[0x4];
	glm::vec3 rot; // 0x140 // ok i guessed wrong, worth a shot
	char pad_14C[0x4];
	char pad_150[0xCB8];
	bone* head; // 0xE08
	char pad_E10[0x10];
	bone* chest; // 0xE20
	bone* hip; // 0xE28
	char pad_E30[0x160];
	camera* cam; // 0xF90
};

struct PlayerManager {
	char pad_0[0x60];
	player* manualPlayer; // 0x60
};

class enemy { // idk yet
public:
	char pad_0[0x130];
	glm::vec3 pos; // 0x130
	char pad_13C[0x4];
	glm::vec3 rot; // 0x140 // ok i guessed wrong, worth a shot
	char pad_14C[0x4];
	char pad_150[0xCB8];
	bone* head; // 0xE08
	char pad_E10[0x10];
	bone* chest; // 0xE20
	bone* hip; // 0xE28
	char pad_E30[0x160];
	camera* cam; // 0xF90
};

struct EmItems {
	char pad_0[0x20];
	enemy* firstItem; // 0x20
};

struct EmList {
	char pad_0[0x10];
	EmItems* enemyItems; // 0x10
};

struct EnemyManager {
	char pad_0[0xB0];
	EmList* enemyList; // 0xB0
};

struct HitData {
	float unkn00; // 0x00
	float unkn04; // 0x04
	float unkn08; // 0x08
	float unkn0C; // 0x0C
	float unkn10; // 0x10
	float unkn14; // 0x14
	float unkn18; // 0x18
	float unkn1C; // 0x1C
	float unkn20; // 0x20
	float unkn24; // 0x24
	float unkn28; // 0x28
	float unkn2C; // 0x2C
	float unkn30; // 0x30
	float unkn34; // 0x34
	float unkn38; // 0x38
	float unkn3C; // 0x3C
	float unkn40; // 0x40
	float unkn44; // 0x44
	float unkn48; // 0x48
	float unkn4C; // 0x4C
	float unkn50; // 0x50
	float unkn54; // 0x54
	float unkn58; // 0x58
	float unkn5C; // 0x5C
	float unkn60; // 0x60
	float unkn64; // 0x64
	float unkn68; // 0x68
	float unkn6C; // 0x6C
	glm::vec3 pos; // 0x70
	float radius; // 0x7C
};

static player* GetPlayer() {
	uintptr_t* playerManagerPtr = *(uintptr_t**)((uintptr_t)DrawLines::baseAddr + 0x7E61690);
	if (!playerManagerPtr) { return nullptr; }

	PlayerManager* playerManager = (PlayerManager*)playerManagerPtr;
	if (!playerManager) { return nullptr; }

	return playerManager->manualPlayer;
}

static enemy* GetVoidEnemy() {
	uintptr_t* enemyManagerPtr = *(uintptr_t**)((uintptr_t)DrawLines::baseAddr + 0x7E69A30);
	if (!enemyManagerPtr) { return nullptr; }

	EnemyManager* enemyManager = (EnemyManager*)enemyManagerPtr;
	if (!enemyManager) { return nullptr; }

	EmList* enemyList = enemyManager->enemyList;
	if (!enemyList) { return nullptr; }

	EmItems* enemyItems = enemyList->enemyItems;
	if (!enemyItems) { return nullptr; }

	return enemyItems->firstItem;
}

static glm::mat4x4 CalculateViewProjectionMatrix(camera* cam, float hFov, float aspectRatio, float nearPlane = 0.1f, float farPlane = 1000.0f) {
	glm::vec3 forward = glm::normalize(cam->target - cam->pos);
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
	glm::vec3 up = glm::cross(right, forward);

	glm::mat4x4 view = glm::lookAt(cam->pos, cam->target, up);

	float hFovRadians = glm::radians(hFov);
	float vFovRadians = 2.0f * glm::atan(glm::tan(hFovRadians / 2.0f) / aspectRatio);

	glm::mat4x4 proj = glm::perspective(vFovRadians, aspectRatio, nearPlane, farPlane);

	return proj * view;
}