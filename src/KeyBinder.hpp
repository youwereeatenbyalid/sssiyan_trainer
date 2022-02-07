#pragma once

#include <string>
#include <functional>
#include <mutex>
#include <vector>
#include <dinput.h>
#include <memory.h>
#include <array>
#include <map>
#include <optional>
#include "utility/Config.hpp"
#include "Config.hpp"
#include "utility/FunctionHook.hpp"

#include "sdk/ReClass.hpp"

typedef uint32_t REGPK_Flag;

#define REGPK_None			0x00000000
#define REGPK_LUp			(1 << 0)
#define REGPK_LDown			(1 << 1)
#define REGPK_LLeft			(1 << 2)
#define REGPK_LRight		(1 << 3)
#define REGPK_RUp			(1 << 4)
#define REGPK_RDown			(1 << 5)
#define REGPK_RLeft			(1 << 6)
#define REGPK_RRight		(1 << 7)
#define REGPK_LTrigTop		(1 << 8)
#define REGPK_LTrigBottom	(1 << 9)
#define REGPK_RTrigTop		(1 << 10)
#define REGPK_RTrigBottom	(1 << 11)
#define REGPK_LStickPush	(1 << 12)
#define REGPK_RStickPush	(1 << 13)
#define REGPK_CLeft			(1 << 14)
#define REGPK_CRight		(1 << 15)
#define REGPK_CCenter		(1 << 16)
#define REGPK_Decide		(1 << 17)
#define REGPK_Cancel		(1 << 18)
#define REGPK_PlatformHome	(1 << 19)
#define REGPK_EmuLup		(1 << 20)
#define REGPK_EmuLright		(1 << 21)
#define REGPK_EmuLdown		(1 << 22)
#define REGPK_EmuLleft		(1 << 23)
#define REGPK_EmuRup		(1 << 24)
#define REGPK_EmuRright		(1 << 25)
#define REGPK_EmuRdown		(1 << 26)
#define REGPK_EmuRleft		(1 << 27)
#define REGPK_LSL			(1 << 28)
#define REGPK_LSR			(1 << 29)
#define REGPK_RSL			(1 << 30)
#define REGPK_RSR			(1 << 31)
#define REGPK_All			0xFFFFFFFF

enum OnState_ : uint8_t {
	OnState_Null,
	OnState_Press,
	OnState_Hold,
	OnState_Release
};

enum KMFlag_ : uint8_t {
	KMFlag_None,
	KMFlag_OnlyNonModifier,
	KMFlag_OnlyModifier
};

class KeyBinder
{
public:
	KeyBinder() = default;
	KeyBinder(KeyBinder const&) = delete;
	KeyBinder& operator=(const KeyBinder&) = delete;
	~KeyBinder() = default;

	static auto Get(){
		if (!m_instance)
			m_instance = std::make_shared<KeyBinder>();

		return m_instance;
	}

	static void ResetInstance() {
		m_instance.reset();
	}
 
public:
	// Keyboard

	static inline bool IsModifierKey(const uint8_t& key);
	static std::string_view DIKToS(const uint8_t& key);
	static uint8_t SToDIK(const std::string_view key);

	// Controller

	static std::string_view XboxCtrlrKToS(const REGPK_Flag& key);
	static std::string_view DSCtrlrKToS(const REGPK_Flag& key);
	static REGPK_Flag SToXboxCtrlrK(const std::string_view& key);
	static REGPK_Flag SToDSCtrlrK(const std::string_view& key);

public:
	using KBKeyList = std::vector<uint8_t>;
	using DIKeyList = std::array<uint8_t, 256>;

	using OptFunc = std::optional<std::function<void()>>;

	struct Keys_t
	{
		KBKeyList keyboard;
		REGPK_Flag controller;
	};

