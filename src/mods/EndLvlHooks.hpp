#pragma once
#include "Mod.hpp"

namespace EndLvlHooks
{
    class EndLvlReset
    {
    protected:
        virtual void reset() = 0;

    public:

        EndLvlReset();
    };

    class EndLvlHooks : public Mod
    {
    private:
        typedef void (EndLvlReset::* resetF)();

        struct ClassFunc
        {
             EndLvlReset *ptr;
             resetF func;

            ClassFunc(EndLvlReset *thisPtr, resetF &f) : ptr(thisPtr), func(f) { }
        };


        static inline std::vector<std::unique_ptr<ClassFunc>> resetFuncList{};

        void init_check_box_info() override
        {
            m_check_box_name = m_prefix_check_box_name + std::string(get_name());
            m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
        };

        std::unique_ptr<FunctionHook> m_request_retire_mission_hook;
        std::unique_ptr<FunctionHook> m_retry_mission_hook;
        std::unique_ptr<FunctionHook> m_exit_mission_hook;
        std::unique_ptr<FunctionHook> m_checkpoint_mission_hook;
        std::unique_ptr<FunctionHook> m_secret_mission_hook;
        std::unique_ptr<FunctionHook> m_exit_bp_mission_hook;
        std::unique_ptr<FunctionHook> m_request_result_hook;

    public:

        static inline uintptr_t retryMissionRet = 0;
        static inline uintptr_t exitMissionRet = 0;
        static inline uintptr_t checkpointMissionRet = 0;
        static inline uintptr_t secretMissionRet = 0;
        static inline uintptr_t exitBpMissionRet = 0;
        static inline uintptr_t requestResultRet = 0;

        static void subscribe_reset_func(EndLvlReset *thisPtr, resetF func)
        {
            resetFuncList.emplace_back(std::make_unique<ClassFunc>(thisPtr, func));
        }

        EndLvlHooks() = default;

        static void end_actions_asm()
        {
            for (auto& t : resetFuncList)
                std::invoke(t->func, *(t->ptr));
        }

        static naked void retry_mission_detour()
        {
            __asm {
                push rax
                push rcx
                push rdx
                push r8
                push r9
                sub rsp, 32
                call qword ptr[EndLvlHooks::end_actions_asm]
                add rsp, 32
                pop r9
                pop r8
                pop rdx
                pop rcx
                pop rax

                originalcode :
                mov [rsp + 0x18], rbx
                jmp qword ptr[EndLvlHooks::retryMissionRet]
            }
        }

        static naked void exit_mission_detour()
        {
            __asm {
                push rax
                push rcx
                push rdx
                push r8
                push r9
                sub rsp, 32
                call qword ptr[EndLvlHooks::end_actions_asm]
                add rsp, 32
                pop r9
                pop r8
                pop rdx
                pop rcx
                pop rax

                originalcode :
                mov [rsp + 0x10], rbx
                jmp qword ptr[EndLvlHooks::exitMissionRet]
            }
        }

        static naked void checkpoint_mission_detour()
        {
            __asm {
                push rax
                push rcx
                push rdx
                push r8
                push r9
                sub rsp, 32
                call qword ptr[EndLvlHooks::end_actions_asm]
                add rsp, 32
                pop r9
                pop r8
                pop rdx
                pop rcx
                pop rax

                originalcode :
                mov [rsp + 0x10], rbx
                jmp qword ptr[EndLvlHooks::checkpointMissionRet]
            }
        }

        static naked void smiss_exit_detour()
        {
            __asm {
                push rax
                push rcx
                push rdx
                push r8
                push r9
                sub rsp, 32
                call qword ptr[EndLvlHooks::end_actions_asm]
                add rsp, 32
                pop r9
                pop r8
                pop rdx
                pop rcx
                pop rax

                originalcode :
                mov[rsp + 0x10], rbx
                jmp qword ptr[EndLvlHooks::secretMissionRet]
            }
        }

        static naked void exit_bp_mission_detour()
        {
            __asm {
                push rax
                push rcx
                push rdx
                push r8
                push r9
                sub rsp, 32
                call qword ptr[EndLvlHooks::end_actions_asm]
                add rsp, 32
                pop r9
                pop r8
                pop rdx
                pop rcx
                pop rax

                originalcode :
                mov[rsp + 0x10], rbx
                jmp qword ptr[EndLvlHooks::exitBpMissionRet]
            }
        }

        static naked void request_result_detour()
        {
            __asm {
                push rax
                push rcx
                push rdx
                push r8
                push r9
                sub rsp, 32
                call qword ptr[EndLvlHooks::end_actions_asm]
                add rsp, 32
                pop r9
                pop r8
                pop rdx
                pop rcx
                pop rax

                originalcode :
                mov[rsp + 0x18], rbx
                jmp qword ptr[EndLvlHooks::requestResultRet]
            }
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

            if (!install_hook_absolute(retryMissionAddr.value() + 0x6, m_retry_mission_hook, &retry_mission_detour, &retryMissionRet, 0x5))
            {
                spdlog::error("[{}] failed to initialize", get_name());
                return "Failed to initialize EndLvlHooks.retryMission";
            }

            if (!install_hook_absolute(exitMissionAddr.value() + 0x9, m_exit_mission_hook, &exit_mission_detour, &exitMissionRet, 0x5))
            {
                spdlog::error("[{}] failed to initialize", get_name());
                return "Failed to initialize EndLvlHooks.exitMission";
            }

            if (!install_hook_absolute(checkpointMissionAddr.value(), m_checkpoint_mission_hook, &checkpoint_mission_detour, &checkpointMissionRet, 0x5))
            {
                spdlog::error("[{}] failed to initialize", get_name());
                return "Failed to initialize EndLvlHooks.checkpointMission";
            }

            if (!install_hook_absolute(exitSecretMissionAddr, m_secret_mission_hook, &smiss_exit_detour, &secretMissionRet, 0x5))
            {
                spdlog::error("[{}] failed to initialize", get_name());
                return "Failed to initialize EndLvlHooks.exitSecretMission";
            }

            if (!install_hook_absolute(exitBpMissionAddr.value() + 0x2, m_exit_bp_mission_hook, &exit_bp_mission_detour, &exitBpMissionRet, 0x5))
            {
                spdlog::error("[{}] failed to initialize", get_name());
                return "Failed to initialize EndLvlHooks.exitBpMission";
            }

            if (!install_hook_absolute(requestResultAddr.value() + 0x4, m_request_result_hook, &request_result_detour, &requestResultRet, 0x5))
            {
                spdlog::error("[{}] failed to initialize", get_name());
                return "Failed to initialize EndLvlHooks.requestResult";
            }

            return Mod::on_initialize();
        };

    };
}