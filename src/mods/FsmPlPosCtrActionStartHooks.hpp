#pragma once
#include "Mod.hpp"
#include "PlSetActionData.hpp"
#include "AirTrickDodge.hpp"
#include "GameFunctions/GameFunc.hpp"
#include "events/Events.hpp"
#include "events/EventArgs.hpp"

namespace gf = GameFunctions;

namespace ActionStartHooks
{

	class PosCtrlSpeedEventArgs : Events::EventArgs
	{
	public:
		enum Axis
		{
			Undef,
			X,
			Y,
			Z,
			NX,
			NY,
			NZ
		};

	private:
		const uintptr_t _ctrl;
		uintptr_t _character;
		gf::Vec3 *_curSpeed;
		Axis *_basisAxis;
		bool *_isFly;

	public:
		PosCtrlSpeedEventArgs(uintptr_t posController, uintptr_t character, gf::Vec3 *curSpeed, Axis *basisAxis, bool *isFly) : _character(character), _curSpeed(curSpeed), _basisAxis(basisAxis), _isFly(isFly), _ctrl(posController) {}
		PosCtrlSpeedEventArgs(uintptr_t posCtrl) : _ctrl(posCtrl)
		{
			if (_ctrl != 0)
			{
				_character = *(uintptr_t*)(posCtrl + 0x28);
				auto tmp = *(uintptr_t*)(posCtrl + 0xA0);
				_curSpeed = (gf::Vec3*)(tmp + 0x20);
				_basisAxis = (Axis*)(posCtrl + 0x98);
				_isFly = (bool*)(posCtrl + 0x64);
			}
		}

		uintptr_t get_pos_ctrl() const noexcept { return _ctrl; }
		uintptr_t get_char() const noexcept {return _character; }
		gf::Vec3 *cur_speed() noexcept { return _curSpeed; }
		Axis *basis_axis() noexcept { return _basisAxis; }
		bool *is_fly() noexcept { return _isFly; }
	};

	class FsmPlPosCtrActionStartHooks : public Mod
	{
	private:
		void init_check_box_info() override
		{
			m_check_box_name = m_prefix_check_box_name + std::string(get_name());
			m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
		}

		std::unique_ptr<FunctionHook> m_is_air_hook;
		std::unique_ptr<FunctionHook> m_is_overwrite_ground_fit_lenght;
		std::unique_ptr<FunctionHook> m_move_speed;

		enum JmpBehaviour : short
		{
			RetJmp,
			Je_Jne,
			Default
		};

		static inline Events::Event<std::shared_ptr<PosCtrlSpeedEventArgs>> moveSpeedChangeEvent;

	public:
		FsmPlPosCtrActionStartHooks() = default;

		template<typename T>
		static void move_speed_change_sub(std::shared_ptr<Events::EventHandler<T, std::shared_ptr<PosCtrlSpeedEventArgs>>> handler)
		{
			moveSpeedChangeEvent.subscribe(handler);
		}

		template<typename T>
		static void move_speed_change_unsub(std::shared_ptr<Events::EventHandler<T, std::shared_ptr<PosCtrlSpeedEventArgs>>> handler)
		{
			moveSpeedChangeEvent.unsubscribe(handler);
		}

		static inline uintptr_t isAirRet = 0;
		static inline uintptr_t isAirJe = 0;
		static inline uintptr_t overwriteGroundFitRet = 0;
		static inline uintptr_t overwriteGroundFitJe = 0;
		static inline uintptr_t moveSpeedRet = 0;

		static naked void is_air_detour()
		{
			__asm {
				push rax
				push rbx
				push rcx
				push rdx
				push rsp
				push r8
				push r9
				push r10
				push r11
				mov rcx, rdi
				sub rsp, 32
				call qword ptr[FsmPlPosCtrActionStartHooks::is_air_asm]
				add rsp, 32
				pop r11
				pop r10
				pop r9
				pop r8
				pop rsp
				pop rdx
				pop rcx
				pop rbx
				cmp ax, JmpBehaviour::Je_Jne
				je je_ret_pop
				cmp ax, JmpBehaviour::Default
				je originalcode
				pop rax
				jmp qword ptr[FsmPlPosCtrActionStartHooks::isAirRet]

				originalcode:
				pop rax
				cmp[rdi + 0x64], bl
				je je_ret
				jmp qword ptr[FsmPlPosCtrActionStartHooks::isAirRet]

				je_ret_pop :
				pop rax

				je_ret :
				jmp qword ptr[FsmPlPosCtrActionStartHooks::isAirJe]
			}
		}

