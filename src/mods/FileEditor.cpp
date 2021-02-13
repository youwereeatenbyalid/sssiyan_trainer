
#include "FileEditor.hpp"
#include "mods/PlayerTracker.hpp"
#include <filesystem>


bool FileEditor::cheaton{NULL};
wchar_t FileEditor::dantenormalcostume[] = L"Character/Player/pl0100_Dante";
wchar_t FileEditor::dantebosscostume[] = L"Character/Player/pl0130_Dante";
wchar_t FileEditor::dantedmc2costume[] = L"Character/Player/pl0101_Dante";
std::vector<std::basic_string<wchar_t>> FileEditor::filenames;
FileEditor* g_hook = nullptr;
// clang-format off
// only in clang/icl mode on x64, sorry
FileEditor::FileEditor() {
	g_hook = this;
}

void* __fastcall FileEditor::function_hook(uintptr_t this_p, uintptr_t RDX, wchar_t* file_path) {
	return g_hook->file_loader(this_p, RDX, file_path);
}

void* __fastcall FileEditor::file_loader(uintptr_t this_p, uintptr_t RDX, wchar_t* file_path){
	wchar_t * substring = wcsstr(file_path, FileEditor::dantenormalcostume);
	if (substring != NULL){
		for(auto& filename : FileEditor::filenames){
			if(wcsstr(file_path,filename.c_str()) != NULL){
				wcsncpy(substring, dantedmc2costume, 29);
			}
		}
		
	}
	substring = wcsstr(file_path, FileEditor::dantebosscostume);
	if (substring != NULL) {
		for (auto& filename : FileEditor::filenames) {
			if (wcsstr(file_path, filename.c_str()) != NULL) {
				wcsncpy(substring, dantedmc2costume, 29);
			}
		}

	}
	auto ret = m_sub_function_hook->get_original<decltype(FileEditor::function_hook)>()(this_p, RDX, file_path);
	return ret;
}
void FileEditor::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> FileEditor::on_initialize() {
  init_check_box_info();
  namespace fs = std::filesystem;
  fs::path folderpath = fs::current_path();
  folderpath /= "natives\\x64\\character\\player\\pl0101_dante";
  
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  ischecked = &FileEditor::cheaton;
  //onpage    = commonpage;
  full_name_string     = "FileEditor Full Name";
  author_string        = "Author";
  description_string   = "This is the description of FileEditor.";

  auto addr = utility::scan(base, "40 53 57 41 55 41 57 48 83 EC 48 49");
  if (!addr) {
    return "Unable to find FileEditor pattern.";
  }
  m_sub_function_hook = std::make_unique<FunctionHook>(addr.value(),&function_hook);
  m_sub_function_hook->create();

  //attempt to iterate

  for (auto& p : fs::recursive_directory_iterator(folderpath)){
   if (p.is_regular_file())
	  filenames.push_back(p.path().stem());
  }
	  
  return Mod::on_initialize();
}

// during load
void FileEditor::on_config_load(const utility::Config &cfg) {}
// during save
void FileEditor::on_config_save(utility::Config &cfg) {}
// do something every frame
void FileEditor::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
void FileEditor::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void FileEditor::on_draw_ui() {}
