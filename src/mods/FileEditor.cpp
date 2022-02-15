#include "FileEditor.hpp"

// clang-format off
// only in clang/icl mode on x64, sorry

std::optional<std::vector<fs::path>> list_files(const fs::path& root, const std::optional<std::string>& ext = {}, const std::optional<std::string>& ex_ext = {}) {
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

std::optional<std::vector<fs::path>> list_dirs(const fs::path& root) {
    if (!fs::exists(root) || !fs::is_directory(root)) {
        return {};
    }

    std::vector<fs::path> ret;
    fs::directory_iterator it(root);

    for (auto& i : it) {
        if (is_directory(i)) {
            ret.push_back(i);
        }
    }

    return ret;
}

namespace fs = std::filesystem;

FileEditor* g_FileEditor = nullptr;

bool FileEditor::m_is_active{ false };

// Positions to return to after asm detours
uintptr_t FileEditor::m_scroll_list_jmp_ret{ NULL };
uintptr_t FileEditor::m_costume_list_jmp_ret{ NULL };
uintptr_t FileEditor::m_costume_list_jnl_ret{ NULL };

uint64_t FileEditor::m_original_costume_count{ NULL };

uint32_t FileEditor::m_nero_costume_count{ NULL };
uint32_t FileEditor::m_nero_last_og_costume_count{ NULL };
bool FileEditor::m_nero_csize{ false };

uint32_t FileEditor::m_dante_costume_count{ NULL };
uint32_t FileEditor::m_dante_last_og_costume_count{ NULL };
bool FileEditor::m_dante_csize{ false };

uint32_t FileEditor::m_gilver_costume_count{ NULL };
uint32_t FileEditor::m_gilver_last_og_costume_count{ NULL };
bool FileEditor::m_gilver_csize{ false };

uint32_t FileEditor::m_vergil_costume_count{ NULL };
uint32_t FileEditor::m_vergil_last_og_costume_count{ NULL };
bool FileEditor::m_vergil_csize{ false };

//uintptr_t FileEditor::costume_select_jmp_ret{NULL};

// Function pointers to call in the asm blocks cuz of... clang 9 :facepalm:
uintptr_t FileEditor::m_costume_list_size_addr{ NULL };

bool FileEditor::m_is_in_select_menu;

// Super and ex costume toggles
bool FileEditor::m_load_super{ false };
bool FileEditor::m_load_ex{ false };

uint32_t FileEditor::m_selected_character{ NULL };

std::optional<FileEditor::Char_Default_Costume_Info> FileEditor::m_selected_char_costume_info{{}};

// Costume list
/*
std::vector<uint32_t> FileEditor::m_nero_extra_costumes{};
std::vector<uint32_t> FileEditor::m_dante_extra_costumes{};
std::vector<uint32_t> FileEditor::m_gilver_extra_costumes{};
std::vector<uint32_t> FileEditor::m_vergil_extra_costumes{};*/

/*
uint32_t FileEditor::m_nero_costumes[]{ 0,0,0,0,0,0,0,8,2,0,1,3,0x19,0x1A,0x1B,0x1C,0x1D,0x1E };
uint32_t FileEditor::m_dante_costumes[]{ 0,0,0,0,0,0,0,8,2,0,1,3,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F };
uint32_t FileEditor::m_gilver_costumes[]{ 0,0,0,0,0,0,0,8,2,0,1,3,0x1B,0x1C,0x1D,0x1E,0x1F,0x20 };
uint32_t FileEditor::m_vergil_costumes[]{ 0,0,0,0,0,0,0,8,2,0,1,3,0x16,0x17,0x18,0x19,0x1A,0x1B };*/

FileEditor::Costume_List_t* FileEditor::m_new_costume_list_p{ NULL };

// Hitch's shit that I modified it now get's the selected character as a parameter \
to manage the portrait count shown in the costume select menu and also constructs  \
new portrait lis because for some reason it crashes when I tried doing it in the manage costume list function :/
uint32_t FileEditor::get_costume_list_size(uint32_t character, uint64_t original_size) {
    m_selected_character = character;

    // Some weird calculation to decide the length of the scroll list
    switch (character) {
    case (uint8_t)HotSwapCFG::CharacterValue::ev_nero:
    {
        m_nero_costume_count -= m_nero_last_og_costume_count;
        m_nero_costume_count += original_size;
        m_nero_last_og_costume_count = original_size;
        return m_nero_csize ? m_nero_costume_count : original_size;
    }
        break;

    case (uint8_t)HotSwapCFG::CharacterValue::ev_dante:
    {
        m_dante_costume_count -= m_dante_last_og_costume_count;
        m_dante_costume_count += original_size;
        m_dante_last_og_costume_count = original_size;
        return m_dante_csize ? m_dante_costume_count : original_size;
    }
        break;

    case (uint8_t)HotSwapCFG::CharacterValue::ev_v:
    {
        m_gilver_costume_count -= m_gilver_last_og_costume_count;
        m_gilver_costume_count += original_size;
        m_gilver_last_og_costume_count = original_size;
        return m_gilver_csize ? m_gilver_costume_count : original_size;
    }
        break;

    case (uint8_t)HotSwapCFG::CharacterValue::ev_vergil:
    {
        m_vergil_costume_count -= m_vergil_last_og_costume_count;
        m_vergil_costume_count += original_size;
        m_vergil_last_og_costume_count = original_size;
        return m_vergil_csize ? m_vergil_costume_count : original_size;
    }
        break;

    default:
        break;
    }

    return original_size;
}

naked void FileEditor::scroll_list_detour(){
    __asm {
        cmp  m_is_active, 1
        jne  original_code

    custom_list_size:
        push rcx
        push rdx
        sub  rsp, 32
        mov  ecx, dword ptr [rdi + 0xE8]
        add  ecx, 1
        mov  rdx, rax
        call qword ptr [m_costume_list_size_addr]
        add  rsp, 32
        pop  rdx
        pop  rcx
        jmp  original_code

    original_code:
        mov  [r15], rax
        mov  eax, [r15]
        jmp  m_scroll_list_jmp_ret
        push r8
        mov ecx, dword ptr [rdi+0xE8]
        lea r8, FileEditor::get_costume_list_size
        call r8
        pop r8
        pop rcx
        mov [r15], rax
        mov eax, [r15]
        jmp qword ptr [m_scroll_list_jmp_ret]
    }
}

naked void FileEditor::costume_list_detour() {
    __asm {
        cmp  m_is_active, 1
        jne  original_code

    custom_list:
        cmp  eax, 0x8
        jnl  jmp_jnl
        cmp  byte ptr [r13 + 0xE8], 0
        je   lea_nero
        cmp  byte ptr [r13 + 0xE8], 1
        je   lea_dante
        cmp  byte ptr [r13 + 0xE8], 2
        je   lea_gilver
        cmp  byte ptr [r13 + 0xE8], 3
        je   lea_vergil

    lea_nero:
        cmp  m_nero_csize, 0
        je   load_original_array
        mov  rcx, m_new_costume_list_p
        jmp  original_result
    
    lea_dante:
        cmp  m_dante_csize, 0
        je   load_original_array
        mov  rcx, m_new_costume_list_p
        jmp  original_result
    
    lea_gilver:
        cmp  m_gilver_csize, 0
        je   load_original_array
        mov  rcx, m_new_costume_list_p
        jmp  original_result
    
    lea_vergil:
        cmp  m_vergil_csize, 0
        je   load_original_array
        mov  rcx, m_new_costume_list_p
        jmp  original_result

    original_code:
        cmp  eax, [rdx + 0x18]
        jnl  jmp_jnl

    load_original_array:
        mov  rcx, [rdx + 0x10]

    original_result:
        test rcx, rcx
    
    jmp_ret:
        jmp  qword ptr [m_costume_list_jmp_ret]

    jmp_jnl:
        jmp  qword ptr [m_costume_list_jnl_ret]

    }
}

FileEditor::FileEditor()
    :m_mod_roots{{}}, m_hot_swaps{{}}, m_sys_hot_swaps{ {} }, m_nero_swaps{ {} }, m_dante_swaps{ {} }, m_gilver_swaps{ {} }, m_vergil_swaps{ {} }, m_show_costume_options{ false }
{
	g_FileEditor = this;
    m_costume_list_size_addr = (uintptr_t)&get_costume_list_size;
}

void FileEditor::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> FileEditor::on_initialize() {
  init_check_box_info();
  
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE

  m_is_enabled = &m_is_active;
  m_on_page    = gamemode;

  m_full_name_string     = "Asset Swapper (+)";
  m_author_string        = "Darkness (TheDarkness704)";
  m_description_string   = "Lets you load custom assets into the game and manage them. Effects will take action after each loading screen.";

  set_up_hotkey();

  load_mods();
  load_sys_mods();

  auto file_loader_fn = patterns->find_addr(base, "40 53 57 41 55 41 57 48 83 EC 48 49");
  auto costume_list_maker_fn = patterns->find_addr(base, "48 8B C4 55 53 41 55 41 56 48 8D 68 A1");
  auto selected_costume_processor_fn = patterns->find_addr(base, "48 89 5C 24 18 57 48 83 EC 20 C7 82 C0 00 00 00 09 00 00 00 48 8B FA");
  auto ui_costume_name_fn = patterns->find_addr(base, "48 89 5C 24 10 57 48 83 EC 20 41 8B F8 48 8D 59 40 4C 8B C2 48 8B CB 48 8D 54 24 30 E8 DF FD FF FF 48 8B 44 24 30 48 3B 03");

  auto scroll_list_addr = patterns->find_addr(base, "41 89 07 41 8B 07");
  auto costume_list_addr = patterns->find_addr(base, "3B 42 18 7D 56");
  //auto ui_costume_name_manager_addr = patterns->find_addr(base, "49 8B 44 C1 38 48 8B 5C 24 38 48 83 C4 20 5F");

  if (!file_loader_fn) {
    return "Unable to find FileEditor pattern.";
  }

  if (!costume_list_maker_fn) {
      return "Unable to find costume_list_maker pattern.";
  }

  if (!selected_costume_processor_fn) {
      return "Unable to find the selected_costume_processor pattern.";
  }

  if (!ui_costume_name_fn) {
      return "Unable to find the ui_costume_name pattern.";
  }

  if (!scroll_list_addr) {
      return "Unable to find scroll_list pattern.";
  }

  if (!costume_list_addr) {
      return "Unable to find costume_list pattern.";
  }

  // This adds to the slots available in the costume selection menu
  if (!install_hook_absolute(scroll_list_addr.value(), m_scroll_list_hook, &scroll_list_detour, &m_scroll_list_jmp_ret, 6)) {
      //return an error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize m_scroll_list_hook";
  }

  // This constrols the information of each costume slot in the selection menu
  if (!install_hook_absolute(costume_list_addr.value(), m_costume_list_hook, &costume_list_detour, &m_costume_list_jmp_ret, 12)) {
      //return an error string in case something goes wrong
      spdlog::error("[{}] failed to initialize", get_name());
      return "Failed to initialize m_costume_list_hook";
  }

  FileEditor::m_costume_list_jnl_ret = costume_list_addr.value() + 0x5B;

  m_file_loader_hook = std::make_unique<FunctionHook>(*file_loader_fn, (uintptr_t)&FileEditor::file_loader_internal);
  m_costume_list_maker_hook = std::make_unique<FunctionHook>(*costume_list_maker_fn, (uintptr_t)&FileEditor::costume_list_maker_internal);
  m_selected_costume_processor_hook = std::make_unique<FunctionHook>(*selected_costume_processor_fn, (uintptr_t)&FileEditor::selected_costume_processor_internal);
  m_ui_costume_name_hook = std::make_unique<FunctionHook>(*ui_costume_name_fn, (uintptr_t)&FileEditor::ui_costume_name_internal);

  // This redirects what assets the game loads
  if (!m_file_loader_hook->create()) {
    return "Failed to hook File Loader.";
  }

  // This redirects the function that makes the list when selecting costumes
  if (!m_costume_list_maker_hook->create()) {
      return "Failed to hook Costume List Generator.";
  }

  if (!m_selected_costume_processor_hook->create()) {
      return "Failed to hook Selected Costume Processor.";
  }

  if (!m_ui_costume_name_hook->create()) {
      return "Failed to hook UI Costume Name.";
  }

  return Mod::on_initialize();
}
#include <iostream>
void FileEditor::load_mods()
{
    m_show_costume_options = false;

    // Reseting variables that contain old mods' info
    m_hot_swaps.value().clear();
    m_nero_swaps.value().clear();
    m_dante_swaps.value().clear();
    m_gilver_swaps.value().clear();
    m_vergil_swaps.value().clear();
    
    m_nero_extra_costumes.clear();
    m_dante_extra_costumes.clear();
    m_gilver_extra_costumes.clear();
    m_vergil_extra_costumes.clear();

    m_nero_csize = false;
    m_dante_csize = false;
    m_gilver_csize = false;
    m_vergil_csize = false;
    
    FileEditor::m_nero_costume_count = m_nero_last_og_costume_count;
    FileEditor::m_dante_costume_count = m_dante_last_og_costume_count;
    FileEditor::m_gilver_costume_count = m_gilver_last_og_costume_count;
    FileEditor::m_vergil_costume_count = m_vergil_last_og_costume_count;

    const std::string cfg_name = "modinfo";
    const std::string cfg_ext = ".ini";

    //get mod names
    m_mod_roots = list_dirs("natives/x64/Mods");

    if(m_mod_roots){
        // Using this to put the char specific costumes to the end of the priorities
        uint64_t costume_index = UINT64_MAX;
        // Using these to associate each costume with a slot in the menu according to the character it's tied to
        uint32_t nero_costume_slot = 0;
        uint32_t dante_costume_slot = 0;
        uint32_t gilver_costume_slot = 0;
        uint32_t vergil_costume_slot = 0;
        for(auto mod_root : *m_mod_roots){
            // Read Mod Info
            auto mod_cfg = HotSwapCFG(mod_root / (cfg_name + cfg_ext));
            //get mod name
            auto mod_name = mod_root.filename()/*not actually "file" name, but the folder name*/.string();
            //mod files is a list of all lifes (whats? :/) not including the .ini? yes
            auto mod_files = list_files(fs::path("natives/x64/Mods") / mod_root.filename()/*same deal as above*/ / "natives/x64", {}, cfg_ext);
            if (mod_files){
                //getting rid of the stupid extenstions
                for (auto& file : *mod_files) {
                    file = file.parent_path() / file.stem();
                }
                //converting the path to the format the game uses (starting from insidethe x64 folder as the root)
                auto original_mod_file_paths = *mod_files;
                for (auto& file : original_mod_file_paths) {
                    auto file_path = file.string();
                    file = file_path.substr(file_path.find("natives/x64/Mods\\" + mod_root.filename().string()) + (("natives/x64/Mods\\" + mod_name + "natives/x64\\").size()) + 1);
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
                
                //filling the array with our mods
                auto priority = mod_cfg.get_character() ? costume_index-- : m_hot_swaps.value().size();
                //Asset_Hotswap hotswap{ false, priority, mod_cfg.get_character(), {}, mod_cfg.get_name(), mod_cfg.get_main_name(), "##" + mod_cfg.get_main_name(), mod_cfg.get_description(), mod_cfg.get_version(), mod_cfg.get_author(), path_replacement};
                m_hot_swaps.value().push_back(std::make_shared<Asset_Hotswap>(
                    Asset_Hotswap{ 
                        false, priority, mod_cfg.get_character(), 
                        {}, mod_cfg.get_name(), L"Test String",
                        mod_cfg.get_main_name(), "##" + mod_cfg.get_main_name(), mod_cfg.get_description(), 
                        mod_cfg.get_version(), mod_cfg.get_author(), path_replacement, 
                        {}, 0
                    }
                ));

                auto& mod = m_hot_swaps.value().back();
                mod->on_ptr = &mod->is_on;
                if(mod->character){
                    m_show_costume_options = true;

                    mod->costume_name = UI_String_t(mod->w_name.c_str());
                    /*mod->costume_name.value().reference.classHash = 0xDFC8C592;
                    mod->costume_name.value().reference.noIdea1 = 0xECFF;
                    mod->costume_name.value().reference.noIdea2 = 0x4DB9;
                    mod->costume_name.value().reference.noIdea3 = 0x838FF3576C3BE2A8;*/

                    switch(mod->character.value()){
                    case (uint8_t)HotSwapCFG::CharacterValue::ev_nero:
                        m_nero_csize = true;
                        mod->costume_id = 4 + nero_costume_slot;
                        mod->slot_in_select_menu = nero_costume_slot++;
                        m_nero_swaps.value().push_back(mod);
                        m_nero_extra_costumes.push_back(mod->costume_id);
                        m_nero_costume_count++;
                        break;
                    case (uint8_t)HotSwapCFG::CharacterValue::ev_dante:
                        m_dante_csize = true;
                        mod->costume_id = 4 + dante_costume_slot;
                        mod->slot_in_select_menu = dante_costume_slot++;
                        m_dante_swaps.value().push_back(mod);
                        m_dante_extra_costumes.push_back(mod->costume_id);
                        m_dante_costume_count++;
                        break;
                    case (uint8_t)HotSwapCFG::CharacterValue::ev_v:
                        m_gilver_csize = true;
                        mod->costume_id = 4 + gilver_costume_slot;
                        mod->slot_in_select_menu = gilver_costume_slot++;
                        m_gilver_swaps.value().push_back(mod);
                        m_gilver_extra_costumes.push_back(mod->costume_id);
                        m_gilver_costume_count++;
                        break;
                    case (uint8_t)HotSwapCFG::CharacterValue::ev_vergil:
                        m_vergil_csize = true;
                        mod->costume_id = 4 + vergil_costume_slot;
                        mod->slot_in_select_menu = vergil_costume_slot++;
                        m_vergil_swaps.value().push_back(mod);
                        m_vergil_extra_costumes.push_back(mod->costume_id);
                        m_vergil_costume_count++;
                        break;
                    default:
                        break;
                    }


                }
            }
        }
    }
    
}

void FileEditor::load_sys_mods(){
    const std::string cfg_name = "modinfo";
    const std::string cfg_ext = ".ini";
    m_sys_mod_roots = list_dirs("natives/x64/System");
    if (m_sys_mod_roots) {
        for (auto mod_root : *m_sys_mod_roots) {
            // Read Mod Info
            auto mod_cfg = HotSwapCFG(mod_root / (cfg_name + cfg_ext));
            //get mod name
            auto mod_name = mod_root.filename()/*not actually "file" name, but the folder name*/.string();
            //mod files is a list of all lifes (whats? :/) not including the .ini? yes
            auto mod_files = list_files(fs::path("natives/x64/System") / mod_root.filename()/*same deal as above*/ / "natives/x64", {}, cfg_ext);
            if (mod_files) {
                //getting rid of the stupid extenstions
                for (auto& file : *mod_files) {
                    file = file.parent_path() / file.stem();
                }
                //converting the path to the format the game uses (starting from insidethe x64 folder as the root)
                auto original_mod_file_paths = *mod_files;
                for (auto& file : original_mod_file_paths) {
                    auto file_path = file.string();
                    file = file_path.substr(file_path.find("natives/x64/System\\" + mod_root.filename().string()) + (("natives/x64/System\\" + mod_name + "natives/x64\\").size()) + 1);
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

                    path_replacement.push_back({ org_path, new_path });
                }

                //filling the array with our mods
                auto priority = m_sys_hot_swaps.value().size();
                //Asset_Hotswap hotswap{ false, priority, mod_cfg.get_character(), {}, mod_cfg.get_name(), mod_cfg.get_main_name(), "##" + mod_cfg.get_main_name(), mod_cfg.get_description(), mod_cfg.get_version(), mod_cfg.get_author(), path_replacement};
                m_sys_hot_swaps.value().push_back(std::make_shared<Asset_Hotswap>(
                    Asset_Hotswap{
                        true, priority, mod_cfg.get_character(),
                        {}, mod_cfg.get_name(), L"Test String",
                        mod_cfg.get_main_name(), "##" + mod_cfg.get_main_name(), mod_cfg.get_description(),
                        mod_cfg.get_version(), mod_cfg.get_author(), path_replacement,
                        {}, 0
                    }
                ));

                auto& mod = m_sys_hot_swaps.value().back();
                mod->on_ptr = &mod->is_on;
            }
        }
    bind_sys_mod("LDK", &LDK::cheaton);
    bind_sys_mod("SWVoid",&SpardaWorkshop::cheaton);
    bind_sys_mod("SWBP",&SpardaWorkshop::bp_cheaton);
    bind_sys_mod("SWBP", &SpardaWorkshop::bp_cheaton);
    bind_sys_mod("BPExtra",&BpStageJump::altfloor);
    }
}


void FileEditor::bind_sys_mod(std::string modname, bool* on_value) {
    if (m_sys_hot_swaps) {
        for (auto& asset_mod : *m_sys_hot_swaps) {
            if (asset_mod->name == modname) {
                asset_mod->on_ptr = on_value;
            }
        }
    }
};

// during load
void FileEditor::on_config_load(const utility::Config &cfg) {
    m_config = cfg;

    for (auto& asset_mod : *m_hot_swaps) {
        if(!asset_mod->character){
            *asset_mod->on_ptr = cfg.get<bool>("AssetMod(\"" + asset_mod->main_name + "\")Enable").value_or(false);
            asset_mod->priority = cfg.get<unsigned int>("AssetMod(\"" + asset_mod->main_name + "\")Priority").value_or(asset_mod->priority);
        }
    }
    std::sort(m_hot_swaps.value().begin(), m_hot_swaps.value().end(), [](const std::shared_ptr<Asset_Hotswap>& a, const std::shared_ptr<Asset_Hotswap>& b){return a->priority < b->priority;});

    m_load_super = cfg.get<bool>("AssetMod_SuperCostume").value_or(false);
    m_load_ex = cfg.get<bool>("AssetMod_ExCostume").value_or(false);

    for (auto& costume : *m_nero_swaps) {
        *costume->on_ptr = cfg.get<bool>("CostumeMod(\"" + costume->main_name + "\")Enable").value_or(false);
        if(*costume->on_ptr) break;
    }

    for (auto& costume : *m_dante_swaps) {
        *costume->on_ptr = cfg.get<bool>("CostumeMod(\"" + costume->main_name + "\")Enable").value_or(false);
        if(*costume->on_ptr) break;
    }

    for (auto& costume : *m_gilver_swaps) {
        *costume->on_ptr = cfg.get<bool>("CostumeMod(\"" + costume->main_name + "\")Enable").value_or(false);
        if(*costume->on_ptr) break;
    }

    for (auto& costume : *m_vergil_swaps) {
        *costume->on_ptr = cfg.get<bool>("CostumeMod(\"" + costume->main_name + "\")Enable").value_or(false);
        if(*costume->on_ptr) break;
    }
}
// during save
void FileEditor::on_config_save(utility::Config &cfg) {
    for (auto& asset_mod : *m_hot_swaps) {
        if(!asset_mod->character){
            cfg.set<bool>("AssetMod(\"" + asset_mod->main_name + "\")Enable", *asset_mod->on_ptr);
            cfg.set<unsigned int>("AssetMod(\"" + asset_mod->main_name + "\")Priority", asset_mod->priority);
        }
    }

    cfg.set<bool>("AssetMod_SuperCostume", m_load_super);
    cfg.set<bool>("AssetMod_ExCostume", m_load_ex);

    for (auto& costume : *m_nero_swaps) {
        cfg.set<bool>("CostumeMod(\"" + costume->main_name + "\")Enable", *costume->on_ptr);
    }

    for (auto& costume : *m_dante_swaps) {
        cfg.set<bool>("CostumeMod(\"" + costume->main_name + "\")Enable", *costume->on_ptr);
    }

    for (auto& costume : *m_gilver_swaps) {
        cfg.set<bool>("CostumeMod(\"" + costume->main_name + "\")Enable", *costume->on_ptr);
    }

    for (auto& costume : *m_vergil_swaps) {
        cfg.set<bool>("CostumeMod(\"" + costume->main_name + "\")Enable", *costume->on_ptr);
    }
}
// do something every frame
// void FileEditor::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void FileEditor::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here

void FileEditor::asset_swap_ui(std::optional<std::vector<std::shared_ptr<Asset_Hotswap>>>& hot_swaps)
{
    for (UINT n = 0; n < hot_swaps.value().size(); n++) {
        auto& asset_mod = hot_swaps.value()[n];

        if(asset_mod->character) continue;

        ImGui::Checkbox(asset_mod->label.c_str(), asset_mod->on_ptr);
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

            if (delta != 0) {
                int n_next = n + delta;
                if (n_next >= 0 && n_next < hot_swaps.value().size() && !hot_swaps.value()[n]->character && !hot_swaps.value()[n_next]->character)
                {
                    std::swap(hot_swaps.value()[n]->priority, hot_swaps.value()[n_next]->priority);
                    std::swap(hot_swaps.value()[n], hot_swaps.value()[n_next]);
                    ImGui::ResetMouseDragDelta();
                }
            }
        }

        if (node_open) {
            if (asset_mod->author) {
                ImGui::SetCursorPosX(65.f);
                ImGui::TextWrapped("Author: %s", asset_mod->author.value().c_str());
            }
            if (asset_mod->version) {
                ImGui::SetCursorPosX(65.f);
                ImGui::TextWrapped("Version: %s", asset_mod->version.value().c_str());
            }
            if (asset_mod->description) {
                ImGui::SetCursorPosX(65.f);
                ImGui::TextWrapped("Description: %s", asset_mod->description.value().c_str());
            }
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::TreePop();
        }
    }
}

void FileEditor::costume_swap_ui(std::optional<std::vector<std::shared_ptr<Asset_Hotswap>>>& costume_swaps)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    for (UINT n = 0; n < costume_swaps.value().size(); n++) {
        auto& asset_mod = costume_swaps.value()[n];

        if(*asset_mod->on_ptr){
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(253, 215, 1)));
        }
        bool node_open = ImGui::TreeNodeEx(asset_mod->name.c_str());
        if(*asset_mod->on_ptr){

            ImGui::PopStyleColor();
        }

        if (node_open) {
            if (asset_mod->author) {
                ImGui::TextWrapped("Author: %s", asset_mod->author.value().c_str());
            }
            if (asset_mod->version) {
                ImGui::TextWrapped("Version: %s", asset_mod->version.value().c_str());
            }
            if (asset_mod->description) {
                ImGui::TextWrapped("Description: %s", asset_mod->description.value().c_str());
            }
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::TreePop();
        }
    }
}

