#include "GameFunc.hpp"

namespace GameFunctions
{
	/// <summary>
	/// Class to manage setting the action of the game model
	/// </summary>
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
		/// <summary>
		/// Constructor function
		/// </summary>
		/// <param name="gameModel">gamemodel to set actions for</param>
		GameModelSetAction(uintptr_t gameModel)
		{
			this->gameModel = gameModel;

			//DevilMayCry5.app_GameModel__setAction173857 
			//setAction(System.String, System.UInt32, System.Single, System.Single, via.motion.InterpolationMode, via.motion.InterpolationCurve, System.Boolean, System.Boolean, System.Boolean, app.GameModel.ActionPriority)
			//AOB: ( CD part of address ) CC CC CC CC CC CC CC CC CC 48 89 5C 24 08 57 48 83 EC 60 48 8B 41
			//57 48 83 EC 60 48 8B 41 50 48 8B FA 48 8B D9 48 83 78 18 00 0F 85 (9A) (backup AOB in case the first one doesn't work)
			//fAddr += 0x12EC360;
			//set_action = (f_set_action)fAddr;
		}
		/// <summary>
		/// Set the action of the gamemodel.
		/// </summary>
		/// <param name="nameString">Name of action</param>
		/// <param name="layerNo">animation layer to apply action to</param>
		/// <param name="startFrame">start frame of animation</param>
		/// <param name="interpolationFrame">what frame the animation can be interpolated (from or to, not sure which) </param>
		/// <param name="mode">Type of interpolation used.</param>
		/// <param name="curve">Interpolation curve</param>
		/// <param name="isImmediate">Whether to immediately transition to new action(?)</param>
		/// <param name="passSelect">(?)</param>
		/// <param name="isPuppetTransition">if the gamemodel is a network puppet(?)</param>
		/// <param name="priority">ActionPriority (unsure)</param>
		/// <returns></returns>
		void invoke(void* nameString, int layerNo, float startFrame, float interpolationFrame, InterpolationMode mode,
			InterpolationCurve curve, bool isImmediate, bool passSelect, bool isPuppetTransition, ActionPriority priority) noexcept
		{
			if(!utility::isGoodReadPtr(gameModel, 8))
				return;
			sdk::call_object_func_easy<void*>((REManagedObject*)gameModel, "setAction(System.String, System.UInt32, System.Single, System.Single, via.motion.InterpolationMode, via.motion.InterpolationCurve, System.Boolean, System.Boolean, System.Boolean, app.GameModel.ActionPriority)",
				nameString, layerNo, startFrame, interpolationFrame, mode, curve, isImmediate, passSelect, isPuppetTransition, priority);
			//set_action(get_thread_context(), gameModel, nameString, layerNo, startFrame, interpolationFrame, mode, curve, isImmediate, passSelect, isPuppetTransition, priority);
		}
		/// <summary>
		/// operator call for class. Set the action of the gamemodel.
		/// </summary>
		/// <param name="nameString">Name of action</param>
		/// <param name="layerNo">animation layer to apply action to</param>
		/// <param name="startFrame">start frame of animation</param>
		/// <param name="interpolationFrame">what frame the animation can be interpolated (from or to, not sure which) </param>
		/// <param name="mode">Type of interpolation used.</param>
		/// <param name="curve">Interpolation curve</param>
		/// <param name="isImmediate">Whether to immediately transition to new action(?)</param>
		/// <param name="passSelect">(?)</param>
		/// <param name="isPuppetTransition">if the gamemodel is a network puppet(?)</param>
		/// <param name="priority">ActionPriority (unsure)</param>
		/// <returns></returns>
		void operator()(void* nameString, int layerNo, float startFrame, float interpolationFrame, InterpolationMode mode,
			InterpolationCurve curve, bool isImmediate, bool passSelect, bool isPuppetTransition, ActionPriority priority) noexcept
		{
			return invoke(nameString, layerNo, startFrame, interpolationFrame, mode, curve, isImmediate, passSelect, isPuppetTransition, priority);
		}
	};
	/// <summary>
	/// Vergil-Specific SetActionClass
	/// </summary>
	class PlVergilSetAction : public GameModelSetAction
	{
	public:
	private:
		uintptr_t pl;

		typedef void(_cdecl* f_vergil_set_action)(uintptr_t treadVM, uintptr_t plVergil, int action, int layerNo, float startFrame, float interpolationFrame, InterpolationMode mode,
			InterpolationCurve curve, bool isCommandAction, bool isPuppetTransition);
		f_vergil_set_action vergil_set_action;

	public:
		/// <summary>
		/// constructor function, uses vergil specific function call.
		/// </summary>
		/// <param name="playerVergil"></param>
		PlVergilSetAction(uintptr_t playerVergil)
		{
			pl = playerVergil;
			//fAddr += 0x5456A0;
			//48 89 5C 24 20 56 57 41 57 48 83 EC 60
			//DevilMayCry5.app_PlayerVergilPL__setAction113951 
			//setAction(app.PlayerVergilPL.Actions, System.UInt32, System.Single, System.Single, via.motion.InterpolationMode, via.motion.InterpolationCurve, System.Boolean, System.Boolean)
			//vergil_set_action = (f_vergil_set_action)fAddr;
		}
		/// <summary>
		/// Set the action of the PL Vergil Gamemodel.
		/// </summary>
		/// <param name="action">Action ID</param>
		/// <param name="layerNo">animation layer to apply action to</param>
		/// <param name="startFrame">start frame of animation</param>
		/// <param name="interpolationFrame">what frame the animation can be interpolated (from or to, not sure which) </param>
		/// <param name="mode">Type of interpolation used.</param>
		/// <param name="curve">Interpolation curve</param>
		/// <param name="isCommandAction">(?)</param>
		/// <param name="isPuppetTransition">if the gamemodel is a network puppet(?)</param>
		/// <returns></returns>
		void invoke(int action, int layerNo, float startFrame, float interpolationFrame, InterpolationMode mode,
			InterpolationCurve curve, bool isCommandAction, bool isPuppetTransition) noexcept
		{
			if(!utility::isGoodReadPtr(pl, 8))
				return;
			sdk::call_object_func_easy<void*>((REManagedObject*)pl, "setAction("
				"app.PlayerVergilPL.Actions, System.UInt32, System.Single, System.Single, via.motion.InterpolationMode, via.motion.InterpolationCurve, System.Boolean, System.Boolean)",
				action,						layerNo,		startFrame,		interpolationFrame, mode,					curve,							isCommandAction, isPuppetTransition);
			//vergil_set_action(get_thread_context(), pl, action, layerNo, startFrame, interpolationFrame, mode, curve, isCommandAction, isPuppetTransition);
		}
		/// <summary>
		/// operator call for class. Set the action of the PL Vergil Gamemodel.
		/// </summary>
		/// <param name="action">Action ID</param>
		/// <param name="layerNo">animation layer to apply action to</param>
		/// <param name="startFrame">start frame of animation</param>
		/// <param name="interpolationFrame">what frame the animation can be interpolated (from or to, not sure which) </param>
		/// <param name="mode">Type of interpolation used.</param>
		/// <param name="curve">Interpolation curve</param>
		/// <param name="isCommandAction">(?)</param>
		/// <param name="isPuppetTransition">if the gamemodel is a network puppet(?)</param>
		/// <returns></returns>
		void operator()(int action, int layerNo, float startFrame, float interpolationFrame, InterpolationMode mode,
			InterpolationCurve curve, bool isCommandAction, bool isPuppetTransition)
		{
			invoke(action, layerNo, startFrame, interpolationFrame, mode, curve, isCommandAction, isPuppetTransition);
		}
	};
}