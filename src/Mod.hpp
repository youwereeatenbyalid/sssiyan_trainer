#pragma once
// NOTE(): Doing this because clang-cl now gives
// errors when using __declspec(naked) in x64 
#ifdef __clang__
#define naked __attribute__ ((naked))
#else
#define naked __declspec(naked)
#endif

// Game can't use virtual keys unless the menu is open.
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <vector>
#include <unordered_map>
#include <memory>

#include <imgui/imgui.h>

#include <utility/Detour.hpp>
#include "sdk/ReClass.hpp"
#include "sdk/Offsets.hpp"
#include "utility/Config.hpp"
#include "utility/Scan.hpp"
#include "ModFramework.hpp"
#include "InitPatternsManager.hpp"

class IModValue {
public:
    using Ptr = std::unique_ptr<IModValue>;

    virtual ~IModValue() {};
    virtual bool draw(std::string_view name) = 0;
    virtual void draw_value(std::string_view name) = 0;
    virtual void config_load(const utility::Config& cfg) = 0;
    virtual void config_save(utility::Config& cfg) = 0;
};

// Convenience classes for imgui
template <typename T>
class ModValue : public IModValue {
public:
    using Ptr = std::unique_ptr<ModValue<T>>;

    static auto create(std::string_view config_name, T& default_value = T{}) {
        return std::make_unique<ModValue<T>>(config_name, default_value);
    }

    ModValue(std::string_view config_name, T default_value) 
        : m_config_name{ config_name },
        m_value{ default_value }
    {
    }

    virtual ~ModValue() override {};

    virtual void config_load(const utility::Config& cfg) override {
        auto v = cfg.get<T>(m_config_name);

        if (v) {
            m_value = *v;
        }
    };

    virtual void config_save(utility::Config& cfg) override {
        cfg.set<T>(m_config_name, m_value);
    };

    operator T&() {
        return m_value;
    }

    T& value() {
        return m_value;
    }

    const auto& get_config_name() const {
        return m_config_name;
    }

protected:
    T& m_value{};
    std::string m_config_name{ "Default_ModValue" };
};

class ModToggle : public ModValue<bool> {
public:
    using Ptr = std::unique_ptr<ModToggle>;

    ModToggle(std::string_view config_name, bool default_value) 
        : ModValue<bool>{ config_name, default_value } 
    { 
    }

    static auto create(std::string_view config_name, bool default_value = false) {
        return std::make_unique<ModToggle>(config_name, default_value);
    }

    bool draw(std::string_view name) override {
        ImGui::PushID(this);
        auto ret = ImGui::Checkbox(name.data(), &m_value);
        ImGui::PopID();

        return ret;
    }

    void draw_value(std::string_view name) override {
        ImGui::Text("%s: %i", name.data(), m_value);
    }

    bool toggle() {
        return m_value = !m_value;
    }
};

class ModFloat : public ModValue<float> {
public:
    using Ptr = std::unique_ptr<ModFloat>;

    ModFloat(std::string_view config_name, float default_value) 
        : ModValue<float>{ config_name, default_value } { }

    static auto create(std::string_view config_name, float default_value = 0.0f) {
        return std::make_unique<ModFloat>(config_name, default_value);
    }

    bool draw(std::string_view name) override {
        ImGui::PushID(this);
        auto ret = ImGui::InputFloat(name.data(), &m_value);
        ImGui::PopID();

        return ret;
    }

    void draw_value(std::string_view name) override {
        ImGui::Text("%s: %f", name.data(), m_value);
    }
};

class ModSlider : public ModFloat {
public:
    using Ptr = std::unique_ptr<ModSlider>;

    static auto create(std::string_view config_name, float mn = 0.0f, float mx = 1.0f, float default_value = 0.0f) {
        return std::make_unique<ModSlider>(config_name, mn, mx, default_value);
    }

    ModSlider(std::string_view config_name, float mn = 0.0f, float mx = 1.0f, float default_value = 0.0f)
        : ModFloat{ config_name, default_value },
        m_range{ mn, mx }
    {
    }