void FileEditor::on_draw_ui() {
    if (ImGui::Button("Reload Mods")) {  
        std::map<std::string, Info_Back> priorities;
        
        for (auto& asset_mod : *m_hot_swaps) {
            priorities[asset_mod->main_name] = Info_Back{ *asset_mod->on_ptr, asset_mod->priority };
        }
        
        load_mods();

        for (auto& asset_mod : *m_hot_swaps) {
            if(priorities.find(asset_mod->main_name) != priorities.end()) {
                auto& mod_info_back = priorities[asset_mod->main_name];
                *asset_mod->on_ptr = mod_info_back.is_on;
                asset_mod->priority = mod_info_back.priority;
            } else {
                asset_mod->priority = -1;
            }
        }

        std::sort(m_hot_swaps.value().begin(), m_hot_swaps.value().end(), [](const std::shared_ptr<Asset_Hotswap>& a, const std::shared_ptr<Asset_Hotswap>& b){return a->priority < b->priority;});
    }

    if(m_show_costume_options){
        ImGui::SameLine();
        ImGui::Checkbox("EX Costume", &m_load_ex);
        ImGui::SameLine();
        ImGui::Checkbox("Super Costume", &m_load_super);

        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(ImColor(0, 0, 0, 0)));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(ImColor(0, 0, 0, 0)));

        ImGui::Text("Nero Costumes");
        costume_swap_ui(m_nero_swaps);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Dante Costumes");
        costume_swap_ui(m_dante_swaps);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("V Costumes");
        costume_swap_ui(m_gilver_swaps);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Vergil Costumes");
        costume_swap_ui(m_vergil_swaps);

        ImGui::PopStyleColor(2);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Mods");
    asset_swap_ui(m_hot_swaps);
}

