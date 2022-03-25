
#include "TextEditor.hpp"
#include "mods/PlayerTracker.hpp"
#include "mods/LDK.hpp"
#include "mods/SpardaWorkshop.hpp"
uintptr_t TextEditor::jmp_ret{NULL};
bool TextEditor::cheaton{NULL};
wchar_t TextEditor::ldkcomp[] = L"LDKSTRINGREPLACE______";
wchar_t TextEditor::voidcomp[] = L"SWSTRINGREPLACE__";
wchar_t TextEditor::voidpausecomp[] = L"SWPAUSEMENU_____";
wchar_t TextEditor::voidconfirmcomp[] = L"VOIDCONFIRMSTRING";
wchar_t TextEditor::voiddescriptioncomp[] = L"SWMAINMENU_____";
wchar_t TextEditor::ldkdescriptioncomp[] = L"SOSDESCRIPTIONSTRING";
wchar_t TextEditor::ldkstring[] = L"LEGENDARY DARK KNIGHTS";
wchar_t TextEditor::sosstring[] =L"SON OF SPARDA";

wchar_t TextEditor::voidstring[] = L"THE VOID";
wchar_t TextEditor::swstring[] = L"SPARDA'S WORKSHOP";

wchar_t TextEditor::voidpausestring[] = L"Leave the void.";
wchar_t TextEditor::swpausestring[] = L"Leave Sparda's Workshop.";

wchar_t TextEditor::voidconfirmstring[] = L"Quit the Void?";
wchar_t TextEditor::swconfirmstring[] = L"Quit Sparda's Workshop?";

wchar_t TextEditor::voiddescriptionstring[] = L"Practice controls and moves.";
wchar_t TextEditor::swdescriptionstring[] = L"Practice advanced combos in an environmental sandbox.";

wchar_t TextEditor::sosdescriptionstring[] = L"You'll have to be stylish to\r\nsurvive this mode.";
wchar_t TextEditor::ldkdescriptionstring[] = L"A challenge fit for Sparda.\r\nBring your friends, you'll need them.";
TextEditor* g_hook = nullptr;

TextEditor::TextEditor() {
	g_hook = this;
}
// clang-format off
// only in clang/icl mode on x64, sorry
void* TextEditor::function_hook(int* a1, wchar_t* a2){
	return g_hook->string_replace(a1, a2);
}
void* TextEditor::string_replace(int* a1, wchar_t* a2){
	wchar_t* substring;
	
	if (wcsstr(a2, TextEditor::ldkcomp) != NULL){
		if(LDK::cheaton){
			a2 = TextEditor::ldkstring;
		}else{
			a2 = TextEditor::sosstring;
		}
	}
	else if(wcsstr(a2, TextEditor::voidcomp)!= NULL){
		if (SpardaWorkshop::cheaton) {
			a2 = TextEditor::swstring;
		}
		else {
			a2 = TextEditor::voidstring;
		}
	}
	else if (wcsstr(a2, TextEditor::voidpausecomp) != NULL) {
		if (SpardaWorkshop::cheaton) {
			a2 = TextEditor::swpausestring;
		}
		else {
			a2 = TextEditor::voidpausestring;
		}
	}
	else if (wcsstr(a2, TextEditor::voidconfirmcomp) != NULL) { //Doesn't work for whatever reason.
		if (SpardaWorkshop::cheaton) {
			a2 = TextEditor::swconfirmstring;
		}
		else {
			a2 = TextEditor::voidconfirmstring;
		}
	}
	else if (wcsstr(a2, TextEditor::voiddescriptioncomp) != NULL) {
		if (SpardaWorkshop::cheaton) {
			a2 = TextEditor::swdescriptionstring;
		}
		else {
			a2 = TextEditor::voiddescriptionstring;
		}
	}
	else if (wcsstr(a2, TextEditor::ldkdescriptioncomp) != NULL) {
		if (LDK::cheaton) {
			a2 = TextEditor::ldkdescriptionstring;
		}
		else {
			a2 = TextEditor::sosdescriptionstring;
		}
	}

	auto ret = m_sub_function_hook->get_original<decltype(TextEditor::function_hook)>()(a1, a2);
	return ret;
}
void TextEditor::init_check_box_info() {
  m_check_box_name = m_prefix_check_box_name + std::string(get_name());
  m_hot_key_name   = m_prefix_hot_key_name + std::string(get_name());
}

std::optional<std::string> TextEditor::on_initialize() {
  init_check_box_info();
  auto base = g_framework->get_module().as<HMODULE>(); // note HMODULE
  m_is_enabled = &TextEditor::cheaton;
  //onpage    = commonpage;

  auto addr = m_patterns_cache->find_addr(base, "40 53 57 41 56 48 83 EC 20 45 33");
  if (!addr) {
	  return "Unable to find Text Editor Pattern.";
  }
  m_full_name_string     = "Text Editor";
  m_author_string        = "The HitchHiker";
  m_description_string   = "Replaces Strings.";

  set_up_hotkey();
  m_sub_function_hook = std::make_unique<FunctionHook>(addr.value(), &function_hook);
  m_sub_function_hook->create();

  return Mod::on_initialize();
}

// during load
// void TextEditor::on_config_load(const utility::Config &cfg) {}
// during save
// void TextEditor::on_config_save(utility::Config &cfg) {}
// do something every frame
// void TextEditor::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void TextEditor::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
// void TextEditor::on_draw_ui() {}