    bool draw(std::string_view name) override {
        ImGui::PushID(this);
        auto ret = ImGui::SliderFloat(name.data(), &m_value, m_range.x, m_range.y);
        ImGui::PopID();

        return ret;
    }

    void draw_value(std::string_view name) override {
        ImGui::Text("%s: %f [%f, %f]", name.data(), m_value, m_range.x, m_range.y);
    }

    auto& range() {
        return m_range;
    }

protected:
    Vector2f m_range{ 0.0f, 1.0f };
};

class ModInt32 : public ModValue<int32_t> {
public:
    using Ptr = std::unique_ptr<ModInt32>;

    static auto create(std::string_view config_name, uint32_t default_value = 0) {
        return std::make_unique<ModInt32>(config_name, default_value);
    }

    ModInt32(std::string_view config_name, uint32_t default_value = 0)
        : ModValue{ config_name, static_cast<int>(default_value) }
    {
    }

    bool draw(std::string_view name) override {
        ImGui::PushID(this);
        auto ret = ImGui::InputInt(name.data(), &m_value);
        ImGui::PopID();

        return ret;
    }

    void draw_value(std::string_view name) override {
        ImGui::Text("%s: %i", name.data(), m_value);
    }
};

class ModKey: public ModInt32 {
public:
    using Ptr = std::unique_ptr<ModKey>;

    static auto create(std::string_view config_name, int32_t default_value = UNBOUND_KEY) {
        return std::make_unique<ModKey>(config_name, default_value);
    }

    ModKey(std::string_view config_name, int32_t default_value = UNBOUND_KEY)
        : ModInt32{ config_name, static_cast<uint32_t>(default_value) }
    {
    }

    bool draw(std::string_view name) override {
        if (name.empty()) {
            return false;
        }

        ImGui::PushID(this);
        ImGui::Button(name.data());

        if (ImGui::IsItemHovered()) {
            auto& keys = g_framework->get_keyboard_state();

            for (auto k = 0; k < keys.size(); ++k) {
                if (keys[k]) {
                    m_value = is_erase_key(k) ? UNBOUND_KEY : k;
                    break;
                }
            }

            ImGui::SameLine();
            ImGui::Text("Press any key");
        }
        else {
            ImGui::SameLine();

            if (m_value >= 0 && m_value <= 255) {
                ImGui::Text("%i", m_value);
            }
            else {
                ImGui::Text("Not bound");
            }
        }

        ImGui::PopID();

        return true;
    }

    bool is_key_down() const {
        if (m_value < 0 || m_value > 255) {
            return false;
        }

        return g_framework->get_keyboard_state()[(uint8_t)m_value] != 0;
    }

    bool is_key_down_once() {
        auto down = is_key_down();

        if (!m_was_key_down && down) {
            m_was_key_down = true;
            return true;
        }

        if (!down) {
            m_was_key_down = false;
        }

        return false;
    }

    bool is_erase_key(int k) const {
        switch (k) {
        case DIK_ESCAPE:
        case DIK_BACKSPACE:
            return true;

        default:
            return false;
        }
    }

    static constexpr int32_t UNBOUND_KEY = -1;

protected:
    bool m_was_key_down{ false };
};

class Mod {
protected:
    using ValueList = std::vector<std::reference_wrapper<IModValue>>;

public:
    enum Page : uint32_t {
        Page_None,
        Page_Breaker,
        Page_Wiresnatch,
        Page_Nero,
        Page_DanteSDT,
        Page_DanteCheat,
        Page_DanteStrive,
        Page_CommonCheat,
        Page_VergilSDT,
        Page_VergilTrick,
        Page_VergilDoppel,
        Page_VergilCheat,
        Page_VergilVFXSettings,
        Page_GameMode,
        Page_BloodyPalace,
        Page_Balance,
        Page_Enemies,
        Page_Encounters,
        Page_Camera,
        Page_QOL,
        Page_Mechanics,
        Page_EnemyStep,
        Page_Animation,
        Page_Taunt, 
        Page_Gilver,
		Page_SecretMission,
        Page_Count
        
    };