bool FileEditor::asset_check(const wchar_t* game_path, const wchar_t* mod_path) const {
	while (*game_path) if (towlower(*game_path++) != towlower(*mod_path++)) return false;
	return true;
}

void __fastcall FileEditor::selected_costume_processor(uintptr_t RCX, uintptr_t ui2120GUI)
{
    uint32_t& selected_costume_slot = *(uint32_t*)(ui2120GUI + 0xCC);

    uint32_t og_slots = 4;

    switch (m_selected_character) {
    case (uint8_t)HotSwapCFG::CharacterValue::ev_nero:
    {
        // Enabling the costume that is selected and disabling the rest for that character
        for (auto& mod : *g_FileEditor->m_nero_swaps) {
            if (mod->slot_in_select_menu.value() + m_nero_last_og_costume_count == selected_costume_slot) {
                *mod->on_ptr = true;
            }
            else {
                *mod->on_ptr = false;
            }

            if (g_FileEditor->m_config)
                g_FileEditor->m_config.value().set<bool>("CostumeMod(\"" + mod->main_name + "\")Enable", *mod->on_ptr);
        }

        og_slots = m_nero_last_og_costume_count;
    }
        break;
    
    case (uint8_t)HotSwapCFG::CharacterValue::ev_dante:
    {
        // Enabling the costume that is selected and disabling the rest for that character
        for (auto& mod : *g_FileEditor->m_dante_swaps) {
            if (mod->slot_in_select_menu.value() + m_dante_last_og_costume_count == selected_costume_slot) {
                *mod->on_ptr = true;
            }
            else {
                *mod->on_ptr = false;
            }

            if (g_FileEditor->m_config)
                g_FileEditor->m_config.value().set<bool>("CostumeMod(\"" + mod->main_name + "\")Enable", *mod->on_ptr);
        }

        og_slots = m_dante_last_og_costume_count;
    }
        break;

    case (uint8_t)HotSwapCFG::CharacterValue::ev_v:
    {
        // Enabling the costume that is selected and disabling the rest for that character
        for (auto& mod : *g_FileEditor->m_gilver_swaps) {
            if (mod->slot_in_select_menu.value() + m_gilver_last_og_costume_count == selected_costume_slot) {
                *mod->on_ptr = true;
            }
            else {
                *mod->on_ptr = false;
            }

            if (g_FileEditor->m_config)
                g_FileEditor->m_config.value().set<bool>("CostumeMod(\"" + mod->main_name + "\")Enable", *mod->on_ptr);
        }

        og_slots = m_gilver_last_og_costume_count;
    }
        break;

    case (uint8_t)HotSwapCFG::CharacterValue::ev_vergil:
    {
        // Enabling the costume that is selected and disabling the rest for that character
        for (auto& mod : *g_FileEditor->m_vergil_swaps) {
            if (mod->slot_in_select_menu.value() + m_vergil_last_og_costume_count == selected_costume_slot) {
                *mod->on_ptr = true;
            }
            else {
                *mod->on_ptr = false;
            }

            if (g_FileEditor->m_config) 
                g_FileEditor->m_config.value().set<bool>("CostumeMod(\"" + mod->main_name + "\")Enable", *mod->on_ptr);
        }

        og_slots = m_vergil_last_og_costume_count;
    }
        break;

    default:
        break;
    }

    
    // Don't mess with it if they chose a default costume
    if(selected_costume_slot > og_slots - 1){
        // Now that they selected a costume mod mess with it
        if(m_selected_char_costume_info){
            if (!m_load_ex) {
                if (m_load_super) {
                    // Original super
                    selected_costume_slot = m_selected_char_costume_info.value().org_super;
                }
                else {
                    // Original non super
                    selected_costume_slot = m_selected_char_costume_info.value().org;
                }
            }
            else {
                if (m_load_super) {
                    // Ex Super
                    selected_costume_slot = m_selected_char_costume_info.value().ex_super;
                }
                else {
                    // Ex non super
                    selected_costume_slot = m_selected_char_costume_info.value().ex;
                }
            }
        }
    }

    // Save the enabled costumes in the config file so the next time we run the game it would load the costume that was last selected
    if(g_FileEditor->m_config) g_FileEditor->m_config.value().save();

    auto selected_costume_processor_fn = m_selected_costume_processor_hook->get_original<decltype(FileEditor::selected_costume_processor_internal)>();
    selected_costume_processor_fn(RCX, ui2120GUI);
}

