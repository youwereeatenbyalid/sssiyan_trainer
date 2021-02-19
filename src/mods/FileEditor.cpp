#include "FileEditor.hpp"

std::optional<std::vector<fs::path>>
list_files(const fs::path& root, const std::optional<std::string>& ext = {},
           const std::optional<std::string>& ex_ext = {}) {
  if (!fs::exists(root) || !fs::is_directory(root)) {
    return {};
  }

  std::vector<fs::path> ret;

  fs::recursive_directory_iterator it(root);

  for (auto& i : it) {
    if (fs::is_regular_file(i)) {
      if (ext) {
        if (ex_ext) {
          if (it->path().extension() == ext &&
              it->path().extension() != ex_ext) {
            ret.push_back(i);
          }
        } else {
          if (it->path().extension() == ext) {
            ret.push_back(i);
          }
        }
      } else {
        if (ex_ext) {
          if (it->path().extension() != ex_ext) {
            ret.push_back(i);
          }
        } else {
          ret.push_back(i);
        }
      }
    }
  }

  if (ret.size()) {
    return ret;
  }

  return {};
}

namespace fs = std::filesystem;

FileEditor* g_fileEditor = nullptr;

// clang-format off
// only in clang/icl mode on x64, sorry

FileEditor::FileEditor()
    :m_is_active{ false }, m_file_config_paths{{}}, m_hot_swaps{{}}
{
	g_fileEditor = this;
}

void FileEditor::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> FileEditor::on_initialize() {
  init_check_box_info();
  
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  ischecked = &(FileEditor::m_is_active);
  onpage    = gamemode;

  full_name_string     = "Asset Swapper";
  author_string        = "Darkness (TheDarkness704)";
  description_string   = "In the dark times of need a hero of Darkness rose and made this shit.";

  std::string cfg_ext = ".ini";

  m_file_config_paths = list_files("natives/x64/Mods", cfg_ext);
  
  if(m_file_config_paths){
    for(auto config : *m_file_config_paths){
      // Read Mod Info
      auto mod_cfg = HotSwapCFG(config);
      auto mod_name_length = config.stem().string().size();
      auto mod_files = list_files(fs::path("natives/x64/Mods") / config.stem(), {}, cfg_ext);
      if (mod_files){
        for (auto& file : *mod_files) {
          file = file.parent_path() / file.stem();
        }

        auto original_mod_file_paths = *mod_files;
        for (auto& file : original_mod_file_paths) {
          auto file_path = file.string();
          file = file_path.substr(file_path.find("natives/x64/Mods/") + (sizeof("natives/x64/Mods/") - 1 + mod_name_length + 1) + 1);
        }

        auto replace_mod_file_path = *mod_files;
        for (auto& file : replace_mod_file_path) {
          auto file_path = file.string();
          file = file_path.substr(file_path.find("natives/x64/") + (sizeof("natives/x64/") - 1));
        }

        std::vector<Asset_Path> path_replacement;

        for (UINT i = 0; i < (*mod_files).size(); i++) {
            auto org_path = original_mod_file_paths[i].wstring();
            std::replace(org_path.begin(), org_path.end(), '\\', '/');
            
            auto new_path = replace_mod_file_path[i].wstring();
            std::replace(new_path.begin(), new_path.end(), '\\', '/');

            path_replacement.push_back({org_path, new_path});
        }

        m_hot_swaps.value().push_back(std::make_shared<Asset_Hotswap>(Asset_Hotswap{false, m_hot_swaps.value().size(), mod_cfg.get_name(), mod_cfg.get_main_name(), "##" + mod_cfg.get_main_name(), mod_cfg.get_description(), mod_cfg.get_version(), mod_cfg.get_author(), path_replacement}));
      }
    }
  }

  auto file_loader_fn = utility::scan(base, "40 53 57 41 55 41 57 48 83 EC 48 49");

  if (!file_loader_fn) {
    return "Unable to find FileEditor pattern.";
  }

  m_file_loader_hook = std::make_unique<FunctionHook>(*file_loader_fn, (uintptr_t)&FileEditor::internal_file_loader);

  if (!m_file_loader_hook->create()) {
    return "Failed to hook File Loader.";
  }

  return Mod::on_initialize();
}

