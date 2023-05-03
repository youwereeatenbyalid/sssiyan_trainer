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
			fAddr += 0x12EC360;
			set_action = (f_set_action)fAddr;
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
			set_action(get_thread_context(), gameModel, nameString, layerNo, startFrame, interpolationFrame, mode, curve, isImmediate, passSelect, isPuppetTransition, priority);
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
			fAddr += 0x5456A0;
			vergil_set_action = (f_vergil_set_action)fAddr;
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
			vergil_set_action(get_thread_context(), pl, action, layerNo, startFrame, interpolationFrame, mode, curve, isCommandAction, isPuppetTransition);
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