void __fastcall FileEditor::selected_costume_processor_internal(uintptr_t RCX, uintptr_t ui2120GUI)
{
    g_FileEditor->selected_costume_processor(RCX, ui2120GUI);
}

void __fastcall FileEditor::costume_list_maker(uintptr_t RCX, uintptr_t ui2120GUI)
{
    uint32_t character = *(uint32_t*)(ui2120GUI + 0xE8) + 1;
    auto& original_costume_list = *(Costume_List_t*)(*(uintptr_t*)(*(uintptr_t*)(ui2120GUI + 0xE0) + 0x10));

    // Clearing the last loaded extra portrait ID list to load the current one
    std::vector<uint32_t> extra_costumes{};

    // This part retirieves the original costumes' positions
    uint8_t org_super = 0xFF;
    uint8_t org = 0xFF;
    uint8_t ex = 0xFF;
    uint8_t ex_super = 0xFF;
    uint32_t originalg_costume_count = original_costume_list.size;
    uint32_t total_costume_count = original_costume_list.size;

    switch (character) {
    case (uint8_t)HotSwapCFG::CharacterValue::ev_nero:
        originalg_costume_count = m_nero_last_og_costume_count;
        total_costume_count = m_nero_costume_count;
        extra_costumes = m_nero_extra_costumes;
        break;
    
    case (uint8_t)HotSwapCFG::CharacterValue::ev_dante:
        originalg_costume_count = m_dante_last_og_costume_count;
        total_costume_count = m_dante_costume_count;
        extra_costumes = m_dante_extra_costumes;
        break;

    case (uint8_t)HotSwapCFG::CharacterValue::ev_v:
        originalg_costume_count = m_gilver_last_og_costume_count;
        total_costume_count = m_gilver_costume_count;
        extra_costumes = m_gilver_extra_costumes;
        break;

    case (uint8_t)HotSwapCFG::CharacterValue::ev_vergil:
        originalg_costume_count = m_vergil_last_og_costume_count;
        total_costume_count = m_vergil_costume_count;
        extra_costumes = m_vergil_extra_costumes;
        break;

    default:
        break;
    }

    // Iterating through the first elemnts that are unlocked of the array to find the slot of default costumes in case the user doesn't have all 4 costumes unlocked
    for (UINT i = 0; i < originalg_costume_count; i++) {
        // Switch on the slot
        switch (original_costume_list[i]) {
        case 0: // Original
            org = i;
            break;

        case 1: // Ex Color
            ex = i;
            break;

        case 2: // Original Super
            org_super = i;
            break;

        case 3: // Ex Color Super
            ex_super = i;
            break;

        default:
            break;
        }
    }

    if(org == 0xFF) org = 1;
    if(ex == 0xFF)  ex = org;
    if(org_super == 0xFF)  org_super = org;
    if(ex_super == 0xFF)  ex_super = org;

    // Saving the slot of each costume into our info struct
    m_selected_char_costume_info = {org_super, org, ex, ex_super};

    // This part manages the portrait shown for each costume

    m_costume_list_container.clear();
    uint32_t list_size = originalg_costume_count + extra_costumes.size();
    m_costume_list_container.reserve(sizeof(Costume_List_t) + (list_size < 4 ? 4 : list_size) * sizeof(uint32_t));
    m_new_costume_list_p = new(m_costume_list_container.data()) Costume_List_t(original_costume_list, originalg_costume_count, extra_costumes);
    // Hacking...
    //std::cout << (void*)m_new_costume_list_p << "Done!" << std::endl;
    auto costume_list_maker_fn = m_costume_list_maker_hook->get_original<decltype(FileEditor::costume_list_maker_internal)>();
    costume_list_maker_fn(RCX, ui2120GUI);
}

