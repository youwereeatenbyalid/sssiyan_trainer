#pragma once
#include <mods/GameFunctions/GameFunc.hpp>
namespace GameFunctions
{
	class Transform_SetPosition : public GameFunc<void>
	{
	private:
		typedef void (__cdecl *f_set_pos)(void *rcx, void *transform, Vec3 newPos);
		f_set_pos set_pos;
		void *transform = nullptr;

		void invoke() override
		{
			throw std::bad_function_call();
		}

		void operator()() override{ invoke(); }

	public:
		Transform_SetPosition()
		{
			fAddr += 0x258BC0;
			set_pos = (f_set_pos)fAddr;
		}

		Transform_SetPosition(void *transformObj) : Transform_SetPosition() 
		{ 
			transform = transformObj;
		}

		void invoke(void *transformObj, Vec3 newPos)
		{
			transform = transformObj;
			invoke(newPos);
		}

		void invoke(Vec3 newPos)
		{
			if(!IsBadReadPtr(transform, 8))
				set_pos(NULL, transform, newPos);
		}

		void operator()(void* transformObj, Vec3 newPos) { invoke(transformObj, newPos); }

		void operator()(Vec3 newPos) { invoke( newPos); }

		void set_transform(void *obj) {transform = obj; }

		void *get_transform() const { return transform; }

		//Change character pos and colliders pos. Only for character obj.
		static void set_character_pos(uintptr_t character, Vec3 newPos, bool moveCollidersOnly = false)
		{
			if(character == 0)
				return;
			auto gameObj = *(uintptr_t*)(character + 0x10);
			auto transformGameObj = *(uintptr_t*)(gameObj + 0x18);
			auto cachedCharController = *(uintptr_t*)(character + 0x2F0);
			auto cachedSubCharController = *(uintptr_t*)(character + 0x2F8);
			if(cachedCharController == 0 || cachedSubCharController == 0)
				return;
			if (!moveCollidersOnly)
			{
				Transform_SetPosition setPos((void*)transformGameObj);
				setPos(newPos);
			}

			*(bool*)(cachedCharController + 0x30) = false;
			*(bool*)(cachedSubCharController + 0x30) = false;

			*(Vec3*)(cachedCharController + 0x140) = newPos;
			*(Vec3*)(cachedCharController + 0x150) = newPos;
			*(Vec3*)(cachedCharController + 0x160) = newPos;

			*(Vec3*)(cachedSubCharController + 0x140) = newPos;
			*(Vec3*)(cachedSubCharController + 0x150) = newPos;
			*(Vec3*)(cachedSubCharController + 0x160) = newPos;

			*(bool*)(cachedCharController + 0x30) = true;
			*(bool*)(cachedSubCharController + 0x30) = true;
		}

	};

	class PositionErrorCorrector : public GameFunc<void>
	{
	private:
		typedef void(__cdecl* f_set_pos)(void* rcx, void* corrector, Vec3 newPos);
		f_set_pos set_pos;
		uintptr_t setPosAddr = 0x11D3FD0;
		
		typedef void(__cdecl* f_stop_internal)(void* rcx, void* corrector);
		f_stop_internal def_stop_internal;
		uintptr_t stopInternalAddr = 0x11D3CA0;

		typedef void(__cdecl* f_restatrt)(void* rcx, void* corrector);
		f_restatrt def_restart;
		uintptr_t restartAddr = 0x11CF640;

		void* corrector = nullptr;

		void invoke() override
		{
			throw std::bad_function_call();
		}

		void operator()() override
		{
			invoke();
		}

		bool check_ptrs()
		{
			if (!IsBadReadPtr((void*)threadContext, 8) && !IsBadReadPtr(corrector, 8))
				return true;
			return false;
		}

		void check_thread_context()
		{
			if (threadContext == 0)
				threadContext = get_thread_context();
		}

		bool full_check()
		{
			check_thread_context();
			return check_ptrs();
		}

	public:
		PositionErrorCorrector()
		{
			setPosAddr += fAddr;
			set_pos = (f_set_pos)setPosAddr;
			stopInternalAddr += fAddr;
			def_stop_internal = (f_stop_internal)stopInternalAddr;
			restartAddr += fAddr;
			def_restart = (f_restatrt)(restartAddr);
			threadContext = get_thread_context();
		}

		PositionErrorCorrector(void* correctorObj) : PositionErrorCorrector()
		{
			corrector = correctorObj;
		}

		void set_corrector(void* obj)
		{
			corrector = obj;
		}

		void set_position(Vec3 newPos)
		{
			threadContext = get_thread_context();
			if(full_check())
				set_pos((void*)threadContext, corrector, newPos);
		}

		void set_position(void *correctorObj, Vec3 newPos)
		{
			corrector = correctorObj;
			set_position(newPos);
		}

		void stop_internal()
		{
			threadContext = get_thread_context();
			if(full_check())
				def_stop_internal((void*)threadContext, corrector);
		}

		void stop_internal(void *correctorObj)
		{
			corrector = correctorObj;
			stop_internal();
		}

		void restart()
		{
			threadContext = get_thread_context();
			if(full_check())
				def_restart((void*)threadContext, corrector);
		}

		void restart(void *correctorObj)
		{
			corrector = correctorObj;
			restart();
		}
	};

	class SetSafePosition : public GameFunc<void>
	{
	private:
		typedef void(__cdecl* f_set_pos)(void* rcx, void* player, Vec3 pos);
		f_set_pos set_safe_pos;

		const ptrdiff_t setSafePosOffs = 0x162DE10;

		void invoke() override { }

		void operator()() override { }

	public:

		SetSafePosition()
		{
			fAddr += setSafePosOffs;
			set_safe_pos = (f_set_pos)fAddr;
		}

		void invoke(void *player, Vec3 pos)
		{
			set_safe_pos(nullptr, player, pos);
		}

		void operator()(void* player, Vec3 pos)
		{
			set_safe_pos(nullptr, player, pos);
		}
	};
}