		static naked void ground_fit_detour()
		{
			__asm {
				push rax
				push rbx
				push rcx
				push rdx
				push rsp
				push r8
				push r9
				push r10
				push r11
				mov rcx, rdi
				sub rsp, 32
				call qword ptr[FsmPlPosCtrActionStartHooks::ground_fit_asm]
				add rsp, 32
				pop r11
				pop r10
				pop r9
				pop r8
				pop rsp
				pop rdx
				pop rcx
				pop rbx
				cmp ax, JmpBehaviour::Je_Jne
				je je_ret_pop
				cmp ax, JmpBehaviour::Default
				je originalcode
				pop rax
				jmp qword ptr[FsmPlPosCtrActionStartHooks::overwriteGroundFitRet]

				originalcode:
				pop rax
				cmp byte ptr[rdi + 0x7D], 00
				je je_ret
				jmp qword ptr[FsmPlPosCtrActionStartHooks::overwriteGroundFitRet]

				je_ret_pop :
				pop rax

				je_ret :
				jmp qword ptr[FsmPlPosCtrActionStartHooks::overwriteGroundFitJe]
			}
		}

		static naked void move_speed_detour()
		{
			__asm {
				test byte ptr[rdi + 0x40], 0x8//?
				seta al

				push rax
				push rbx
				push rcx
				push rdx
				push rdi
				push rsp
				push r8
				push r9
				push r10
				push r11
				mov rcx, rdi
				sub rsp, 32
				call qword ptr[FsmPlPosCtrActionStartHooks::speed_edit_asm]
				add rsp, 32
				pop r11
				pop r10
				pop r9
				pop r8
				pop rsp
				pop rdi
				pop rdx
				pop rcx
				pop rbx
				pop rax
				jmp qword ptr[FsmPlPosCtrActionStartHooks::moveSpeedRet]
			}
		}

		static JmpBehaviour is_air_asm(uintptr_t posCtrAction)
		{
			JmpBehaviour res = Default;
			auto charPtr = *(uintptr_t*)(posCtrAction + 0x28);
			if (AirTrickDodge::move_check_asm(charPtr, AirTrickDodge::isNoInertia))
				res = RetJmp;
			return res;
		}

		static JmpBehaviour ground_fit_asm(uintptr_t posCtrAction)
		{
			JmpBehaviour res = Default;
			auto charPtr = *(uintptr_t*)(posCtrAction + 0x28);
			if (AirTrickDodge::move_check_asm(charPtr, AirTrickDodge::isOverwriteGroundFitLength))
				res = Je_Jne;
			return res;
		}

		static void speed_edit_asm(uintptr_t posControllerAction)
		{
			auto arg = std::make_shared<PosCtrlSpeedEventArgs>(posControllerAction);
			moveSpeedChangeEvent.invoke(arg);
		}

		std::string_view get_name() const override
		{
			return "FsmPlPosCtrActionStartHooks";
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

			auto isAirAddr = m_patterns_cache->find_addr(base, "38 5F 64 74 4A"); //DevilMayCry5.exe+63794F
			if (!isAirAddr)
			{
				return "Unanable to find FsmPlPosCtrActionStartHooks.isInAirAddr pattern.";
			}

			auto isGroundFitAddr = m_patterns_cache->find_addr(base, "80 7F 7D 00 0F 84 93 00 00 00"); //DevilMayCry5.exe+637E7F
			if (!isGroundFitAddr)
			{
				return "Unanable to find FsmPlPosCtrActionStartHooks.isGroundFitAddr pattern.";
			}

			auto moveSpeedAddr = m_patterns_cache->find_addr(base, "F6 47 40 08 0F 97 C0"); //DevilMayCry5.exe+633B0C
			if (!moveSpeedAddr)
			{
				return "Unanable to find FsmPlPosCtrActionStartHooks.moveSpeedAddr pattern.";
			}

			isAirJe = isAirAddr.value() + 0x4F;
			overwriteGroundFitJe = isGroundFitAddr.value() + 0x9D;

			if (!install_hook_absolute(isAirAddr.value(), m_is_air_hook, &is_air_detour, &isAirRet, 0x5))
			{
				spdlog::error("[{}] failed to initialize", get_name());
				return "Failed to initialize FsmPlPosCtrActionStartHooks.isAir";
			}

			if (!install_hook_absolute(isGroundFitAddr.value(), m_is_overwrite_ground_fit_lenght, &ground_fit_detour, &overwriteGroundFitRet, 0xA))
			{
				spdlog::error("[{}] failed to initialize", get_name());
				return "Failed to initialize FsmPlPosCtrActionStartHooks.isGroundFit";
			}

			if (!install_hook_absolute(moveSpeedAddr.value(), m_move_speed, &move_speed_detour, &moveSpeedRet, 0x7))
			{
				spdlog::error("[{}] failed to initialize", get_name());
				return "Failed to initialize FsmPlPosCtrActionStartHooks.moveSpeed";
			}

			return Mod::on_initialize();
		}
	};
}


//clang-format on