void __fastcall FileEditor::costume_list_maker_internal(uintptr_t RCX, uintptr_t ui2120GUI)
{
    g_FileEditor->costume_list_maker(RCX, ui2120GUI);
}

void* __fastcall FileEditor::file_loader(uintptr_t this_p, uintptr_t RDX, const wchar_t* file_path)
{
    auto _sys = [&]() {
        if (m_sys_hot_swaps) {
            for (auto& asset_mod : *m_sys_hot_swaps) {
                if (*asset_mod->on_ptr) {
                    for (auto& mod_replace_paths : asset_mod->redirection_list) {
                        if (asset_check(mod_replace_paths.org_path.c_str(), file_path)) {
                            file_path = mod_replace_paths.new_path.c_str();
                            return;
                        }
                    }
                }
            }
        }
    }; _sys();

    auto _mod = [&](){
        if(m_is_active && m_hot_swaps){
            for (auto& asset_mod : *m_hot_swaps) {
                if(*asset_mod->on_ptr){
                    for(auto& mod_replace_paths : asset_mod->redirection_list){
                        if (asset_check(mod_replace_paths.org_path.c_str(), file_path)) {
                            file_path = mod_replace_paths.new_path.c_str();
                            return;
                        }
                    }
                }
            }
        }
    }; _mod();


    auto file_loader_fn = m_file_loader_hook->get_original<decltype(FileEditor::file_loader_internal)>();
	return file_loader_fn(this_p, RDX, file_path);
}

