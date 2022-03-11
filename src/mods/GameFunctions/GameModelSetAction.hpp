#include "GameFunc.hpp"

namespace GameFunctions
{
	class GameModelSetAction : public GameFunc<void>
	{
	public:
		enum InterpolationMode
		{
			None,
			FrontFade,
			CrossFade,
			SyncCrossFade,
			SyncPointCrossFade
		};

		enum InterpolationCurve
		{
			Linear,
			Smooth,
			Easeln,
			EaseOut
		};

		enum ActionPriority
		{
			Normal,
			Death
		};

	private:
		uintptr_t gameModel;

		typedef void (_cdecl* f_set_action)(uintptr_t treadVM, uintptr_t gameModel, void *nameString, int layerNo, float startFrame, float interpolationFrame, InterpolationMode mode, 
		InterpolationCurve curve, bool isImmediate, bool passSelect, bool isPuppetTransition, ActionPriority priority);
		f_set_action set_action;

		void invoke() override { }
		void operator()() override { }

	protected:
		GameModelSetAction() { }

	public:
		GameModelSetAction(uintptr_t gameModel)
		{
			this->gameModel = gameModel;
			fAddr += 0x12EC360;
			set_action = (f_set_action)fAddr;
		}

		void invoke(void* nameString, int layerNo, float startFrame, float interpolationFrame, InterpolationMode mode,
			InterpolationCurve curve, bool isImmediate, bool passSelect, bool isPuppetTransition, ActionPriority priority) noexcept
		{
			if(!utility::isGoodReadPtr(gameModel, 8))
				return;
			set_action(get_thread_context(), gameModel, nameString, layerNo, startFrame, interpolationFrame, mode, curve, isImmediate, passSelect, isPuppetTransition, priority);
		}

		void operator()(void* nameString, int layerNo, float startFrame, float interpolationFrame, InterpolationMode mode,
			InterpolationCurve curve, bool isImmediate, bool passSelect, bool isPuppetTransition, ActionPriority priority) noexcept
		{
			return invoke(nameString, layerNo, startFrame, interpolationFrame, mode, curve, isImmediate, passSelect, isPuppetTransition, priority);
		}
	};

	class PlVergilSetAction : public GameModelSetAction
	{
	public:
	private:
		uintptr_t pl;

		typedef void(_cdecl* f_vergil_set_action)(uintptr_t treadVM, uintptr_t plVergil, int action, int layerNo, float startFrame, float interpolationFrame, InterpolationMode mode,
			InterpolationCurve curve, bool isCommandAction, bool isPuppetTransition);
		f_vergil_set_action vergil_set_action;

	public:
		PlVergilSetAction(uintptr_t playerVergil)
		{
			pl = playerVergil;
			fAddr += 0x5456A0;
			vergil_set_action = (f_vergil_set_action)fAddr;
		}

		void invoke(int action, int layerNo, float startFrame, float interpolationFrame, InterpolationMode mode,
			InterpolationCurve curve, bool isCommandAction, bool isPuppetTransition) noexcept
		{
			if(!utility::isGoodReadPtr(pl, 8))
				return;
			vergil_set_action(get_thread_context(), pl, action, layerNo, startFrame, interpolationFrame, mode, curve, isCommandAction, isPuppetTransition);
		}

		void operator()(int action, int layerNo, float startFrame, float interpolationFrame, InterpolationMode mode,
			InterpolationCurve curve, bool isCommandAction, bool isPuppetTransition)
		{
			invoke(action, layerNo, startFrame, interpolationFrame, mode, curve, isCommandAction, isPuppetTransition);
		}
	};
}