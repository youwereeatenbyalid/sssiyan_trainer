#pragma once
#include "Mod.hpp"
#include "events/Events.hpp"

namespace EndLvlHooks
{    
    enum EndType
    {
        RetryMission,
        CheckpointMission,
        ExitMission,
        SecretMissionExit,
        Bp,
        RequestResult,
        RequestResultStaffRoll //cool, capcom, cool, this calles after Dante credit fight if fight skipped/ losed to Vergil(?)
    };
    class EndLvlHooks : public Mod
    {
    private:
        friend class IEndLvl;

        static inline Events::Event<EndType> endLvlEvent{};

        template<typename T>
        static void subscribe_end_event(std::shared_ptr<Events::EventHandler<T, EndType>> eh)
        {
            endLvlEvent.subscribe(eh);
        }

        template<typename T>
        static void unsubscribe_end_event(std::shared_ptr<Events::EventHandler<T, EndType>> eh)
        {
            endLvlEvent.unsubscribe(eh);
        }

        void init_check_box_info() override
        {
            m_check_box_name = m_prefix_check_box_name + std::string(get_name());
            m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
        };

        static inline EndLvlHooks* _mod = nullptr;

        std::unique_ptr<FunctionHook> m_request_retire_mission_hook;
        std::unique_ptr<FunctionHook> m_retry_mission_hook;
        std::unique_ptr<FunctionHook> m_exit_mission_hook;
        std::unique_ptr<FunctionHook> m_checkpoint_mission_hook;
        std::unique_ptr<FunctionHook> m_secret_mission_hook;
        std::unique_ptr<FunctionHook> m_exit_bp_mission_hook;
        std::unique_ptr<FunctionHook> m_request_result_hook;
        std::unique_ptr<FunctionHook> m_request_result_staffroll_hook;

    public:


        EndLvlHooks() = default;

        static inline void retry_mission_detour(uintptr_t threadCtxt, uintptr_t obj)
        {
            endLvlEvent.invoke(RetryMission);
            _mod->m_retry_mission_hook->get_original<decltype(EndLvlHooks::retry_mission_detour)>()(threadCtxt, obj);
        }

        static inline void exit_mission_detour(uintptr_t threadCtxt, uintptr_t obj, bool isChangeTitleMenu, bool isChangeResult, bool isChangeResultToMainMenu)
        {
            endLvlEvent.invoke(ExitMission);
            _mod->m_exit_mission_hook->get_original<decltype(EndLvlHooks::exit_mission_detour)>()(threadCtxt, obj, isChangeTitleMenu, isChangeResult, isChangeResultToMainMenu);
        }

        static inline void checkpoint_mission_detour(uintptr_t threadCtxt, uintptr_t obj, bool isFromPauseMenu)
        {
            endLvlEvent.invoke(CheckpointMission);
            _mod->m_checkpoint_mission_hook->get_original<decltype(EndLvlHooks::checkpoint_mission_detour)>()(threadCtxt, obj, isFromPauseMenu);
        }

        static inline void smiss_exit_detour(uintptr_t threadCtxt, uintptr_t obj)
        {
            endLvlEvent.invoke(SecretMissionExit);
            _mod->m_secret_mission_hook->get_original<decltype(EndLvlHooks::smiss_exit_detour)>()(threadCtxt, obj);
        }

        static inline void exit_bp_mission_detour(uintptr_t threadCtxt, uintptr_t obj, bool isChangeTitleMenu, bool isChangeResultToMainMenu)
        {
            endLvlEvent.invoke(Bp);
            _mod->m_exit_bp_mission_hook->get_original<decltype(EndLvlHooks::exit_bp_mission_detour)>()(threadCtxt, obj, isChangeTitleMenu, isChangeResultToMainMenu);
        }

        static inline void request_result_detour(uintptr_t threadCtxt, uintptr_t obj, bool isOverlayCapture)
        {
            endLvlEvent.invoke(RequestResult);
            _mod->m_request_result_hook->get_original<decltype(EndLvlHooks::request_result_detour)>()(threadCtxt, obj, isOverlayCapture);
        }

        static inline void request_staffresult_detour(uintptr_t threadCtxt, uintptr_t obj, bool isOverlayCapture)
        {
            endLvlEvent.invoke(RequestResultStaffRoll);
            _mod->m_request_result_staffroll_hook->get_original<decltype(EndLvlHooks::request_staffresult_detour)>()(threadCtxt, obj, isOverlayCapture);
        }

        std::string_view get_name() const override
        {
            return "EndLvlHooks";
        }
        std::string get_checkbox_name() override
        {
            return m_check_box_name;
        };
        std::string get_hotkey_name() override
        {
            return m_hot_key_name;
        };