void* __fastcall FileEditor::file_loader_internal(uintptr_t this_p, uintptr_t RDX, const wchar_t* file_path)
{
    return g_FileEditor->file_loader(this_p, RDX, file_path);
}



UI_String_t* __fastcall FileEditor::ui_costume_name(uintptr_t RCX, uintptr_t RDX, uint32_t costume_id)
{
    //std::cout << "ID: " << costume_id << std::endl;

    auto fn_ui_costume_name = m_ui_costume_name_hook->get_original<decltype(FileEditor::ui_costume_name_internal)>();

    UI_String_t* ui_string = fn_ui_costume_name(RCX, RDX, costume_id);

    auto _ = [&](){
        switch (m_selected_character) {
        case (uint8_t)HotSwapCFG::CharacterValue::ev_nero:
            for (auto& mod : *m_nero_swaps) {
                if(mod->costume_id == costume_id){
                    ui_string = &mod->costume_name.value();
                    return;
                }
            }
            break;

        case (uint8_t)HotSwapCFG::CharacterValue::ev_dante:
            for (auto& mod : *m_dante_swaps) {
                if(mod->costume_id == costume_id){
                    ui_string = &mod->costume_name.value();
                    return;
                }
            }
            break;

        case (uint8_t)HotSwapCFG::CharacterValue::ev_v:
            for (auto& mod : *m_gilver_swaps) {
                if(mod->costume_id == costume_id){
                    ui_string = &mod->costume_name.value();
                    return;
                }
            }
            break;

        case (uint8_t)HotSwapCFG::CharacterValue::ev_vergil:
            for (auto& mod : *m_vergil_swaps) {
                if(mod->costume_id == costume_id){
                    ui_string = &mod->costume_name.value();
                    //std::wcout << mod->costume_name.value().m_English << std::endl;
                    return;
                }
            }
            break;

        default:
            break;
        }
    };_();

    //std::cout << m_selected_character << ' ' << costume_id << ' ' << (void*)ui_string << std::endl;


    return ui_string;
}

