#pragma once
#include <map>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "Mod.hpp"
#include "sdk/ReClass.hpp"

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

private:
  enum StringValue {
    ev_null,
    ev_mod_name,
    ev_description,
    ev_version,
    ev_author
  };

  std::map<std::string, StringValue> m_variable_map;

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
};

class FileEditor : public Mod {
public:
    FileEditor();
    ~FileEditor() = default;
    // mod name string for config
    std::string_view get_name() const override { return "AssetSwapper"; }
    std::string get_checkbox_name() override { return m_check_box_name; };
    std::string get_hotkey_name() override { return m_hot_key_name; };
    
    bool m_is_active;

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
    // function hook instance for our detour, convinient wrapper 
    // around minhook
    void init_check_box_info() override;
    
    std::unique_ptr<FunctionHook> m_file_loader_hook{};
    static void* __fastcall internal_file_loader(uintptr_t this_p, uintptr_t RDX, const wchar_t* file_path);
    void* __fastcall file_editor(uintptr_t this_p, uintptr_t RDX, const wchar_t* file_path);

    inline bool asset_check(const wchar_t* game_path, const wchar_t* mod_path) const;

private: // structs
    typedef struct {
        /*const wchar_t* org_path;
        const wchar_t* new_path;*/
        std::wstring org_path;
        std::wstring new_path;
    } Asset_Path;

    typedef struct {
        bool is_on;
        unsigned int priority;
        std::string name;
        std::string main_name;
        std::string label;
        std::optional<std::string> description;
        std::optional<std::string> version;
        std::optional<std::string> author;
        std::vector<Asset_Path> redirection_list;
    } Asset_Hotswap;

private:
    std::optional<std::vector<fs::path>> m_file_config_paths{};
    std::optional<std::vector<std::shared_ptr<Asset_Hotswap>>> m_hot_swaps{};
};
