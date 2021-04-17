#pragma once
#include <map>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include "mods/LDK.hpp"
#include "mods/SpardaWorkshop.hpp"
// clang-format off
namespace fs = std::filesystem;

class HotSwapCFG {
public:
    HotSwapCFG(fs::path cfg_path);
    HotSwapCFG(HotSwapCFG& other) = delete;
    HotSwapCFG(HotSwapCFG&& other) = delete;
    ~HotSwapCFG() = default;
    std::string get_name() const { return m_mod_name; }
    std::string get_main_name() const { return m_main_name; }
    std::optional<std::string> get_description() const { return m_description; }
    std::optional<std::string> get_version() const { return m_version; }
    std::optional<std::string> get_author() const { return m_author; }
    std::optional<int> get_character() const {return m_character;}
private:
    static enum class StringValue : const uint8_t {
        ev_null,
        ev_mod_name,
        ev_description,
        ev_version,
        ev_author,
        ev_character
    };

    static enum class CharacterValue : const uint8_t {
        ev_null,
        ev_nero,
        ev_dante,
        ev_v,
        ev_vergil
    };

    std::map<std::string, StringValue> m_variable_map;
    std::map<std::string, CharacterValue> m_char_name_map;

private:
    void process_line(std::string variable, std::string value);
    std::string str_to_lower(std::string s) { 
        ::std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

private:
    fs::path m_cfg_path;
    std::string m_mod_name;
    std::string m_main_name;
    std::optional<std::string> m_description;
    std::optional<std::string> m_version;
    std::optional<std::string> m_author;
    std::optional<uint8_t> m_character;

    friend class FileEditor;
};

class FileEditor : public Mod {
public:
    FileEditor();
    ~FileEditor() = default;
    // mod name string for config
    std::string_view get_name() const override { return "AssetSwapper"; }
    std::string get_checkbox_name() override { return m_check_box_name; };
    std::string get_hotkey_name() override { return m_hot_key_name; };

    static bool m_is_active;

    // called by m_mods->init() you'd want to override this
    std::optional<std::string> on_initialize() override;

    // Override this things if you want to store values in the config file
    void on_config_load(const utility::Config& cfg) override;
    void on_config_save(utility::Config& cfg) override;
    
    // on_frame() is called every frame regardless whether the gui shows up.
    void on_frame() override;
    // on_draw_ui() is called only when the gui shows up
    // you are in the imgui window here.
    void on_draw_ui() override;
    // on_draw_debug_ui() is called when debug window shows up
    void on_draw_debug_ui() override;
    
private:
    // Forward declration because I like to keep my structs at the end of the class
    struct Costume_List_t;
    struct Char_Default_Costume_Info;

    // function hook instance for our detour, convinient wrapper 
    // around minhook
    void init_check_box_info() override;

    std::vector<char> m_costume_list_container;

    static uint64_t m_original_costume_count;

    static uint32_t get_costume_list_size(uint32_t character, uint64_t original_size);
    static uintptr_t m_costume_list_size_addr;

    static uintptr_t m_scroll_list_jmp_ret;
    static uintptr_t m_costume_list_jmp_ret;
    static uintptr_t m_costume_list_jnl_ret;
    //static uintptr_t costume_select_jmp_ret;

    static Costume_List_t* m_new_costume_list_p;

    static std::optional<Char_Default_Costume_Info> m_selected_char_costume_info;

    static uint32_t m_nero_costume_count;
    static uint32_t m_nero_last_og_costume_count;
    static bool m_nero_csize;

    static uint32_t m_dante_costume_count;
    static uint32_t m_dante_last_og_costume_count;
    static bool m_dante_csize;

    static uint32_t m_gilver_costume_count;
    static uint32_t m_gilver_last_og_costume_count;
    static bool m_gilver_csize;

    static uint32_t m_vergil_costume_count;
    static uint32_t m_vergil_last_og_costume_count;
    static bool m_vergil_csize;

    static uint32_t m_selected_character;

    static bool m_is_in_select_menu;

    // Showing the checkbox for the Super/EX costumes
    bool m_show_costume_options;

    static bool m_load_super;
    static bool m_load_ex;

    static naked void scroll_list_detour();
    static naked void costume_list_detour();
    
    // Saving a copy of the config data so we can use it for our own stuff after the first time the load function was called
    std::optional<utility::Config> m_config;

    // Actual hooks
    std::unique_ptr<FunctionHook> m_file_loader_hook{};
    std::unique_ptr<FunctionHook> m_costume_list_maker_hook{};
    std::unique_ptr<FunctionHook> m_selected_costume_processor_hook{};
    std::unique_ptr<FunctionHook> m_ui_costume_name_hook{};
    
    // asm detours
    std::unique_ptr<FunctionHook> m_scroll_list_hook{};
    std::unique_ptr<FunctionHook> m_costume_list_hook{};

    //Real men's hooks
    static void* __fastcall file_loader_internal(uintptr_t this_p, uintptr_t RDX, const wchar_t* file_path);
    void* __fastcall file_loader(uintptr_t this_p, uintptr_t RDX, const wchar_t* file_path);

    static void __fastcall costume_list_maker_internal(uintptr_t RCX, uintptr_t ui2120GUI);
    void __fastcall costume_list_maker(uintptr_t RCX, uintptr_t ui2120GUI);

    static void __fastcall selected_costume_processor_internal(uintptr_t RCX, uintptr_t ui2120GUI);
    void __fastcall selected_costume_processor(uintptr_t RCX, uintptr_t ui2120GUI);