	static void AddBind(const std::string& name, const OptFunc& pressFunc, const OptFunc& holdFunc, const OptFunc& releaseFunc, Keys_t keys = { {}, REGPK_None });
	static void AddBind(const std::string& name, const OptFunc& func, Keys_t keys = { {}, REGPK_None });
	static void AddBind(const std::string& name, const OptFunc& func, const OnState_& keyState, Keys_t keys = { {}, REGPK_None });

	static void RemoveBind(const std::string& name);

	static std::vector<std::string> GetDuplicates();

	static void AcknowledgeInput() { m_isIgnoring = false; }
	static void IgnoreInput() { m_isIgnoring = true; }

	static std::string_view GetConfigName() {
		return m_config_file;
	}

	static void SetConfigName(std::string_view newConfigName) {
		m_config_file = newConfigName;
	}

	static bool SaveConfig() {
		return m_config.save();
	}

	static bool LoadConfig() {
		return m_config.load();
	}

	static bool SaveAllBinds(bool updateFile = false);
	static bool LoadAllBinds(bool refreshFromFile = false, bool clearIfNotFound = false);

	static bool SaveBind(const std::string& name, bool updateFile = false);
	static bool LoadBind(const std::string& name, bool refreshFromFile = false, bool clearIfNotFound = false);

	utility::Config GetConfig() const {
		return m_config;
	}

	/*template <typename T>
	std::optional<T&> GetVar(std::string name);

	template <typename T>
	void SetVar(std::string name, T newValue);*/

	static void SetName(const std::string& currentName, const std::string& newName);

	static std::optional<std::function<void()>> GetFunc(const std::string& name, OnState_ keyState);

	static void SetFunc(const std::string& name, const OptFunc& newFunc, OnState_ keyState);

	static size_t GetBindCount() {
		return m_bindCount;
	}

public: // Keyboard
	static void OnKeyboardUpdate(const DIKeyList& kbState);

	static void AddBind(const std::string& name, const OptFunc& pressFunc, const OptFunc& holdFunc, const OptFunc& releaseFunc, const KBKeyList& keys);
	static void AddBind(const std::string& name, const OptFunc& pressFunc, const OptFunc& holdFunc, const OptFunc& releaseFunc, const uint8_t& key);
	static void AddBind(const std::string& name, const OptFunc& func, const KBKeyList& keys);
	static void AddBind(const std::string& name, const OptFunc& func, const uint8_t& key);
	static void AddBind(const std::string& name, const OptFunc& func, const OnState_& keyState, const KBKeyList& keys);
	static void AddBind(const std::string& name, const OptFunc& func, const OnState_& keyState, const uint8_t& key);

	static std::vector<std::string> GetKBBindsOf(const KBKeyList& keys);

	static const auto& GetKeyboardState() { return m_lastKBState; }

	static KBKeyList GetKeyboardKeys(const std::string& name, KMFlag_ keyModeFlag = KMFlag_None);

	static std::vector<std::string> GetKeyboardKeysStr(const std::string& name, KMFlag_ keyModeFlag = KMFlag_None);

	static void SetKBKeys(const std::string& name, const KBKeyList& newDIKeys);

	static void RemoveDuplicatesOf(const KBKeyList& keys, const std::string& keep);

public: // Controller
	static void OnGamePadUpdate(const HIDGamePadDevice& controllerState);

	static void AddBind(const std::string& name, const OptFunc& pressFunc, const OptFunc& holdFunc, const OptFunc& releaseFunc, const REGPK_Flag& controllerKeys);
	static void AddBind(const std::string& name, const OptFunc& pressFunc, const OptFunc& holdFunc, const OptFunc& releaseFunc, const std::vector<REGPK_Flag>& controllerKeys);
	static void AddBind(const std::string& name, const OptFunc& func, const REGPK_Flag& controllerKeys);
	static void AddBind(const std::string& name, const OptFunc& func, const std::vector<REGPK_Flag>& controllerKeys);
	static void AddBind(const std::string& name, const OptFunc& func, const OnState_& keyState, const REGPK_Flag& controllerKeys);
	static void AddBind(const std::string& name, const OptFunc& func, const OnState_& keyState, const std::vector<REGPK_Flag>& controllerKeys);