    enum Input {
		Input_Sword         = 0x1,
		Input_Gun           = 0x2,
		Input_Jump          = 0x4,
		Input_Taunt         = 0x8,
		Input_LockOn        = 0x10,
        Input_ChangeTarget  = 0x20,
        Input_DPad          = 0x40,
        Input_DevilTrigger  = 0x80,
        Input_DPadUp        = 0x100,
        Input_DPadDown      = 0x200,
        Input_DPadLeft      = 0x400,
        Input_DPadRight     = 0x800, 
        Input_Style         = 0x1000,
        Input_RightTrigger  = 0x4000,
        Input_LeftTrigger   = 0x2000,
        Input_ResetCamera   = 0x8000,
        Input_SDT           = 0x10000
    };

	static inline std::unique_ptr<InitPatternsManager> m_patterns_cache = nullptr;

    virtual ~Mod() {};
    virtual std::string_view get_name() const { return "UnknownMod"; };
    // can be used for ModValues, like Mod_ValueName
    virtual std::string generate_name(std::string_view name) { return std::string{ get_name() } + "_" + name.data(); }

	// Wrapper for easy install of hooks.
	// \param offset : offset from game exe base where hook will be installed 
	// \param hook : FunctionHook object instance
	// \param detour : Function pointer to your naked detour function.
	// if your detour function is called my_detour then just pass &my_detour
	// \param ret : Pointer to a variable that will get return address
	// \param next_instruction_offset : Optional offset to the next instruction to calculate return address automatically
	// leave this blank to get return address from minhook, note that minhook copies overwritten bytes automatically.
	inline bool install_hook_offset(ptrdiff_t offset, std::unique_ptr<FunctionHook>& hook, void* detour, uintptr_t* ret, ptrdiff_t next_instruction_offset = 0) {
		uintptr_t base = g_framework->get_module().as<uintptr_t>();
		uintptr_t location = base + offset;
#ifdef _DEBUG
		if (hook) {
			throw std::runtime_error("Install hook called multiple times with same instance\n");
		}
		printf("Installing offset hook at location: %p\n", location);
#endif
		hook = std::make_unique<FunctionHook>(location, detour);
		if (!hook->create()) {
			spdlog::error("Failed to create hook!");
			return false;
		}

		if (next_instruction_offset) {
			*ret = location + next_instruction_offset;
		}
		else {
			*ret = hook->get_original();
		}
		return true;
	}

	// same deal but using absolute address
	inline bool install_hook_absolute(uintptr_t location, std::unique_ptr<FunctionHook>& hook, void* detour, uintptr_t* ret, ptrdiff_t next_instruction_offset = 0) {
#ifdef _DEBUG
		if (hook) {
			throw std::runtime_error("Install hook called multiple times with same instance\n");
		}
		printf("Installing absolute hook at location: %p\n", location);
#endif
		hook = std::make_unique<FunctionHook>(location, detour);
		if (!hook->create()) {
			spdlog::error("Failed to create hook!");
			return false;
		}

		if (next_instruction_offset) {
			*ret = location + next_instruction_offset;
		}
		else {
			*ret = hook->get_original();
		}
		return true;
	}


    inline bool install_new_detour_offset(ptrdiff_t offset, std::shared_ptr<Detour_t>& detour_type, void* detour, uintptr_t* ret, ptrdiff_t next_instruction_offset = 0) {
        uintptr_t base = g_framework->get_module().as<uintptr_t>();
        uintptr_t location = base + offset;
#ifdef _DEBUG
        if (detour_type) {
            throw std::runtime_error("Install new detour type called multiple times with same instance\n");
        }
        printf("Installing absolute hook at location: %p\n", location.value());
#endif
        detour_type = std::make_shared<Detour_t>(location, detour, next_instruction_offset);
        m_detours.push_back(detour_type);

        if (!detour_type->is_valid()) {
            spdlog::error("Failed to create New Detour Type!");
            return false;
        }
        //prety sure this is handled and set up by the detour class, let's see how it goes.
        *ret = detour_type->get_return_address();
        return true;
    }