    static UI_String_t* __fastcall ui_costume_name_internal(uintptr_t RCX, uintptr_t RDX, uint32_t costume_id);
    UI_String_t* __fastcall ui_costume_name(uintptr_t RCX, uintptr_t RDX, uint32_t costume_id);

    void load_mods();
    void load_sys_mods();
    void bind_sys_mod(std::string modname, bool* on_value);
    inline bool asset_check(const wchar_t* game_path, const wchar_t* mod_path) const;

private: // structs
    struct Asset_Path{
        /*const wchar_t* org_path;
        const wchar_t* new_path;*/
        std::wstring org_path;
        std::wstring new_path;
    };

    struct Asset_Hotswap{
        bool is_on;
        uint64_t priority;
        std::optional<uint32_t> character;
        std::optional<uint32_t> slot_in_select_menu;
        std::string name;
        std::wstring w_name;
        std::string main_name;
        std::string label;
        std::optional<std::string> description;
        std::optional<std::string> version;
        std::optional<std::string> author;
        std::vector<Asset_Path> redirection_list;
        std::optional<UI_String_t> costume_name;
        uint32_t costume_id;
        bool* on_ptr = nullptr;
    };

    struct Info_Back{
        bool is_on;
        unsigned int priority;
    };
    
    struct Costume_List_t{
        // Only meant to be called when allocating memory properly
        Costume_List_t(uint32_t init_size) 
            :ukn1{ nullptr }, ukn2{ NULL }, ukn3{ NULL },
            ukn4{ nullptr }, ukn5{ NULL }, size{ init_size }
        {
        }

        // Needs proper memory allocation
        Costume_List_t(const Costume_List_t& other) 
            :ukn1{ other.ukn1 }, ukn2{ other.ukn2 }, ukn3{ other.ukn3 },
            ukn4{ other.ukn4 }, ukn5{ other.ukn5 }, size{ other.size }
        {
            memcpy_s(&costumes, size*sizeof(uint32_t), &other.costumes, other.size*sizeof(uint32_t));
        }

        // Needs proper memory allocation
        Costume_List_t(const Costume_List_t& other, uint32_t init_size) 
            :ukn1{ other.ukn1 }, ukn2{ other.ukn2 }, ukn3{ other.ukn3 },
            ukn4{ other.ukn4 }, ukn5{ other.ukn5 }, size{ init_size }
        {
            memcpy_s(&costumes, size*sizeof(uint32_t), &other.costumes, other.size*sizeof(uint32_t));
        }

        // Needs proper memory allocation
        Costume_List_t(const Costume_List_t& other, uint32_t org_size, std::vector<uint32_t> extra) 
            :ukn1{ other.ukn1 }, ukn2{ other.ukn2 }, ukn3{ other.ukn3 },
            ukn4{ other.ukn4 }, ukn5{ other.ukn5 }, size{ org_size + extra.size() }
        {
            // Transfering original costume list to the new list
            for (UINT i = 0; i < org_size; i++) {
                costumes[i] = other.costumes[i];
            }

            // Putting the extra costumes into slots after the original costumes
            for (UINT i = 0; i < extra.size(); i++) {
                costumes[i + org_size] = extra[i];
            }
        }

        void* ukn1;
        uint32_t ukn2;
        uint32_t ukn3;
        void* ukn4;
        uint32_t ukn5;

        uint32_t size;
        uint32_t costumes[];

        uint32_t& operator[](uint64_t index) {
            return this->costumes[index];
        }

        const uint32_t& operator[](uint64_t index) const {
            return this->costumes[index];
        }

        void operator=(const Costume_List_t& other) {
            ukn1 = other.ukn1;
            ukn2 = other.ukn2;
            ukn3 = other.ukn3;
            ukn4 = other.ukn4;
            ukn5 = other.ukn5;
            size = other.size;
            memcpy_s(costumes, size*sizeof(uint32_t), other.costumes, other.size*sizeof(uint32_t));
        }
    };

    struct Char_Default_Costume_Info {
        uint8_t org_super;
        uint8_t org;
        uint8_t ex;
        uint8_t ex_super;
    };

private:
    std::optional<std::vector<fs::path>> m_mod_roots{};
    std::optional<std::vector<fs::path>> m_sys_mod_roots{};
    std::optional<std::vector<std::shared_ptr<Asset_Hotswap>>> m_hot_swaps{};
    std::optional<std::vector<std::shared_ptr<Asset_Hotswap>>> m_sys_hot_swaps{};
    std::optional<std::vector<std::shared_ptr<Asset_Hotswap>>> m_nero_swaps{};
    std::optional<std::vector<std::shared_ptr<Asset_Hotswap>>> m_dante_swaps{};
    std::optional<std::vector<std::shared_ptr<Asset_Hotswap>>> m_gilver_swaps{};
    std::optional<std::vector<std::shared_ptr<Asset_Hotswap>>> m_vergil_swaps{};

    std::vector<uint32_t> m_nero_extra_costumes;
    std::vector<uint32_t> m_dante_extra_costumes;
    std::vector<uint32_t> m_gilver_extra_costumes;
    std::vector<uint32_t> m_vergil_extra_costumes;

    void asset_swap_ui(std::optional<std::vector<std::shared_ptr<Asset_Hotswap>>>& hot_swaps);
    void costume_swap_ui(std::optional<std::vector<std::shared_ptr<Asset_Hotswap>>>& costume_swaps);
};