UI_String_t* __fastcall FileEditor::ui_costume_name_internal(uintptr_t RCX, uintptr_t RDX, uint32_t costume_id)
{
    return g_FileEditor->ui_costume_name(RCX, RDX, costume_id);
}

HotSwapCFG::HotSwapCFG(fs::path cfg_path)
     :m_cfg_path{ cfg_path }, m_mod_name { cfg_path.parent_path().filename().string() }, m_main_name{ cfg_path.parent_path().filename().string() },
      m_description {}, m_version {}, m_author {}
{
    //check if the mod includes the config file
    if(!fs::exists(m_cfg_path)) return;

    //map strings to enum
	m_variable_map["name"] = StringValue::ev_mod_name;
	m_variable_map["description"] = StringValue::ev_description;
	m_variable_map["version"] = StringValue::ev_version;
	m_variable_map["author"] = StringValue::ev_author;
    m_variable_map["character"] = StringValue::ev_character;
    //map characters to enum
    m_char_name_map["nero"] = CharacterValue::ev_nero;
    m_char_name_map["dante"] = CharacterValue::ev_dante;
    m_char_name_map["v"] = CharacterValue::ev_v;
    m_char_name_map["vergil"] = CharacterValue::ev_vergil;
    //input file stream
    std::ifstream cfg(cfg_path.string());
    //if file stream exists
    if (cfg) {
        //line by line
        for (std::string line{}; getline(cfg, line); ) {
            //convert line to stream
            std::istringstream ss{ line };
            std::string variable{};
            std::string value{};
            //split string into variable and value
            if(line.find('=') != std::string::npos){
                getline(ss, variable, '=');
                getline(ss, value);
                //process line
                process_line(variable, value);
            }
        }
    }
}

void HotSwapCFG::process_line(std::string variable, std::string value)
{
    if (!variable.empty() && !value.empty()) {
        //use variable string in stringmap to determine enum
        switch (m_variable_map[str_to_lower(variable)]) {
        //assign to member variables appropriately
        case StringValue::ev_mod_name:
            m_mod_name = value;
            break;

        case StringValue::ev_description:
            m_description = value;
            break;

        case StringValue::ev_version:
            m_version = value;
            break;

        case StringValue::ev_author:
            m_author = value;
            break;

        case StringValue::ev_character:
            m_character = (uint8_t)m_char_name_map[str_to_lower(value)];
            break;

        default:
            break;
        }
    }
}