    inline bool install_new_detour(uintptr_t location, std::shared_ptr<Detour_t>& detour_type, void* detour, uintptr_t* ret, ptrdiff_t next_instruction_offset = 0) {
#ifdef _DEBUG
        if (detour_type) {
            throw std::runtime_error("Install new detour type called multiple times with same instance\n");
        }
        printf("Installing absolute hook at location: %p\n", location.value());
#endif
        detour_type = std::make_shared<Detour_t>(location, detour, next_instruction_offset);
        m_detours.push_back(detour_type);

        if (!detour_type->is_valid()) {
            spdlog::error("Failed to create New Detour Type!");
            return false;
        }
        //prety sure this is handled and set up by the detour class, let's see how it goes.
        *ret = detour_type->get_return_address();
        return true;
    }
    //Calls after all mods has been initialized; Should be pure virtual but then i need to override this in all mods.............
    virtual void after_all_inits() {};

    //Do all sdk::get_smth here
    virtual void on_sdk_init() {};

    // Called when ModFramework::initialize finishes in the first render frame
    // Returns an error string if it fails
    virtual std::optional<std::string> on_initialize() { return std::nullopt; }

    // Functionally equivalent, but on_frame will always get called, on_draw_ui can be disabled by ModFramework
    virtual void on_frame() {}
    virtual void on_draw_ui() {}
	virtual void on_draw_debug_ui() {}
    virtual void on_lua_state_created(lua_State*l) {}
    virtual void on_lua_state_destroyed(lua_State* l) {}
    bool user_enabled{ false };
    bool* m_is_enabled{ nullptr };
    std::vector<std::string> m_in_use_by{}; // Name of mods depending on this mod that are currently enabled
    std::vector<std::string> m_depends_on{}; // Name of mods this mod depends on
    bool m_last_state = false;
    std::vector<std::shared_ptr<Detour_t>> m_detours{};
    //bool& m_is_enabled = *ischecked;
    int m_on_page = Page_None;
    //ModKey modkeytoggle = ModKey("hotkey_on");
    std::string m_full_name_string = "Mod Name";
    std::string m_raw_full_name = "Mod Name";
    std::string m_author_string = "Author Name";
    std::string m_description_string = "Description of the mod.\nNew line commands should work";
    virtual void on_config_load(const utility::Config& cfg) {}
    virtual void on_config_save(utility::Config& cfg) {}
    // Game-specific callbacks
    /*
    virtual void on_pre_update_transform(RETransform* transform) {};
    virtual void on_update_transform(RETransform* transform) {};
    virtual void on_pre_update_camera_controller(RopewayPlayerCameraController* controller) {};
    virtual void on_update_camera_controller(RopewayPlayerCameraController* controller) {};
    virtual void on_pre_update_camera_controller2(RopewayPlayerCameraController* controller) {};
    virtual void on_update_camera_controller2(RopewayPlayerCameraController* controller) {};
    */

  protected:
    const std::string m_prefix_check_box_name = "##";
    const std::string m_prefix_hot_key_name   = "key ";

    std::string m_check_box_name{};
    std::string m_hot_key_name{};

public:
    virtual void init_check_box_info() { return; };
    virtual std::string get_checkbox_name() { return "UnknownMod"; };
	virtual std::string get_hotkey_name() { return "UnknownMod"; };


	virtual void set_up_hotkey() {
        m_raw_full_name = m_full_name_string;

        // Not ideal
        if (m_raw_full_name.size() > 4) {
            if (m_raw_full_name.substr(m_raw_full_name.size() - 4, 4) == " (+)") {
                m_raw_full_name = m_raw_full_name.substr(0, m_raw_full_name.size() - 4);
            }
        }

		g_keyBinds.Get()->AddBind(std::string(get_name()),
			[this]() {

				if(m_is_enabled != nullptr) *m_is_enabled = !*m_is_enabled;

                {
                    ImGuiToast toast(*m_is_enabled ? ImGuiToastType_Success : ImGuiToastType_Error, 500);

                    toast.set_title("%s: %s", m_raw_full_name.c_str(), *m_is_enabled ? "Activated!" : "Deactivated!");

                    g_framework->queue_notification(toast);
                }
			}, OnState_Press);
	}
};