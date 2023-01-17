#pragma once
#include "Mod.hpp"
#include "sdk/DMC5.hpp"
#include "GameFunctions/GameFunc.hpp"

//clang-format off

namespace gf = GameFunctions;

class InputSystem : public Mod
{
public:
	enum InputMode
	{
		Pad,
		MouseAndKey
	};

	enum class GamePadButton : uint32_t
	{
		LUp = 1,
		LDown = 2,
		LLeft = 4,
		LRight = 8,
		RUp = 16,
		RDown = 32,
		RLeft = 64,
		RRight = 128,
		LTrigTop = 256,
		LTrigBottom = 512,
		RTrigTop = 1024,
		RTrigBottom = 2048,
		LStickPush = 4096,
		RStickPush = 8192,
		CLeft = 16384,
		CRight = 32768,
		CCenter = 65536,
		Decide = 131072,
		Cancel = 262144,
		PlatformHome = 524288,
		EmuLup = 1048576,
		EmuLright = 2097152,
		EmuLdown = 4194304,
		EmuLleft = 8388608,
		EmuRup = 16777216,
		EmuRright = 33554432,
		EmuRdown = 67108864,
		EmuRleft = 134217728,
		LSL = 268435456,
		LSR = 536870912,
		RSL = 1073741824,
		RSR = 2147483648,
		None = 0,
		All = 0xFFFFFFFF,
	};

	enum class PadInputGameAction
	{
		AttackS,
		AttackL,
		Jump,
		Provoke,
		LockOn,
		ChangeLockOn,
		Dash,
		Avoid,
		ChgStyleTS,
		ChgStyleRG,
		ChgStyleGS,
		chgStyleSM,
		Special0,
		Special1,
		Special2,
		CameraReset,
		DevilTrigger,
		Cross,
		MoveUp,
		MoveDown,
		MoveLeft,
		MoveRight,
		CameraUp,
		CameraDown,
		CameraLeft,
		CameraRight,
		MaxAction
	};

private:

	void init_check_box_info() override
	{
		m_check_box_name = m_prefix_check_box_name + std::string(get_name());
		m_hot_key_name = m_prefix_hot_key_name + std::string(get_name());
	}

	REManagedObject *HIDManager = nullptr;
	REManagedObject *PadManager = nullptr;
	REManagedObject *_padInput = nullptr;
	GamePadButton *_padButton = nullptr;
	GamePadButton* _padButtonDown = nullptr;
	GamePadButton* _padButtonUp = nullptr;
	InputMode *_inputMode = nullptr;
	REManagedObject *_assignList = nullptr;
	REManagedObject *_keyAssignArray = nullptr;

	sdk::REMethodDefinition* _padInputIsKeyboardDownMethod = nullptr;
	sdk::REMethodDefinition* _isBackInputMethod = nullptr;
	sdk::REMethodDefinition* _isFrontInputMethod = nullptr;
	sdk::REMethodDefinition* _isLeftInputMethod = nullptr;
	sdk::REMethodDefinition* _isRightInputMethod = nullptr;

