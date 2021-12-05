#pragma once
#include "Mods/GameFunctions/GameFunc.hpp"
#include <optional>
//clang-format off
namespace GameFunctions
{
	class CreateShell : public GameFunc<uintptr_t>
	{
		private:

			struct SoundData
			{
				byte mngOffs[0xF] = "";
				std::string* triggerName = new std::string("JudgementCut");
				Vec3 position;
				~SoundData()
				{
					delete triggerName;
				}
			};

			struct DelayParam
			{
				byte mngOffs[0xF] = "";
				float frame = 0.228f;
				void* lockOnTarget = nullptr;//LockOnTargetWork
				SoundData* sound = new SoundData();
				Vec3 initCollisionCheckPos;

				~DelayParam()
				{
					delete sound;
				}
			};

		
		//typedef uintptr_t(__cdecl* spawnShell)(void* rcxArg, void* shellManager, void* prefab, const Vec3 &pos, const Quaternion &rotation, void* owner, int level, int id, const void* delayParam/*, void* a5, void* a6, void *a7, void *a8, void* a9*/);
		typedef uintptr_t(__cdecl* f_CreateShell)(uintptr_t rcxArg, uintptr_t shellManager, uintptr_t prefab, Vec3 pos, Quaternion rotation, uintptr_t owner, int &level, int &id, DelayParam* delay);
		f_CreateShell create_shell;

		uintptr_t pfb = 0;
		uintptr_t owner = 0;
		uintptr_t rcx = 0;
		uintptr_t rcxBase = 0x07EC2F60;//0x07E53660;

		Vec3 pos;
		Quaternion rot;
		int lvl = 0;
		int id = 0;

		DelayParam *delay;

		std::mutex mtx;

		void* pRcx;

		public:
		const std::array<uintptr_t, 5> fancyRcx {0x70, 0x50, 0x408, 0x80, 0x0}; //{0x498, 0x60, 0xC0, 0x0};
		//uintptr_t owner;

		CreateShell()
		{
			fAddr +=  0x1B0A400;
			create_shell = (f_CreateShell)fAddr;
			rcxBase += g_framework->get_module().as<uintptr_t>();
			delay = nullptr;
		}

		CreateShell(uintptr_t prefab) : CreateShell()
		{
			pfb = prefab;
		}

		std::optional<uintptr_t> get_rcx_arg_ptr()
		{
			bool isBadPtr = false;
			auto ptrAddr = GameFunctions::PtrController::get_ptr<uintptr_t>(rcxBase, fancyRcx, isBadPtr, true);
			return isBadPtr ? std::nullopt : std::optional<uintptr_t>{ptrAddr};
		}

		void set_params(uintptr_t rcxParam, uintptr_t prefab, const Vec3& spawnPos, const Quaternion& spawnRot, uintptr_t owner, int level, int id)
		{
			rcx = rcxParam;
			pfb = prefab;
			pos = spawnPos;
			rot = spawnRot;
			lvl = level;
			this->id = id;
			this->owner = owner;
		}

		void set_rcx(uintptr_t rcxParam) 
		{
			rcx = rcxParam;
			pRcx = (void*)(*(uintptr_t*)rcxParam);
		}

		uintptr_t get_prefab() const {return pfb; }

		uintptr_t get_owner() const {return owner; }

		Vec3 get_spawn_pos() const {return pos; }

		Quaternion get_spawn_rot() const {return rot; }

		uintptr_t get_cur_rcx() const {return rcx; }

		int get_level() const {return lvl; }

		int get_id() const {return id; }

		/// <summary></summary>
		/// <returns>Return app.Shell object</returns>
		uintptr_t invoke() override
		{
			uintptr_t res = 0;
			if (pfb != 0 && fAddr != NULL)
			{
				uintptr_t shellMng = *(uintptr_t*)(g_framework->get_module().as<uintptr_t>() + 0x7E60450);
				if(shellMng == 0)
					return 0;
				if (rcx == 0)
				{
					auto rcxOpt = get_rcx_arg_ptr();
					if(!rcxOpt.has_value())
						return 0;
					else 
						rcx = rcxOpt.value();
				}
				res = create_shell(rcx, shellMng, pfb, pos, rot, owner, lvl, id, delay);
				//delete(delay);
			}
			return res;
		}

		/// <summary></summary>
		/// <returns>Return app.Shell object</returns>
		uintptr_t invoke(uintptr_t rcxParam, uintptr_t prefab, const Vec3& spawnPos, const Quaternion& spawnRot, uintptr_t owner, int level, int id)
		{
			set_params(rcxParam, prefab, spawnPos, spawnRot, owner, level, id);
			return invoke();
		}

		/// <summary></summary>
		/// <returns>Return app.Shell object</returns>
		uintptr_t operator ()(uintptr_t rcxParam, uintptr_t prefab, const Vec3& spawnPos, const Quaternion& spawnRot, uintptr_t owner, int level, int id)
		{
			return invoke(rcxParam, prefab, spawnPos, spawnRot, owner, level, id);
		}

		/// <summary></summary>
		/// <returns>Return app.Shell object</returns>
		uintptr_t invoke(const Vec3 &spawnPos, const Quaternion &spawnRot)
		{
			pos = spawnPos;
			rot = spawnRot;
			return invoke();
		}

		/// <summary></summary>
		/// <returns>Return app.Shell object</returns>
		uintptr_t operator()(const Vec3& spawnPos, const Quaternion& spawnRot)
		{
			return invoke(spawnPos, spawnRot);
		}

	};
}
//clang-format on