// during load
void FileEditor::on_config_load(const utility::Config &cfg) {
    for (auto& asset_mod : *m_hot_swaps) {
        asset_mod->is_on = cfg.get<bool>("AssetMod(\"" + asset_mod->main_name + "\")Enable").value_or(false);
        asset_mod->priority = cfg.get<unsigned int>("AssetMod(\"" + asset_mod->main_name + "\")Priority").value_or(asset_mod->priority);
    }

    std::sort(m_hot_swaps.value().begin(), m_hot_swaps.value().end(),[](std::shared_ptr<Asset_Hotswap> a, std::shared_ptr<Asset_Hotswap> b){return a->priority > b->priority;});
}
// during save
void FileEditor::on_config_save(utility::Config &cfg) {
    for (auto& asset_mod : *m_hot_swaps) {
        cfg.set<bool>("AssetMod(\"" + asset_mod->main_name + "\")Enable", asset_mod->is_on);
        cfg.set<unsigned int>("AssetMod(\"" + asset_mod->main_name + "\")Priority", asset_mod->priority);
    }
}
// do something every frame
void FileEditor::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void FileEditor::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void FileEditor::on_draw_ui() {
    for (UINT n = 0; n < m_hot_swaps.value().size(); n++) {
        auto& asset_mod = m_hot_swaps.value()[n];

        ImGui::Checkbox(asset_mod->label.c_str(), &asset_mod->is_on);
        ImGui::SameLine(); bool node_open = ImGui::TreeNodeEx(asset_mod->name.c_str());
        
        if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
        {
            auto mouse_delta_y = ImGui::GetMouseDragDelta(0).y;
            int delta = 0;

            if (mouse_delta_y < -15.0f) {
                delta = -1;
            }
            else if (mouse_delta_y > 15.0f) {
                delta = 1;
            }

            if(delta != 0){
                int n_next = n + delta;
                if (n_next >= 0 && n_next < m_hot_swaps.value().size())
                {
                    std::swap(m_hot_swaps.value()[n]->priority, m_hot_swaps.value()[n_next]->priority);
                    std::swap(m_hot_swaps.value()[n], m_hot_swaps.value()[n_next]);
                    ImGui::ResetMouseDragDelta();
                }
            }
        }

        if (node_open){
            if(asset_mod->author) {
                ImGui::SetCursorPosX(65.f);
                ImGui::TextWrapped("Author: %s", asset_mod->author.value().c_str());
            }
            if(asset_mod->version) {
                ImGui::SetCursorPosX(65.f);
                ImGui::TextWrapped("Version: %s", asset_mod->version.value().c_str());
            }
            if(asset_mod->description){
                ImGui::SetCursorPosX(65.f);
                ImGui::TextWrapped("Description: %s", asset_mod->description.value().c_str());
            }
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::TreePop();
        }
    }
}

bool FileEditor::asset_check(const wchar_t* game_path, const wchar_t* mod_path) const {
	while (*game_path) if (towlower(*game_path++) != towlower(*mod_path++)) return false;
	return true;
}

void* __fastcall FileEditor::file_editor(uintptr_t this_p, uintptr_t RDX, const wchar_t* file_path)
{
    auto _ = [&](){
        if(m_is_active && m_hot_swaps){
            for (auto& asset_mod : *m_hot_swaps) {
                if(asset_mod->is_on){
                    for(auto& mod_replace_paths : asset_mod->redirection_list){
                        if (asset_check(mod_replace_paths.org_path.c_str(), file_path)) {
                            file_path = mod_replace_paths.new_path.c_str();
                            return;
                        }
                    }
                }
            }
        }
    };_();

    auto file_loader_fn = m_file_loader_hook->get_original<decltype(FileEditor::internal_file_loader)>();
	return file_loader_fn(this_p, RDX, file_path);
}

void* __fastcall FileEditor::internal_file_loader(uintptr_t this_p, uintptr_t RDX, const wchar_t* file_path)
{
    return g_fileEditor->file_editor(this_p, RDX, file_path);
}

HotSwapCFG::HotSwapCFG(fs::path cfg_path)
     :m_cfg_path{ cfg_path }, m_mod_name { cfg_path.stem().string() }, m_main_name{ cfg_path.stem().string() },
      m_description {}, m_version {}, m_author {}
{
	m_variable_map["name"] = ev_mod_name;
	m_variable_map["description"] = ev_description;
	m_variable_map["version"] = ev_version;
	m_variable_map["author"] = ev_author;

    std::ifstream cfg(cfg_path.string());

    if (cfg) {
        for (std::string line{}; getline(cfg, line); ) {
            std::istringstream ss{ line };
            std::string variable{};
            std::string value{};

            if(line.find('=') != std::string::npos){
                getline(ss, variable, '=');
                getline(ss, value);
                process_line(variable, value);
            }
        }
    }
}

void HotSwapCFG::process_line(std::string variable, std::string value)
{
    if (!variable.empty() && !value.empty()) {
        switch (m_variable_map[str_to_lower(variable)]) {
        case ev_mod_name:
            m_mod_name = value;
            break;

        case ev_description:
            m_description = value;
            break;

        case ev_version:
            m_version = value;
            break;

        case ev_author:
            m_author = value;
            break;

        default:
            break;
        }
    }
}
