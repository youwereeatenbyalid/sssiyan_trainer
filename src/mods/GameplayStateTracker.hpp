#pragma once
#include "Mod.hpp"
#include "GameFunctions/GameFunc.hpp"
#include "events/Events.hpp"

class GameplayStateTracker : public Mod
{
private:
	void init_check_box_info() override;
	std::unique_ptr<FunctionHook> m_now_flow_hook;
	std::unique_ptr<FunctionHook> m_gamemode_hook;
	std::unique_ptr<FunctionHook> m_bploadflow_hook;
	std::unique_ptr<FunctionHook> m_cutscene_hook;
	std::unique_ptr<FunctionHook> m_pause_hook;
	std::unique_ptr<FunctionHook> m_after_pfbmanager_init;

	Events::Event<> _pfbManagerInited;

	static void pfb_info_add_hook(uintptr_t threadCtxt, uintptr_t pfbInfoList, uintptr_t pfbInfo);

	static inline GameplayStateTracker* _mod = nullptr;

public:
	static inline uintptr_t nowFlowRet = 0;
	static inline uintptr_t gameModeRet = 0;
	static inline uintptr_t pfbMngrAddEmRet = 0;
	static inline uintptr_t bpRetJmp = 0;
	static inline uint32_t bpFlowId = 0;
	static inline uintptr_t isCutsceneRet = 0;
	//static inline uintptr_t isPauseBase = 0x07E55910;
	static inline uintptr_t isPauseExeRet = 0;

	static inline uint32_t nowFlow = 0;//22-game, 17 - start
	static inline uint32_t prevFlow = 0;
	static inline uint32_t gameMode = 0;
	static inline uint32_t flowTmp = 0;

	static inline bool isInMission = false;//NowFlow.isOnMission
	static inline bool isCutscene = false;
	static inline bool isExecutePause = false;//GameManager.PauseManager.isExecutePause

	//static inline const std::array<uintptr_t, 4> isPauseOffst = { 0x100, 0x288, 0xC8, 0x5C4 };

	GameplayStateTracker() = default;

	template<class T>
	static inline void after_pfb_manager_init_sub(std::shared_ptr<Events::EventHandler<T>> eh)
	{
		_mod->_pfbManagerInited.subscribe(eh);
	}

	template<class T>
	static inline void after_pfb_manager_init_unsub(std::shared_ptr<Events::EventHandler<T>> eh)
	{
		_mod->_pfbManagerInited.unsubscribe(eh);
	}

	std::string_view get_name() const override
	{
		return "GameplayStateTracker";
	}
	std::string get_checkbox_name() override
	{
		return m_check_box_name;
	};
	std::string get_hotkey_name() override
	{
		return m_hot_key_name;
	};

	std::optional<std::string> on_initialize() override;

	// Override this things if you want to store values in the config file
	void on_config_load(const utility::Config& cfg) override;
	void on_config_save(utility::Config& cfg) override;

	// on_frame() is called every frame regardless whether the gui shows up.
	void on_frame() override;
	// on_draw_ui() is called only when the gui shows up
	// you are in the imgui window here.
	void on_draw_ui() override;
	// on_draw_debug_ui() is called when debug window shows up
	void on_draw_debug_ui() override;
};