	static std::vector<std::string> GetBindsOf(const REGPK_Flag& keys);

	static std::vector<REGPK_Flag> GetControllerKeys(const std::string& name);
	static std::vector<REGPK_Flag> GetControllerKeys(const REGPK_Flag& keys);

	static std::vector<std::string> GetXboxConterllerKeysStr(const std::string& name);
	static std::vector<std::string> GetXboxConterllerKeysStr(const std::vector<REGPK_Flag>& keys);

	static std::vector<std::string> GetDSConterllerKeysStr(const std::string& name);
	static std::vector<std::string> GetDSConterllerKeysStr(const std::vector<REGPK_Flag>& keys);

	static void SetGPKeys(const std::string& name, const REGPK_Flag& controllerKeys);

	static void RemoveDuplicatesOf(const REGPK_Flag& keys, const std::string& keep);

	static const auto& GPKeyBitToListArray() { return m_lastControllerState; }
	static REGPK_Flag GPKeyListArrayToBit(const std::vector<REGPK_Flag>& keys);

private:
	static std::shared_ptr<KeyBinder> m_instance;

private:
	static std::string_view m_config_file;

	static std::mutex m_inputMutex;
	
	static utility::Config m_config;

	struct Binding;

	static inline void UpdateBindCount();
	static OnState_ GetKeyboardKeyState(const Binding& binding, const DIKeyList& keys, const DIKeyList& lasKeys);
	static bool IsAnyModifierSet(const DIKeyList& keys);

	static OnState_ GetControllerKeyState(const Binding& binding, const REGPK_Flag& controllerKeys, const REGPK_Flag& lastControllerKeys);

private:
	struct Binding {
		Binding(Binding&& other);

		// Keyboard
		Binding(const std::string& name, const OptFunc& pressFunc, const OptFunc& holdFunc, const OptFunc& releaseFunc, const KBKeyList& keys = {}, const std::optional<KBKeyList>& modifierKeys = {}, const REGPK_Flag& controllerKeys = REGPK_None);
		Binding(const std::string& name, const OptFunc& func, const OnState_& keyState, const KBKeyList& keys = {}, const std::optional<KBKeyList>& modifierKeys = {}, const REGPK_Flag& controllerKeys = REGPK_None);

		Binding& operator=(Binding&& other);

		// Keyboard
		bool HasModifier() const;
		bool DoKeysMatch(const DIKeyList& kbState) const;
		bool DoKeysMatch(const KBKeyList& keyList) const;

		// Controller
		bool DoKeysMatch(const REGPK_Flag& controllerState) const;

		bool IsModifierSet(const DIKeyList& kbState) const;

		std::string m_name;

		// Keyboard
		KBKeyList m_keyboardKeys;
		std::optional<KBKeyList> m_modifierKeys{}; // Shift Ctrl Alt

		// Controller
		REGPK_Flag m_controllerKeys{};

		OptFunc m_pressFunc{};
		OptFunc m_holdFunc{};
		OptFunc m_releaseFunc{};
	};

	// Keyboard
	static DIKeyList m_lastKBState;

	// Controller
	static REGPK_Flag m_lastControllerState;

	static std::map<std::string, std::shared_ptr<Binding>> m_bindings;
	static std::vector<std::shared_ptr<Binding>> m_bindingsVec;
	static size_t m_bindCount;
	static bool m_isIgnoring;
};

/*
template <typename T>
std::optional<T&> KeyBind::GetVar(std::string name)
{
	auto const& search = m_bindings.find(name);

	if (search == m_bindings.end())
		return {};

	return search->second.GetVar<T>();
}

template <typename T>
void KeyBind::SetVar(std::string name, T newValue)
{
	auto const& search = m_bindings.find(name);

	if (search == m_bindings.end())
		return;

	search->second.GetVar<T>() = newValue;
}*/

extern KeyBinder g_keyBinds;