        std::optional<std::string> on_initialize() override
        {
            init_check_box_info();
            auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

            _mod = this;

            auto retryMissionAddr = m_patterns_cache->find_addr(base, "40 5F C3 CC CC CC 48 89 5C 24 18 56");// DevilMayCry5.exe+249DCA0 (-0x6)
            if (!retryMissionAddr)
            {
                return "Unanable to find retryMissionAddr pattern.";
            }

            auto exitMissionAddr = m_patterns_cache->find_addr(base, "20 5F C3 CC CC CC CC CC CC 48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 41");// DevilMayCry5.exe+249DA90 (-0x9)
            if (!retryMissionAddr)
            {
                return "Unanable to find exitMissionAddr pattern.";
            }

            auto checkpointMissionAddr = m_patterns_cache->find_addr(base, "48 89 5C 24 10 48 89 74 24 18 57 48 83 EC 20 41 0F B6 F8");// DevilMayCry5.exe+249DFA0
            if (!checkpointMissionAddr)
            {
                return "Unanable to find checkpointMissionAddr pattern.";
            }

            auto exitBpMissionAddr = m_patterns_cache->find_addr(base, "C3 CC 48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 41 56 48 83 EC 20 41");// DevilMayCry5.exe+24A1600 (-0x2)
            if (!exitBpMissionAddr)
            {
                return "Unanable to find exitBpMissionAddr pattern.";
            }

            auto exitSecretMissionAddr = g_framework->get_module().as<uintptr_t>() + 0x249FD20; //Bad AOB shit

            auto requestResultAddr = m_patterns_cache->find_addr(base, "C3 CC CC CC 48 89 5C 24 18 48 89 6C 24 20 56 41 56 41 57 48 83 EC 50 45");// DevilMayCry5.exe+88E940 (-0x4)
            if (!exitBpMissionAddr)
            {
                return "Unanable to find requestResultAddr pattern.";
            }

            auto requestResultStaffRollAddr = m_patterns_cache->find_addr(base, "6E FF FF C3 CC CC CC CC CC CC CC CC CC CC CC CC 48 89 5C 24 18" /*"48 89 5C 24 18 48 89 6C 24 20 56 41 56 41 57 48 83 EC 50 45 33 FF 45 0F B6 F0 41 8B EF 48 8B F2"*/);// DevilMayCry5.app_FlowManager__requestResultStaffRoll241224// DevilMayCry5.exe+88F1D0
            if (!requestResultStaffRollAddr)
            {
                return "Unanable to find requestResultStaffRollAddr pattern.";
            }

            m_retry_mission_hook = std::make_unique<FunctionHook>(retryMissionAddr.value() + 0x6, &EndLvlHooks::retry_mission_detour);
            m_retry_mission_hook->create();
            m_exit_mission_hook = std::make_unique<FunctionHook>(exitMissionAddr.value() + 0x9, &EndLvlHooks::exit_mission_detour);
            m_exit_mission_hook->create();
            m_checkpoint_mission_hook = std::make_unique<FunctionHook>(checkpointMissionAddr.value(), &EndLvlHooks::checkpoint_mission_detour);
            m_checkpoint_mission_hook->create();
            m_secret_mission_hook = std::make_unique<FunctionHook>(exitSecretMissionAddr, &EndLvlHooks::smiss_exit_detour);
            m_secret_mission_hook->create();
            m_exit_bp_mission_hook = std::make_unique<FunctionHook>(exitBpMissionAddr.value() + 0x2, &EndLvlHooks::exit_bp_mission_detour);
            m_exit_bp_mission_hook->create();
            m_request_result_hook = std::make_unique<FunctionHook>(requestResultAddr.value() + 0x4, &EndLvlHooks::request_result_detour);
            m_request_result_hook->create();
            m_request_result_staffroll_hook = std::make_unique<FunctionHook>(requestResultStaffRollAddr.value() + 0x10, &EndLvlHooks::request_staffresult_detour);
            m_request_result_staffroll_hook->create();

            return Mod::on_initialize();
        };

    };

    class IEndLvl
    {
    protected:
        //This function calls before game starting to release all level's resources (bp, mission, secret mission)
        virtual void reset(EndType end) = 0;

    public:

        IEndLvl()
        {
            EndLvlHooks::EndLvlHooks::subscribe_end_event(std::make_shared<Events::EventHandler<IEndLvl, EndType>>(this, &IEndLvl::reset));
        }

        ~IEndLvl()
        {
            EndLvlHooks::EndLvlHooks::unsubscribe_end_event(std::make_shared<Events::EventHandler<IEndLvl, EndType>>(this, &IEndLvl::reset));
        }
    };
}