	inline friend GamePadButton operator |(GamePadButton a, GamePadButton b)
	{
		return static_cast<GamePadButton>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline friend GamePadButton operator &(GamePadButton a, GamePadButton b)
	{
		return static_cast<GamePadButton>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}

	bool is_action_button_pressed(PadInputGameAction gameAction, GamePadButton curButton, uintptr_t keyAssignArray, bool ignoreCharKeyBind = false) const
	{
		if (HIDManager == nullptr || PadManager == nullptr)
			throw std::exception("HIDManager or PadManager was nullptr.");
		uintptr_t item;
		for (int i = 0; i < gf::ListController::get_list_count((uintptr_t)_assignList); i++)
		{
			item = gf::ListController::get_item<uintptr_t>((uintptr_t)_assignList, i);
			if (*(PadInputGameAction*)(item + 0x14) == gameAction)
			{
				auto btn = ignoreCharKeyBind ? *(GamePadButton*)(item + 0x10) : gf::ListController::get_array_item<GamePadButton>(keyAssignArray, i);
				return ((curButton & btn) == btn);
			}
		}
		return false;
	}

	bool get_assign_data(uintptr_t padInput, uintptr_t& keyAssignArrayOut) const
	{
		keyAssignArrayOut = *(uintptr_t*)(padInput + 0x88);
		if (keyAssignArrayOut == 0)
			return false;
		return true;
	}
	
public:
	InputSystem() = default;

	std::string_view get_name() const override
	{
		return "InputSystem";
	}
	std::string get_checkbox_name() override
	{
		return m_check_box_name;
	};
	std::string get_hotkey_name() override
	{
		return m_hot_key_name;
	};

	InputMode get_input_mode() const
	{
		if(_inputMode != nullptr)
			return *_inputMode;
		throw std::exception("_inputMode was nullptr");
	}

	GamePadButton get_pad_button() const
	{
		if(_padButton != nullptr)
			return *_padButton;
		throw std::exception("_padButton was nullptr");
	}

	GamePadButton get_pad_button(uintptr_t padInput) const
	{
		if(padInput == 0)
			throw std::exception("_padButton was nullptr");
		return *(GamePadButton*)(padInput + 0x98);
	}

	GamePadButton get_pad_button_down() const
	{
		if (_padButton != nullptr)
			return *_padButtonDown;
		throw std::exception("_padButton was nullptr");
	}

	GamePadButton get_pad_button_down(uintptr_t padInput) const
	{
		if (padInput == 0)
			throw std::exception("padInput was nullptr");
		return *(GamePadButton*)(padInput + 0x90);
	}

	GamePadButton get_pad_button_up() const
	{
		if (_padButton != nullptr)
			return *_padButtonUp;
		throw std::exception("padButton was nullptr");
	}

	GamePadButton get_pad_button_up(uintptr_t padInput) const
	{
		if (padInput == 0)
			throw std::exception("padInput was nullptr");
		return *(GamePadButton*)(padInput + 0x94);
	}

	REManagedObject* get_pad_input() const noexcept { return _padInput; }

	REManagedObject* get_HIDManager() const noexcept { return HIDManager; }

	REManagedObject* get_PadManager() const noexcept { return PadManager; }

	//Uses padInput from PadMaanger singleton
	//ignoreCharKeyBind - (only for gamepad input) different character can treat action buttons by a different way. For example all dpad actions like "ChgStyleTS", "ChgStyleRG" is "DevilTrigger" for pl0800. 
	//Set this to "true" to ignore special action binding treatment.
	bool is_action_button_pressed(PadInputGameAction gameAction, bool ignoreCharKeyBind = false) const
	{
		if (*_inputMode == MouseAndKey && _padInputIsKeyboardDownMethod != nullptr)
			return _padInputIsKeyboardDownMethod->call<bool>(sdk::get_thread_context(), _padInput, gameAction);
		return is_action_button_pressed(gameAction, *_padButton, (uintptr_t)_keyAssignArray, ignoreCharKeyBind);
	}

	bool is_action_button_pressed(uintptr_t padInput, PadInputGameAction gameAction, bool ignoreCharKeyBind = false) const
	{
		if (*_inputMode == MouseAndKey && _padInputIsKeyboardDownMethod != nullptr)
			return _padInputIsKeyboardDownMethod->call<bool>(sdk::get_thread_context(), padInput, gameAction);
		uintptr_t keyAssignArray = 0;
		if (!get_assign_data(padInput, keyAssignArray))
			return false;
		return is_action_button_pressed(gameAction, *(GamePadButton*)(padInput + 0x98), keyAssignArray, ignoreCharKeyBind);
	}

	bool is_action_button_down(PadInputGameAction gameAction, bool ignoreCharKeyBind = false) const
	{
		return is_action_button_pressed(gameAction, *_padButtonDown, (uintptr_t)_keyAssignArray);
	}

	bool is_action_button_down(uintptr_t padInput, PadInputGameAction gameAction, bool ignoreCharKeyBind = false) const
	{
		uintptr_t assignList = 0, keyAssignArray = 0;
		if (!get_assign_data(padInput, keyAssignArray))
			return false;
		return is_action_button_pressed(gameAction, *(GamePadButton*)(padInput + 0x90), keyAssignArray);
	}

	bool is_action_button_up(PadInputGameAction gameAction, bool ignoreCharKeyBind = false) const
	{
		return is_action_button_pressed(gameAction, *_padButtonUp, (uintptr_t)_keyAssignArray, ignoreCharKeyBind);
	}

	bool is_action_button_up(uintptr_t padInput, PadInputGameAction gameAction, bool ignoreCharKeyBind = false) const
	{
		uintptr_t assignList = 0, keyAssignArray = 0;
		if (!get_assign_data(padInput, keyAssignArray))
			return false;
		return is_action_button_pressed(gameAction, *(GamePadButton*)(padInput + 0x94), keyAssignArray, ignoreCharKeyBind);
	}

	//-----------------------------------------------------------Left stick inputs-------------------------------------------------------------//

	bool is_back_input(float deg)
	{
		return _isBackInputMethod->call<bool>(sdk::get_thread_context(), _padInput, deg);
	}

	bool is_back_input(uintptr_t threadCtxt, float deg)
	{
		return _isBackInputMethod->call<bool>(threadCtxt, _padInput, deg);
	}

	bool is_front_input(float deg)
	{
		return _isFrontInputMethod->call<bool>(sdk::get_thread_context(), _padInput, deg);
	}

	bool is_front_input(uintptr_t threadCtxt, float deg)
	{
		return _isFrontInputMethod->call<bool>(threadCtxt, _padInput, deg);
	}

	bool is_back_input(REManagedObject* padInput, float deg)
	{
		return _isBackInputMethod->call<bool>(sdk::get_thread_context(), padInput, deg);
	}

	bool is_back_input(uintptr_t threadCtxt, REManagedObject* padInput, float deg)
	{
		return _isBackInputMethod->call<bool>(threadCtxt, padInput, deg);
	}

	bool is_front_input(REManagedObject* padInput, float deg)
	{
		return _isFrontInputMethod->call<bool>(sdk::get_thread_context(), padInput, deg);
	}

	bool is_front_input(uintptr_t threadCtxt, REManagedObject* padInput, float deg)
	{
		return _isFrontInputMethod->call<bool>(threadCtxt, padInput, deg);
	}

	bool is_left_input(float deg)
	{
		return _isLeftInputMethod->call<bool>(sdk::get_thread_context(), _padInput, deg);
	}

	bool is_left_input(uintptr_t threadCtxt, float deg)
	{
		return _isLeftInputMethod->call<bool>(threadCtxt, _padInput, deg);
	}

	bool is_left_input(REManagedObject* padInput, float deg)
	{
		return _isLeftInputMethod->call<bool>(sdk::get_thread_context(), padInput, deg);
	}

	bool is_left_input(uintptr_t threadCtxt, REManagedObject* padInput, float deg)
	{
		return _isLeftInputMethod->call<bool>(threadCtxt, padInput, deg);
	}

	bool is_right_input(float deg)
	{
		return _isRightInputMethod->call<bool>(sdk::get_thread_context(), _padInput, deg);
	}

	bool is_right_input(uintptr_t threadCtxt, float deg)
	{
		return _isRightInputMethod->call<bool>(threadCtxt, _padInput, deg);
	}

	bool is_right_input(REManagedObject* padInput, float deg)
	{
		return _isRightInputMethod->call<bool>(sdk::get_thread_context(), padInput, deg);
	}

	bool is_right_input(uintptr_t threadCtxt, REManagedObject* padInput, float deg)
	{
		return _isRightInputMethod->call<bool>(threadCtxt, padInput, deg);
	}

	//----------------------------------------------------------------------------------------------------------------------------------//

	std::optional<std::string> on_initialize() override
	{
		//init_check_box_info();
		auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
		m_on_page = Page_None;
		//m_full_name_string = "";
		m_author_string = "V.P.Zadov";
		//m_description_string = "";

		HIDManager = reframework::get_globals()->get("app.HIDManager");
		PadManager = reframework::get_globals()->get("app.PadManager");
		if (HIDManager != nullptr && PadManager != nullptr)
		{
			_padInput = (REManagedObject*)(*(uintptr_t*)((uintptr_t)PadManager + 0x60));
			_padButton = (GamePadButton*)((uintptr_t)_padInput + 0x98);
			_padButtonDown = (GamePadButton*)((uintptr_t)_padInput + 0x90);
			_padButtonUp = (GamePadButton*)((uintptr_t)_padInput + 0x94);
			_keyAssignArray = (REManagedObject*)(*(uintptr_t*)((uintptr_t)_padInput + 0x88));
			_inputMode = (InputMode*)((uintptr_t)HIDManager + 0x54);
			auto tmp = *(uintptr_t*)((uintptr_t)PadManager + 0x58);
			if(tmp != 0)
				_assignList = (REManagedObject*)(*(uintptr_t*)(tmp + 0x30));
			_padInputIsKeyboardDownMethod = sdk::find_method_definition("app.PadInput", "isKeyboardDown(app.PadInput.GameAction)");
			_isBackInputMethod = sdk::get_object_method(_padInput, "isBackInput(System.Single)");
			_isFrontInputMethod = sdk::get_object_method(_padInput, "isFrontInput(System.Single)");
			_isLeftInputMethod = sdk::get_object_method(_padInput, "isLeftInput(System.Single)");
			_isRightInputMethod = sdk::get_object_method(_padInput, "isRightInput(System.Single)");
		}

		return Mod::on_initialize();
	}
};
//clang-format on