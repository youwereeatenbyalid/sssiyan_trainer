#include "KeyBinder.hpp"

std::shared_ptr<KeyBinder> KeyBinder::m_instance{};

std::map<std::string, std::shared_ptr<KeyBinder::Binding>> KeyBinder::m_bindings{};

std::vector<std::shared_ptr<KeyBinder::Binding>> KeyBinder::m_bindingsVec{};

// Keyboard
KeyBinder::DIKeyList KeyBinder::m_lastKBState{};

// Controller
REGPK_Flag KeyBinder::m_lastControllerState{ REGPK_None };

size_t KeyBinder::m_bindCount{ 0 };

bool KeyBinder::m_isIgnoring{ false };

std::string_view KeyBinder::m_config_file{ KEYBIND_CONFIG_FILENAME };

std::mutex KeyBinder::m_inputMutex{};

utility::Config KeyBinder::m_config{ KEYBIND_CONFIG_FILENAME };

bool KeyBinder::IsModifierKey(const uint8_t& key)
{
	bool ret = false;

	switch (key)
	{
	case DIK_LSHIFT:
	case DIK_RSHIFT:
	case DIK_LCONTROL:
	case DIK_RCONTROL:
	case DIK_LALT:
	case DIK_RALT:
		ret = true;
		break;
	default:
		ret = false;
		break;
	}

	return ret;
}

std::string_view KeyBinder::DIKToS(const uint8_t& key)
{
	std::string_view ret = "NULL";

	switch (key)
	{
	case DIK_ESCAPE:
		ret = "Escape";
		break;
	case DIK_1:
		ret = "1";
		break;
	case DIK_2:
		ret = "2";
		break;
	case DIK_3:
		ret = "3";
		break;
	case DIK_4:
		ret = "4";
		break;
	case DIK_5:
		ret = "5";
		break;
	case DIK_6:
		ret = "6";
		break;
	case DIK_7:
		ret = "7";
		break;
	case DIK_8:
		ret = "8";
		break;
	case DIK_9:
		ret = "9";
		break;
	case DIK_0:
		ret = "0";
		break;
	case DIK_MINUS:
		ret = "Minus";
		break;
	case DIK_EQUALS:
		ret = "Equals";
		break;
	case DIK_BACKSPACE:
		ret = "Backspace";
		break;
	case DIK_TAB:
		ret = "Tab";
		break;
	case DIK_Q:
		ret = "Q";
		break;
	case DIK_W:
		ret = "W";
		break;
	case DIK_E:
		ret = "E";
		break;
	case DIK_R:
		ret = "R";
		break;
	case DIK_T:
		ret = "T";
		break;
	case DIK_Y:
		ret = "Y";
		break;
	case DIK_U:
		ret = "U";
		break;
	case DIK_I:
		ret = "I";
		break;
	case DIK_O:
		ret = "O";
		break;
	case DIK_P:
		ret = "P";
		break;
	case DIK_LBRACKET:
		ret = "LBracket";
		break;
	case DIK_RBRACKET:
		ret = "RBracket";
		break;
	case DIK_RETURN:
		ret = "Enter";
		break;
	case DIK_LCONTROL:
		ret = "LCtrl";
		break;
	case DIK_A:
		ret = "A";
		break;
	case DIK_S:
		ret = "S";
		break;
	case DIK_D:
		ret = "D";
		break;
	case DIK_F:
		ret = "F";
		break;
	case DIK_G:
		ret = "G";
		break;
	case DIK_H:
		ret = "H";
		break;
	case DIK_J:
		ret = "J";
		break;
	case DIK_K:
		ret = "K";
		break;
	case DIK_L:
		ret = "L";
		break;
	case DIK_SEMICOLON:
		ret = "Semicolon";
		break;
	case DIK_APOSTROPHE:
		ret = "Apostrophe";
		break;
	case DIK_GRAVE:
		ret = "`";
		break;
	case DIK_LSHIFT:
		ret = "LShift";
		break;
	case DIK_BACKSLASH:
		ret = "BackSlash";
		break;
	case DIK_Z:
		ret = "Z";
		break;
	case DIK_X:
		ret = "X";
		break;
	case DIK_C:
		ret = "C";
		break;
	case DIK_V:
		ret = "V";
		break;
	case DIK_B:
		ret = "B";
		break;
	case DIK_N:
		ret = "N";
		break;
	case DIK_M:
		ret = "M";
		break;
	case DIK_COMMA:
		ret = "Comma";
		break;
	case DIK_PERIOD:
		ret = "Period";
		break;
	case DIK_SLASH:
		ret = "Slash";
		break;
	case DIK_RSHIFT:
		ret = "RShift";
		break;
	case DIK_NUMPADSTAR:
		ret = "NumpadStar";
		break;
	case DIK_LALT:
		ret = "LAlt";
		break;
	case DIK_SPACE:
		ret = "Space";
		break;
	case DIK_CAPSLOCK:
		ret = "CapsLock";
		break;
	case DIK_F1:
		ret = "F1";
		break;
	case DIK_F2:
		ret = "F2";
		break;
	case DIK_F3:
		ret = "F3";
		break;
	case DIK_F4:
		ret = "F4";
		break;
	case DIK_F5:
		ret = "F5";
		break;
	case DIK_F6:
		ret = "F6";
		break;
	case DIK_F7:
		ret = "F7";
		break;
	case DIK_F8:
		ret = "F8";
		break;
	case DIK_F9:
		ret = "F9";
		break;
	case DIK_F10:
		ret = "F10";
		break;
	case DIK_NUMLOCK:
		ret = "Numlock";
		break;
	case DIK_SCROLL:
		ret = "Scroll";
		break;
	case DIK_NUMPAD7:
		ret = "Numpad7";
		break;
	case DIK_NUMPAD8:
		ret = "Numpad8";
		break;
	case DIK_NUMPAD9:
		ret = "Numpad9";
		break;
	case DIK_NUMPADMINUS:
		ret = "NumpadMinus";
		break;
	case DIK_NUMPAD4:
		ret = "Numpad4";
		break;
	case DIK_NUMPAD5:
		ret = "Numpad5";
		break;
	case DIK_NUMPAD6:
		ret = "Numpad6";
		break;
	case DIK_NUMPADPLUS:
		ret = "NumpadPlus";
		break;
	case DIK_NUMPAD1:
		ret = "Numpad1";
		break;
	case DIK_NUMPAD2:
		ret = "Numpad2";
		break;
	case DIK_NUMPAD3:
		ret = "Numpad3";
		break;
	case DIK_NUMPAD0:
		ret = "Numpad0";
		break;
	case DIK_NUMPADPERIOD:
		ret = "NumpadPeriod";
		break;
	case DIK_OEM_102:
		ret = "OEM_102";
		break;
	case DIK_F11:
		ret = "F11";
		break;
	case DIK_F12:
		ret = "F12";
		break;
	case DIK_F13:
		ret = "F13";
		break;
	case DIK_F14:
		ret = "F14";
		break;
	case DIK_F15:
		ret = "F15";
		break;
	case DIK_KANA:
		ret = "Kana";
		break;
	case DIK_ABNT_C1:
		ret = "ABNT_C1";
		break;
	case DIK_CONVERT:
		ret = "Convert";
		break;
	case DIK_NOCONVERT:
		ret = "NoConvert";
		break;
	case DIK_YEN:
		ret = "Yen";
		break;
	case DIK_ABNT_C2:
		ret = "ABNT_C2";
		break;
	case DIK_NUMPADEQUALS:
		ret = "NumpadEquals";
		break;
	case DIK_PREVTRACK:
		ret = "PrevTack";
		break;
	case DIK_AT:
		ret = "AT";
		break;
	case DIK_COLON:
		ret = "Colon";
		break;
	case DIK_UNDERLINE:
		ret = "Underline";
		break;
	case DIK_KANJI:
		ret = "Kanji";
		break;
	case DIK_STOP:
		ret = "Stop";
		break;
	case DIK_AX:
		ret = "AX";
		break;
	case DIK_UNLABELED:
		ret = "Unlabeled";
		break;
	case DIK_NEXTTRACK:
		ret = "NextTrack";
		break;
	case DIK_NUMPADENTER:
		ret = "NumpadEnter";
		break;
	case DIK_RCONTROL:
		ret = "RCtrl";
		break;
	case DIK_MUTE:
		ret = "Mute";
		break;
	case DIK_CALCULATOR:
		ret = "Calculator";
		break;
	case DIK_PLAYPAUSE:
		ret = "PlayPause";
		break;
	case DIK_MEDIASTOP:
		ret = "MediaStop";
		break;
	case DIK_VOLUMEDOWN:
		ret = "VolumeDown";
		break;
	case DIK_VOLUMEUP:
		ret = "VolumeUp";
		break;
	case DIK_WEBHOME:
		ret = "WebHome";
		break;
	case DIK_NUMPADCOMMA:
		ret = "NumpadComma";
		break;
	case DIK_NUMPADSLASH:
		ret = "NumpadSlash";
		break;
	case DIK_SYSRQ:
		ret = "PrintScreen";
		break;
	case DIK_RALT:
		ret = "RAlt";
		break;
	case DIK_PAUSE:
		ret = "Pause";
		break;
	case DIK_HOME:
		ret = "Home";
		break;
	case DIK_UPARROW:
		ret = "UpArrow";
		break;
	case DIK_PGUP:
		ret = "PGUp";
		break;
	case DIK_LEFTARROW:
		ret = "LeftArrow";
		break;
	case DIK_RIGHTARROW:
		ret = "RightArrow";
		break;
	case DIK_END:
		ret = "End";
		break;
	case DIK_DOWNARROW:
		ret = "DownArrow";
		break;
	case DIK_PGDN:
		ret = "PGDown";
		break;
	case DIK_INSERT:
		ret = "Insert";
		break;
	case DIK_DELETE:
		ret = "Delete";
		break;
	case DIK_LWIN:
		ret = "LWin";
		break;
	case DIK_RWIN:
		ret = "RWin";
		break;
	case DIK_APPS:
		ret = "Apps";
		break;
	case DIK_POWER:
		ret = "Power";
		break;
	case DIK_SLEEP:
		ret = "Sleep";
		break;
	case DIK_WAKE:
		ret = "Wake";
		break;
	case DIK_WEBSEARCH:
		ret = "WebSearch";
		break;
	case DIK_WEBFAVORITES:
		ret = "WebFavorites";
		break;
	case DIK_WEBREFRESH:
		ret = "WebRefresh";
		break;
	case DIK_WEBSTOP:
		ret = "WebStop";
		break;
	case DIK_WEBFORWARD:
		ret = "WebForward";
		break;
	case DIK_WEBBACK:
		ret = "WebBack";
		break;
	case DIK_MYCOMPUTER:
		ret = "MyComputer";
		break;
	case DIK_MAIL:
		ret = "Mail";
		break;
	case DIK_MEDIASELECT:
		ret = "MediaSelect";
		break;
	default:
		ret = "NULL";
		break;
	}

	return ret;
}

uint8_t KeyBinder::SToDIK(const std::string_view key)
{
	uint8_t ret = 0xFF;

	if (key == "Escape")
	{
		ret = DIK_ESCAPE;
	}
	else if (key == "1")
	{
		ret = DIK_1;
	}
	else if (key == "2")
	{
		ret = DIK_2;
	}
	else if (key == "3")
	{
		ret = DIK_3;
	}
	else if (key == "4")
	{
		ret = DIK_4;
	}
	else if (key == "5")
	{
		ret = DIK_5;
	}
	else if (key == "6")
	{
		ret = DIK_6;
	}
	else if (key == "7")
	{
		ret = DIK_7;
	}
	else if (key == "8")
	{
		ret = DIK_8;
	}
	else if (key == "9")
	{
		ret = DIK_9;
	}
	else if (key == "0")
	{
		ret = DIK_0;
	}
	else if (key == "Minus")
	{
		ret = DIK_MINUS;
	}
	else if (key == "Equals")
	{
		ret = DIK_EQUALS;
	}
	else if (key == "Back" || key == "Backspace")
	{
		ret = DIK_BACKSPACE;
	}
	else if (key == "Tab")
	{
		ret = DIK_TAB;
	}
	else if (key == "Q")
	{
		ret = DIK_Q;
	}
	else if (key == "W")
	{
		ret = DIK_W;
	}
	else if (key == "E")
	{
		ret = DIK_E;
	}
	else if (key == "R")
	{
		ret = DIK_R;
	}
	else if (key == "T")
	{
		ret = DIK_T;
	}
	else if (key == "Y")
	{
		ret = DIK_Y;
	}
	else if (key == "U")
	{
		ret = DIK_U;
	}
	else if (key == "I")
	{
		ret = DIK_I;
	}
	else if (key == "O")
	{
		ret = DIK_O;
	}
	else if (key == "P")
	{
		ret = DIK_P;
	}
	else if (key == "LBracket")
	{
		ret = DIK_LBRACKET;
	}
	else if (key == "RBracket")
	{
		ret = DIK_RBRACKET;
	}
	else if (key == "Enter")
	{
		ret = DIK_RETURN;
	}
	else if (key == "LCtrl")
	{
		ret = DIK_LCONTROL;
	}
	else if (key == "A")
	{
		ret = DIK_A;
	}
	else if (key == "S")
	{
		ret = DIK_S;
	}
	else if (key == "D")
	{
		ret = DIK_D;
	}
	else if (key == "F")
	{
		ret = DIK_F;
	}
	else if (key == "G")
	{
		ret = DIK_G;
	}
	else if (key == "H")
	{
		ret = DIK_H;
	}
	else if (key == "J")
	{
		ret = DIK_J;
	}
	else if (key == "K")
	{
		ret = DIK_K;
	}
	else if (key == "L")
	{
		ret = DIK_L;
	}
	else if (key == "Semicolon")
	{
		ret = DIK_SEMICOLON;
	}
	else if (key == "Apostrophe")
	{
		ret = DIK_APOSTROPHE;
	}
	else if (key == "`")
	{
		ret = DIK_GRAVE;
	}
	else if (key == "LShift")
	{
		ret = DIK_LSHIFT;
	}
	else if (key == "BackSlash")
	{
		ret = DIK_BACKSLASH;
	}
	else if (key == "Z")
	{
		ret = DIK_Z;
	}
	else if (key == "X")
	{
		ret = DIK_X;
	}
	else if (key == "C")
	{
		ret = DIK_C;
	}
	else if (key == "V")
	{
		ret = DIK_V;
	}
	else if (key == "B")
	{
		ret = DIK_B;
	}
	else if (key == "N")
	{
		ret = DIK_N;
	}
	else if (key == "M")
	{
		ret = DIK_M;
	}
	else if (key == "Comma")
	{
		ret = DIK_COMMA;
	}
	else if (key == "Period")
	{
		ret = DIK_PERIOD;
	}
	else if (key == "Slash")
	{
		ret = DIK_SLASH;
	}
	else if (key == "RShift")
	{
		ret = DIK_RSHIFT;
	}
	else if (key == "Multiply" || key == "NumpadStar")
	{
		ret = DIK_NUMPADSTAR;
	}
	else if (key == "LMenu" || key == "LAlt")
	{
		ret = DIK_LALT;
	}
	else if (key == "Space")
	{
		ret = DIK_SPACE;
	}
	else if (key == "Capital" || key == "CapsLock")
	{
		ret = DIK_CAPSLOCK;
	}
	else if (key == "F1")
	{
		ret = DIK_F1;
	}
	else if (key == "F2")
	{
		ret = DIK_F2;
	}
	else if (key == "F3")
	{
		ret = DIK_F3;
	}
	else if (key == "F4")
	{
		ret = DIK_F4;
	}
	else if (key == "F5")
	{
		ret = DIK_F5;
	}
	else if (key == "F6")
	{
		ret = DIK_F6;
	}
	else if (key == "F7")
	{
		ret = DIK_F7;
	}
	else if (key == "F8")
	{
		ret = DIK_F8;
	}
	else if (key == "F9")
	{
		ret = DIK_F9;
	}
	else if (key == "F10")
	{
		ret = DIK_F10;
	}
	else if (key == "Numlock")
	{
		ret = DIK_NUMLOCK;
	}
	else if (key == "Scroll")
	{
		ret = DIK_SCROLL;
	}
	else if (key == "Numpad7")
	{
		ret = DIK_NUMPAD7;
	}
	else if (key == "Numpad8")
	{
		ret = DIK_NUMPAD8;
	}
	else if (key == "Numpad9")
	{
		ret = DIK_NUMPAD9;
	}
	else if (key == "Subtract" || key == "NumpadMinus")
	{
		ret = DIK_NUMPADMINUS;
	}
	else if (key == "Numpad4")
	{
		ret = DIK_NUMPAD4;
	}
	else if (key == "Numpad5")
	{
		ret = DIK_NUMPAD5;
	}
	else if (key == "Numpad6")
	{
		ret = DIK_NUMPAD6;
	}
	else if (key == "Add" || key == "NumpadPlus")
	{
		ret = DIK_NUMPADPLUS;
	}
	else if (key == "Numpad1")
	{
		ret = DIK_NUMPAD1;
	}
	else if (key == "Numpad2")
	{
		ret = DIK_NUMPAD2;
	}
	else if (key == "Numpad3")
	{
		ret = DIK_NUMPAD3;
	}
	else if (key == "Numpad0")
	{
		ret = DIK_NUMPAD0;
	}
	else if (key == "Decimal" || key == "NumpadPeriod")
	{
		ret = DIK_NUMPADPERIOD;
	}
	else if (key == "OEM_102")
	{
		ret = DIK_OEM_102;
	}
	else if (key == "F11")
	{
		ret = DIK_F11;
	}
	else if (key == "F12")
	{
		ret = DIK_F12;
	}
	else if (key == "F13")
	{
		ret = DIK_F13;
	}
	else if (key == "F14")
	{
		ret = DIK_F14;
	}
	else if (key == "F15")
	{
		ret = DIK_F15;
	}
	else if (key == "Kana")
	{
		ret = DIK_KANA;
	}
	else if (key == "ABNT_C1")
	{
		ret = DIK_ABNT_C1;
	}
	else if (key == "Convert")
	{
		ret = DIK_CONVERT;
	}
	else if (key == "NoConvert")
	{
		ret = DIK_NOCONVERT;
	}
	else if (key == "Yen")
	{
		ret = DIK_YEN;
	}
	else if (key == "ABNT_C2")
	{
		ret = DIK_ABNT_C2;
	}
	else if (key == "NumpadEquals")
	{
		ret = DIK_NUMPADEQUALS;
	}
	else if (key == "PrevTack")
	{
		ret = DIK_PREVTRACK;
	}
	else if (key == "AT")
	{
		ret = DIK_AT;
	}
	else if (key == "Colon")
	{
		ret = DIK_COLON;
	}
	else if (key == "Underline")
	{
		ret = DIK_UNDERLINE;
	}
	else if (key == "Kanji")
	{
		ret = DIK_KANJI;
	}
	else if (key == "Stop")
	{
		ret = DIK_STOP;
	}
	else if (key == "AX")
	{
		ret = DIK_AX;
	}
	else if (key == "Unlabeled")
	{
		ret = DIK_UNLABELED;
	}
	else if (key == "NextTrack")
	{
		ret = DIK_NEXTTRACK;
	}
	else if (key == "NumpadEnter")
	{
		ret = DIK_NUMPADENTER;
	}
	else if (key == "RCtrl")
	{
		ret = DIK_RCONTROL;
	}
	else if (key == "Mute")
	{
		ret = DIK_MUTE;
	}
	else if (key == "Calculator")
	{
		ret = DIK_CALCULATOR;
	}
	else if (key == "PlayPause")
	{
		ret = DIK_PLAYPAUSE;
	}
	else if (key == "MediaStop")
	{
		ret = DIK_MEDIASTOP;
	}
	else if (key == "VolumeDown")
	{
		ret = DIK_VOLUMEDOWN;
	}
	else if (key == "VolumeUp")
	{
		ret = DIK_VOLUMEUP;
	}
	else if (key == "WebHome")
	{
		ret = DIK_WEBHOME;
	}
	else if (key == "NumpadComma")
	{
		ret = DIK_NUMPADCOMMA;
	}
	else if (key == "Divide" || key == "NumpadSlash")
	{
		ret = DIK_NUMPADSLASH;
	}
	else if (key == "PrintScreen")
	{
		ret = DIK_SYSRQ;
	}
	else if (key == "RMenu" || key == "RAlt")
	{
		ret = DIK_RALT;
	}
	else if (key == "Pause")
	{
		ret = DIK_PAUSE;
	}
	else if (key == "Home")
	{
		ret = DIK_HOME;
	}
	else if (key == "Up" || key == "UpArrow")
	{
		ret = DIK_UPARROW;
	}
	else if (key == "Prior" || key == "PGUp")
	{
		ret = DIK_PGUP;
	}
	else if (key == "LEFT" || key == "LeftArrow")
	{
		ret = DIK_LEFTARROW;
	}
	else if (key == "Right" || key == "RightArrow")
	{
		ret = DIK_RIGHTARROW;
	}
	else if (key == "End")
	{
		ret = DIK_END;
	}
	else if (key == "Down" || key == "DownArrow")
	{
		ret = DIK_DOWNARROW;
	}
	else if (key == "Next" || key == "PGDown")
	{
		ret = DIK_PGDN;
	}
	else if (key == "Insert")
	{
		ret = DIK_INSERT;
	}
	else if (key == "Delete")
	{
		ret = DIK_DELETE;
	}
	else if (key == "LWin")
	{
		ret = DIK_LWIN;
	}
	else if (key == "RWin")
	{
		ret = DIK_RWIN;
	}
	else if (key == "Apps")
	{
		ret = DIK_APPS;
	}
	else if (key == "Power")
	{
		ret = DIK_POWER;
	}
	else if (key == "Sleep")
	{
		ret = DIK_SLEEP;
	}
	else if (key == "Wake")
	{
		ret = DIK_WAKE;
	}
	else if (key == "WebSearch")
	{
		ret = DIK_WEBSEARCH;
	}
	else if (key == "WebFavorites")
	{
		ret = DIK_WEBFAVORITES;
	}
	else if (key == "WebRefresh")
	{
		ret = DIK_WEBREFRESH;
	}
	else if (key == "WebStop")
	{
		ret = DIK_WEBSTOP;
	}
	else if (key == "WebForward")
	{
		ret = DIK_WEBFORWARD;
	}
	else if (key == "WebBack")
	{
		ret = DIK_WEBBACK;
	}
	else if (key == "MyComputer")
	{
		ret = DIK_MYCOMPUTER;
	}
	else if (key == "Mail")
	{
		ret = DIK_MAIL;
	}
	else if (key == "MediaSelect")
	{
		ret = DIK_MEDIASELECT;
	}

	return ret;
}

std::string_view KeyBinder::XboxCtrlrKToS(const REGPK_Flag& key)
{
	std::string_view ret = "NULL";

	switch(key)
	{
	case REGPK_LUp:
		ret = "D-pad Up";
		break;
	case REGPK_LDown:
		ret = "D-pad Down";
		break;
	case REGPK_LLeft:
		ret = "D-pad Left";
		break;
	case REGPK_LRight:
		ret = "D-pad Right";
		break;
	case REGPK_RUp:
		ret = "Y";
		break;
	case REGPK_RDown:
		ret = "A";
		break;
	case REGPK_RLeft:
		ret = "X";
		break;
	case REGPK_RRight:
		ret = "B";
		break;
	case REGPK_LTrigTop:
		ret = "LB";
		break;
	case REGPK_LTrigBottom:
		ret = "LT";
		break;
	case REGPK_RTrigTop:
		ret = "RB";
		break;
	case REGPK_RTrigBottom:
		ret = "RT";
		break;
	case REGPK_LStickPush:
		ret = "LS";
		break;
	case REGPK_RStickPush:
		ret = "RS";
		break;
	case REGPK_CLeft:
		ret = "View";
		break;
	case REGPK_CRight:
		ret = "Menu";
		break;
	}

	return ret;
}

std::string_view KeyBinder::DSCtrlrKToS(const REGPK_Flag& key)
{
	std::string_view ret = "NULL";

	switch (key)
	{
	case REGPK_LUp:
		ret = "D-pad Up";
		break;
	case REGPK_LDown:
		ret = "D-pad Down";
		break;
	case REGPK_LLeft:
		ret = "D-pad Left";
		break;
	case REGPK_LRight:
		ret = "D-pad Right";
		break;
	case REGPK_RUp:
		ret = "Triangle";
		break;
	case REGPK_RDown:
		ret = "X";
		break;
	case REGPK_RLeft:
		ret = "Square";
		break;
	case REGPK_RRight:
		ret = "Circle";
		break;
	case REGPK_LTrigTop:
		ret = "L1";
		break;
	case REGPK_LTrigBottom:
		ret = "L2";
		break;
	case REGPK_RTrigTop:
		ret = "R1";
		break;
	case REGPK_RTrigBottom:
		ret = "R2";
		break;
	case REGPK_LStickPush:
		ret = "L3";
		break;
	case REGPK_RStickPush:
		ret = "R3";
		break;
	case REGPK_CLeft:
		ret = "Options";
		break;
	case REGPK_CRight:
		ret = "Share";
		break;
	}

	return ret;
}

REGPK_Flag KeyBinder::SToXboxCtrlrK(const std::string_view& key)
{
	REGPK_Flag ret = REGPK_None;

	if (key == "D-pad Up") {
		ret = REGPK_LUp;
	}
	else if (key == "D-pad Down") {
		ret = REGPK_LDown;
	}
	else if (key == "D-pad Left") {
		ret = REGPK_LLeft;
	}
	else if (key == "D-pad Right") {
		ret = REGPK_LRight;
	}
	else if (key == "Y") {
		ret = REGPK_RUp;
	}
	else if (key == "A") {
		ret = REGPK_RDown;
	}
	else if (key == "X") {
		ret = REGPK_RLeft;
	}
	else if (key == "B") {
		ret = REGPK_RRight;
	}
	else if (key == "LB") {
		ret = REGPK_LTrigTop;
	}
	else if (key == "LT") {
		ret = REGPK_LTrigBottom;
	}
	else if (key == "RB") {
		ret = REGPK_RTrigTop;
	}
	else if (key == "RT") {
		ret = REGPK_RTrigBottom;
	}
	else if (key == "LS") {
		ret = REGPK_LStickPush;
	}
	else if (key == "RS") {
		ret = REGPK_RStickPush;
	}
	else if (key == "View") {
		ret = REGPK_CLeft;
	}
	else if (key == "Menu") {
		ret = REGPK_CRight;
	}

	return ret;
}

REGPK_Flag KeyBinder::SToDSCtrlrK(const std::string_view& key)
{
	REGPK_Flag ret = REGPK_None;

	if (key == "D-pad Up") {
		ret = REGPK_LUp;
	}
	else if (key == "D-pad Down") {
		ret = REGPK_LDown;
	}
	else if (key == "D-pad Left") {
		ret = REGPK_LLeft;
	}
	else if (key == "D-pad Right") {
		ret = REGPK_LRight;
	}
	else if (key == "Triangle") {
		ret = REGPK_RUp;
	}
	else if (key == "X") {
		ret = REGPK_RDown;
	}
	else if (key == "Square") {
		ret = REGPK_RLeft;
	}
	else if (key == "Circle") {
		ret = REGPK_RRight;
	}
	else if (key == "L1") {
		ret = REGPK_LTrigTop;
	}
	else if (key == "L2") {
		ret = REGPK_LTrigBottom;
	}
	else if (key == "R1") {
		ret = REGPK_RTrigTop;
	}
	else if (key == "R2") {
		ret = REGPK_RTrigBottom;
	}
	else if (key == "L3") {
		ret = REGPK_LStickPush;
	}
	else if (key == "R3") {
		ret = REGPK_RStickPush;
	}
	else if (key == "Options") {
		ret = REGPK_CLeft;
	}
	else if (key == "Share") {
		ret = REGPK_CRight;
	}

	return ret;
}

void KeyBinder::AddBind(const std::string& name, const OptFunc& pressFunc, const OptFunc& holdFunc, const OptFunc& releaseFunc, Keys_t keys)
{
	AddBind(name, pressFunc, holdFunc, releaseFunc, keys.keyboard);

	SetGPKeys(name, keys.controller);
}

void KeyBinder::AddBind(const std::string& name, const OptFunc& func, Keys_t keys)
{
	AddBind(name, func, func, func, keys);
}

void KeyBinder::AddBind(const std::string& name, const OptFunc& func, const OnState_& keyState, Keys_t keys)
{
	AddBind(name, func, keyState, keys.keyboard);

	SetGPKeys(name, keys.controller);
}

void KeyBinder::OnKeyboardUpdate(const DIKeyList& kbState)
{
	if (!m_isIgnoring)
	{
		for (auto& binding : m_bindingsVec)
		{
			bool hasModifier = binding->HasModifier();
			bool modifierCheck{ true };

			if (hasModifier)
			{
				modifierCheck = binding->IsModifierSet(kbState);
			}/*
			else
			{
				modifierCheck = !IsAnyModifierSet(kbState);
			}*/

			switch (GetKeyboardKeyState(*binding, kbState, m_lastKBState))
			{
			case OnState_Press:
			{
				std::lock_guard _{ m_inputMutex };

				if (modifierCheck && binding->m_pressFunc) (*binding->m_pressFunc)();
			}
			break;

			case OnState_Hold:
			{
				std::lock_guard _{ m_inputMutex };

				if (modifierCheck && binding->m_holdFunc) (*binding->m_holdFunc)();
			}
			break;

			case OnState_Release:
			{
				std::lock_guard _{ m_inputMutex };

				if (modifierCheck && binding->m_releaseFunc) (*binding->m_releaseFunc)();
			}
			break;

			case OnState_Null:
			default:
				break;
			}
		}
	}

	m_lastKBState = kbState;
}

// Keyboard AddBind

void KeyBinder::AddBind(const std::string& name, const OptFunc& pressFunc, const OptFunc& holdFunc, const OptFunc&
                          releaseFunc, const KBKeyList& keys)
{
	const auto& search = m_bindings.find(name);

	if(search != m_bindings.end())
	{
		for (auto it = m_bindingsVec.begin(); it != m_bindingsVec.end();)
		{
			if (*it == search->second) 
			{
				it = m_bindingsVec.erase(it);
			}
			else 
			{
				++it;
			}
		}

		m_bindings.erase(name);
	}

	KBKeyList nonModKeys{};
	KBKeyList modKeys{};

	for (auto& key : keys)
	{
		switch (key)
		{
		case DIK_LSHIFT:
		case DIK_RSHIFT:
		case DIK_LCONTROL:
		case DIK_RCONTROL:
		case DIK_LALT:
		case DIK_RALT:
			modKeys.push_back(key);
			break;
		default:
			nonModKeys.push_back(key);
			break;
		}
	}

	if (modKeys.empty())
	{
		m_bindingsVec.emplace_back(std::make_shared<Binding>(name, pressFunc, holdFunc, releaseFunc, nonModKeys));
	}
	else
	{
		m_bindingsVec.emplace_back(std::make_shared<Binding>(name, pressFunc, holdFunc, releaseFunc, nonModKeys, modKeys));
	}

	m_bindings[name] = m_bindingsVec.back();

	UpdateBindCount();
}

void KeyBinder::AddBind(const std::string& name, const OptFunc& pressFunc, const OptFunc& holdFunc, const OptFunc&
                          releaseFunc, const uint8_t& key)
{
	AddBind(name, pressFunc, holdFunc, releaseFunc, KBKeyList{ key });
}

void KeyBinder::AddBind(const std::string& name, const OptFunc& func, const KBKeyList& keys)
{
	AddBind(name, func, func, func, keys);
}

void KeyBinder::AddBind(const std::string& name, const OptFunc& func, const uint8_t& key)
{
	AddBind(name, func, func, func, KBKeyList{ key });
}

void KeyBinder::AddBind(const std::string& name, const OptFunc& func, const OnState_& keyState, const KBKeyList& keys)
{
	const auto& search = m_bindings.find(name);

	if (search != m_bindings.end())
	{
		for (auto it = m_bindingsVec.begin(); it != m_bindingsVec.end();)
		{
			if (*it == search->second)
			{
				it = m_bindingsVec.erase(it);
			}
			else
			{
				++it;
			}
		}

		m_bindings.erase(name);
	}

	KBKeyList nonModKeys{};
	KBKeyList modKeys{};

	for (auto& key : keys)
	{
		switch (key)
		{
		case DIK_LSHIFT:
		case DIK_RSHIFT:
		case DIK_LCONTROL:
		case DIK_RCONTROL:
		case DIK_LALT:
		case DIK_RALT:
			modKeys.push_back(key);
			break;
		default:
			nonModKeys.push_back(key);
			break;
		}
	}

	if (modKeys.empty())
	{
		m_bindingsVec.emplace_back(std::make_shared<Binding>(name, func, keyState, nonModKeys));
	}
	else
	{
		m_bindingsVec.emplace_back(std::make_shared<Binding>(name, func, keyState, nonModKeys, modKeys));
	}

	m_bindings[name] = m_bindingsVec.back();

	UpdateBindCount();
}

void KeyBinder::AddBind(const std::string& name, const OptFunc& func, const OnState_& keyState, const uint8_t& key)
{
	AddBind(name, func, keyState, KBKeyList{ key });
}

void KeyBinder::OnGamePadUpdate(const HIDGamePadDevice& controllerState)
{
	if (!m_isIgnoring)
	{
		for (auto& binding : m_bindingsVec)
		{
			switch (GetControllerKeyState(*binding, controllerState.m_buttons & 0b11111111111111111, m_lastControllerState))
			{
			case OnState_Press:
			{
				std::lock_guard _{ m_inputMutex };

				if (binding->m_pressFunc) (*binding->m_pressFunc)();
			}
			break;

			case OnState_Hold:
			{
				std::lock_guard _{ m_inputMutex };

				if (binding->m_holdFunc) (*binding->m_holdFunc)();
			}
			break;

			case OnState_Release:
			{
				std::lock_guard _{ m_inputMutex };

				if (binding->m_releaseFunc) (*binding->m_releaseFunc)();
			}
			break;

			case OnState_Null:
			default:
				break;
			}
		}
	}

	m_lastControllerState = controllerState.m_buttons & 0b11111111111111111;
}

// Controller AddBind

void KeyBinder::AddBind(const std::string& name, const OptFunc& pressFunc, const OptFunc& holdFunc, const OptFunc& releaseFunc, const REGPK_Flag& controllerKeys)
{
	const auto& search = m_bindings.find(name);

	if (search != m_bindings.end())
	{
		for (auto it = m_bindingsVec.begin(); it != m_bindingsVec.end();)
		{
			if (*it == search->second)
			{
				it = m_bindingsVec.erase(it);
			}
			else
			{
				++it;
			}
		}

		m_bindings.erase(name);
	}

	m_bindingsVec.emplace_back(std::make_shared<Binding>(name, pressFunc, holdFunc, releaseFunc, KBKeyList{}, std::nullopt, controllerKeys));

	m_bindings[name] = m_bindingsVec.back();

	UpdateBindCount();
}

void KeyBinder::AddBind(const std::string& name, const OptFunc& pressFunc, const OptFunc& holdFunc, const OptFunc& releaseFunc, const std::vector<REGPK_Flag>& controllerKeys)
{
	REGPK_Flag keys = REGPK_None;
	for (const auto& key : controllerKeys)
	{
		keys |= key;
	}

	AddBind(name, pressFunc, holdFunc, releaseFunc, keys);
}

void KeyBinder::AddBind(const std::string& name, const OptFunc& func, const REGPK_Flag& controllerKeys)
{
	AddBind(name, func, func, func, controllerKeys);
}

void KeyBinder::AddBind(const std::string& name, const OptFunc& func, const std::vector<REGPK_Flag>& controllerKeys)
{
	REGPK_Flag keys = REGPK_None;
	for (const auto& key : controllerKeys)
	{
		keys |= key;
	}

	AddBind(name, func, func, func, keys);
}

void KeyBinder::AddBind(const std::string& name, const OptFunc& func, const OnState_& keyState, const REGPK_Flag& controllerKeys)
{
	const auto& search = m_bindings.find(name);

	if (search != m_bindings.end())
	{
		for (auto it = m_bindingsVec.begin(); it != m_bindingsVec.end();)
		{
			if (*it == search->second)
			{
				it = m_bindingsVec.erase(it);
			}
			else
			{
				++it;
			}
		}

		m_bindings.erase(name);
	}

	m_bindingsVec.emplace_back(std::make_shared<Binding>(name, func, keyState, KBKeyList{}, std::nullopt, controllerKeys));

	m_bindings[name] = m_bindingsVec.back();

	UpdateBindCount();
}

void KeyBinder::AddBind(const std::string& name, const OptFunc& func, const OnState_& keyState, const std::vector<REGPK_Flag>& controllerKeys)
{
	REGPK_Flag keys = REGPK_None;
	for(const auto& key : controllerKeys)
	{
		keys |= key;
	}

	AddBind(name, func, keyState, keys);
}

std::vector<std::string> KeyBinder::GetBindsOf(const REGPK_Flag& keys)
{
	std::vector<std::string> bindNames = {};

	for (const auto& bind : m_bindingsVec)
	{
		if (bind->DoKeysMatch(keys))
		{
			bindNames.push_back(bind->m_name);
		}
	}

	return bindNames;
}

std::vector<REGPK_Flag> KeyBinder::GetControllerKeys(const std::string& name)
{
	const auto& search = m_bindings.find(name);

	if (search == m_bindings.end())
		return {};

	std::vector<REGPK_Flag> ret = {};

	for(size_t i = 0; i < 32; i++)
	{
		if(search->second->m_controllerKeys & 1 << i)
		{
			ret.push_back(static_cast<REGPK_Flag>(1 << i));
		}
	}

	return ret;
}

std::vector<REGPK_Flag> KeyBinder::GetControllerKeys(const REGPK_Flag& keys)
{
	std::vector<REGPK_Flag> ret = {};

	// Bit 15: REGPK_CRight
	for (size_t i = 0; i < 15; i++)
	{
		if (keys & 1 << i)
		{
			ret.push_back(static_cast<REGPK_Flag>(1 << i));
		}
	}

	return ret;
}

std::vector<std::string> KeyBinder::GetXboxConterllerKeysStr(const std::string& name)
{
	auto keyList = GetControllerKeys(name);

	std::vector<std::string> ret = {};

	ret.reserve(keyList.size());

	for(const auto& key : keyList)
	{
		ret.emplace_back(XboxCtrlrKToS(key));
	}

	return ret;
}

std::vector<std::string> KeyBinder::GetXboxConterllerKeysStr(const std::vector<REGPK_Flag>& keys)
{
	std::vector<std::string> ret = {};

	ret.reserve(keys.size());

	for (const auto& key : keys)
	{
		ret.emplace_back(XboxCtrlrKToS(key));
	}

	return ret;
}

std::vector<std::string> KeyBinder::GetDSConterllerKeysStr(const std::string& name)
{
	auto keyList = GetControllerKeys(name);

	std::vector<std::string> ret = {};

	ret.reserve(keyList.size());

	for (const auto& key : keyList)
	{
		ret.emplace_back(DSCtrlrKToS(key));
	}

	return ret;
}

std::vector<std::string> KeyBinder::GetDSConterllerKeysStr(const std::vector<REGPK_Flag>& keys)
{
	std::vector<std::string> ret = {};

	ret.reserve(keys.size());

	for (const auto& key : keys)
	{
		ret.emplace_back(DSCtrlrKToS(key));
	}

	return ret;
}

void KeyBinder::SetGPKeys(const std::string& name, const REGPK_Flag& controllerKeys)
{
	const auto& search = m_bindings.find(name);

	if (search == m_bindings.end())
		return;

	search->second->m_controllerKeys = controllerKeys;
}

void KeyBinder::RemoveDuplicatesOf(const REGPK_Flag& keys, const std::string& keep)
{
	const auto& binds = GetBindsOf(keys);

	if (!binds.empty()) {
		for (const auto& bindName : binds) {
			if (bindName != keep) {
				SetGPKeys(bindName, REGPK_None);
			}
		}
	}
}

REGPK_Flag KeyBinder::GPKeyListArrayToBit(const std::vector<REGPK_Flag>& keys)
{
	REGPK_Flag ret = REGPK_None;

	for(const auto& keyFlag : keys)
	{
		ret |= keyFlag;
	}

	return ret;
}

void KeyBinder::RemoveBind(const std::string& name)
{
	m_bindings.erase(name);
	for (auto it = m_bindingsVec.begin(); it != m_bindingsVec.end(); ++it)
	{
		if ((*it)->m_name == name) m_bindingsVec.erase(it);
	}

	UpdateBindCount();
}

std::vector<std::string> KeyBinder::GetKBBindsOf(const KBKeyList& keys)
{
	std::vector<std::string> bindNames = {};

	for(const auto& bind : m_bindingsVec)
	{
		if(bind->DoKeysMatch(keys))
		{
			bindNames.push_back(bind->m_name);
		}
	}

	return bindNames;
}

bool KeyBinder::SaveAllBinds(bool updateFile)
{
	//std::lock_guard _{ m_inputMutex };

	for (const auto& bind : m_bindingsVec)
	{
		if (!bind->m_keyboardKeys.empty()) {
			std::vector<std::string> tmpBuffer;

			if (bind->m_modifierKeys && !bind->m_modifierKeys->empty())
			{
				tmpBuffer.reserve(bind->m_modifierKeys->size());
				for (auto& key : *bind->m_modifierKeys)
				{
					tmpBuffer.emplace_back(DIKToS(key));
				}
			}

			tmpBuffer.reserve(bind->m_keyboardKeys.size());
			for (auto& key : bind->m_keyboardKeys)
			{
				tmpBuffer.emplace_back(DIKToS(key));
			}

			m_config.set("KB: " + bind->m_name, tmpBuffer);
		}

		if(bind->m_controllerKeys != REGPK_None)
		{
			std::vector<std::string> tmpBuffer = GetXboxConterllerKeysStr(GetControllerKeys(bind->m_controllerKeys));

			m_config.set("GP: " + bind->m_name, tmpBuffer);
		}
	}

	if (updateFile)
	{
		return SaveConfig();
	}

	return true;
}

// Restores the last saved states inside the memory \
 Will restore the last saved states in the config file if refreshFromFile is true
bool KeyBinder::LoadAllBinds(bool refreshFromFile /*= false*/, bool clearIfNotFound /*= false*/)
{
	//std::lock_guard _{ m_inputMutex };

	if (refreshFromFile)
	{
		if (!LoadConfig()) return false;
	}

	for (auto it = m_bindings.begin(); it != m_bindings.end(); ++it)
	{
		auto& name = it->first;
		LoadBind(name, clearIfNotFound);
	}

	return true;
}

bool KeyBinder::SaveBind(const std::string& name, bool updateFile)
{
	//std::lock_guard _{ m_inputMutex };

	const auto& bind = m_bindings.find(name);

	if (bind == m_bindings.end())
		return false;

	if (!bind->second->m_keyboardKeys.empty()) {
		std::vector<std::string> tmpBuffer;

		if (bind->second->m_modifierKeys && !bind->second->m_modifierKeys->empty())
		{
			tmpBuffer.reserve(bind->second->m_modifierKeys->size());
			for (auto& key : *bind->second->m_modifierKeys)
			{
				tmpBuffer.emplace_back(DIKToS(key));
			}
		}

		for (auto& key : bind->second->m_keyboardKeys)
		{
			tmpBuffer.emplace_back(DIKToS(key));
		}

		m_config.set(bind->second->m_name, tmpBuffer);
	}

	if (bind->second->m_controllerKeys != REGPK_None) {
		std::vector<std::string> tmpBuffer = GetXboxConterllerKeysStr(GetControllerKeys(bind->second->m_controllerKeys));

		m_config.set("GP: " + bind->second->m_name, tmpBuffer);
	}

	if (updateFile)
	{
		return SaveConfig();
	}

	return true;
}

bool KeyBinder::LoadBind(const std::string& name, bool refreshFromFile, bool clearIfNotFound)
{
	//std::lock_guard _{ m_inputMutex };

	if (refreshFromFile)
	{
		if (!LoadConfig()) return false;
	}

	const auto& bind = m_bindings.find(name);

	if (bind == m_bindings.end())
		return false;

	const auto keyboardKeys = m_config.get<std::vector<std::string>>("KB: " + name);

	if(!keyboardKeys)
	{
		if (clearIfNotFound) {
			bind->second->m_keyboardKeys.clear();
			bind->second->m_modifierKeys = {};
		}
	}
	else {
		KBKeyList modifierTmpBuffer;
		KBKeyList keyTmpBuffer;

		for (auto& key : *keyboardKeys)
		{
			uint8_t diKey = SToDIK(key);
			switch (diKey)
			{
			case DIK_LSHIFT:
			case DIK_RSHIFT:
			case DIK_LCONTROL:
			case DIK_RCONTROL:
			case DIK_LALT:
			case DIK_RALT:
				modifierTmpBuffer.push_back(diKey);
				break;
			default:
				keyTmpBuffer.push_back(diKey);
				break;
			}
		}

		if (!modifierTmpBuffer.empty())
		{
			bind->second->m_modifierKeys = std::make_optional(modifierTmpBuffer);
		}

		bind->second->m_keyboardKeys = keyTmpBuffer;
	}

	const auto controllerKeys = m_config.get<std::vector<std::string>>("GP: " + name);

	if(!controllerKeys)
	{
		if (clearIfNotFound) {
			bind->second->m_controllerKeys = REGPK_None;
		}
	} else
	{
		REGPK_Flag keysTemp = REGPK_None;

		for(const auto& key : *controllerKeys)
		{
			keysTemp |= SToXboxCtrlrK(key);
		}

		bind->second->m_controllerKeys = keysTemp;
	}

	return true;
}

KeyBinder::KBKeyList KeyBinder::GetKeyboardKeys(const std::string& name, KMFlag_ keyModeFlag /*= KMFlag_None*/)
{
	const auto& search = m_bindings.find(name);

	if (search == m_bindings.end())
		return {};

	KBKeyList ret = {};

	switch (keyModeFlag)
	{
	case KMFlag_OnlyNonModifier:
		{
			ret = search->second->m_keyboardKeys;
		}
		break;

	case KMFlag_OnlyModifier:
		{
			if (search->second->m_modifierKeys)
			{
				ret = *search->second->m_modifierKeys;
			}
		}
		break;

	case KMFlag_None:
	default:
		{
			ret.insert(ret.begin(), search->second->m_keyboardKeys.begin(), search->second->m_keyboardKeys.end());

			if (search->second->m_modifierKeys)
			{
				ret.insert(ret.begin(), search->second->m_modifierKeys->begin(),
				           search->second->m_modifierKeys->end());
			}
		}
		break;
	}

	return ret;
}

std::vector<std::string> KeyBinder::GetKeyboardKeysStr(const std::string& name, KMFlag_ keyModeFlag /*= KMFlag_None*/)
{
	const auto& search = m_bindings.find(name);

	if (search == m_bindings.end() || search->second->m_keyboardKeys.empty() || (keyModeFlag == KMFlag_OnlyModifier && !search->
		second->m_modifierKeys))
		return {};

	std::vector<std::string> ret = {};

	bool getModKeys = false;
	bool getNonModKeys = false;

	getModKeys = keyModeFlag == KMFlag_None || keyModeFlag == KMFlag_OnlyModifier;
	getNonModKeys = keyModeFlag == KMFlag_None || keyModeFlag == KMFlag_OnlyNonModifier;

	if (getModKeys && search->second->m_modifierKeys)
	{
		ret.reserve(search->second->m_modifierKeys->size());
		for (auto& key : *search->second->m_modifierKeys)
		{
			ret.emplace_back(DIKToS(key));
		}
	}

	if (getNonModKeys)
	{
		ret.reserve(search->second->m_keyboardKeys.size());
		for (auto& key : search->second->m_keyboardKeys)
		{
			ret.emplace_back(DIKToS(key));
		}
	}

	return ret;
}

void KeyBinder::SetKBKeys(const std::string& name, const KBKeyList& newDIKeys)
{
	const auto& search = m_bindings.find(name);

	if (search == m_bindings.end())
		return;

	KBKeyList nonModKeys{};
	KBKeyList modKeys{};

	for (auto& key : newDIKeys)
	{
		switch (key)
		{
		case DIK_LSHIFT:
		case DIK_RSHIFT:
		case DIK_LCONTROL:
		case DIK_RCONTROL:
		case DIK_LALT:
		case DIK_RALT:
			modKeys.push_back(key);
			break;
		default:
			nonModKeys.push_back(key);
			break;
		}
	}

	if (nonModKeys.empty())
	{
		search->second->m_keyboardKeys.clear();
		search->second->m_modifierKeys->clear();
		return;
	}
	search->second->m_keyboardKeys = nonModKeys;

	if (modKeys.empty())
	{
		search->second->m_modifierKeys = {};
	}
	else
	{
		search->second->m_modifierKeys = std::make_optional(modKeys);
	}
}

void KeyBinder::SetName(const std::string& currentName, const std::string& newName)
{
	if (newName != currentName)
	{
		const auto& search_current = m_bindings.find(currentName);
		const auto& search_new = m_bindings.find(newName);

		if (search_current == m_bindings.end() || search_new == m_bindings.end())
			return;

		m_bindingsVec.emplace_back(search_current->second);
		m_bindings[newName] = m_bindingsVec.back();

		m_bindings.erase(currentName);
		for (auto it = m_bindingsVec.begin(); it != m_bindingsVec.end(); ++it)
		{
			if ((*it)->m_name == currentName) m_bindingsVec.erase(it);
		}
	}
}

std::optional<std::function<void()>> KeyBinder::GetFunc(const std::string& name, OnState_ keyState)
{
	const auto& search = m_bindings.find(name);

	if (search == m_bindings.end())
		return {};

	OptFunc ret = {};

	switch (keyState)
	{
	case OnState_Press:
		ret = search->second->m_pressFunc;
		break;

	case OnState_Hold:
		ret = search->second->m_holdFunc;
		break;

	case OnState_Release:
		ret = search->second->m_releaseFunc;
		break;

	case OnState_Null:
	default:
		ret = {};
		break;
	}

	return ret;
}

void KeyBinder::SetFunc(const std::string& name, const OptFunc& newFunc, OnState_ keyState)
{
	const auto& search = m_bindings.find(name);

	if (search == m_bindings.end())
		return;

	switch (keyState)
	{
	case OnState_Press:
		search->second->m_pressFunc = newFunc;
		break;

	case OnState_Hold:
		search->second->m_holdFunc = newFunc;
		break;

	case OnState_Release:
		search->second->m_releaseFunc = newFunc;
		break;

	case OnState_Null:
	default:
		break;
	}
}

std::vector<std::string> KeyBinder::GetDuplicates()
{
	std::vector<std::string> dups = {};

	for (auto it = m_bindingsVec.begin(); it != m_bindingsVec.end(); ++it)
	{
		auto& temp = (*it)->m_name;
		for (const auto& binding : m_bindingsVec)
			if (binding->m_name == temp)
				dups.push_back(temp);
	}

	return dups;
}

void KeyBinder::RemoveDuplicatesOf(const KBKeyList& keys, const std::string& keep)
{
	const auto& binds = GetKBBindsOf(keys);

	if (!binds.empty()) {
		for (const auto& bindName : binds) {
			if (bindName != keep) {
				SetKBKeys(bindName, KBKeyList{});
			}
		}
	}
}

void KeyBinder::UpdateBindCount()
{
	m_bindCount = m_bindings.size();
}

OnState_ KeyBinder::GetKeyboardKeyState(const Binding& binding, const DIKeyList& keys, const DIKeyList& lasKeys)
{
	OnState_ ret = OnState_Null;

	bool isDown = binding.DoKeysMatch(keys);
	bool wasDown = binding.DoKeysMatch(lasKeys);

	if (isDown && !wasDown) ret = OnState_Press;
	if (isDown && wasDown) ret = OnState_Hold;
	if (!isDown && wasDown) ret = OnState_Release;

	return ret;
}

bool KeyBinder::IsAnyModifierSet(const DIKeyList& keys)
{
	bool res = keys[DIK_LSHIFT] || keys[DIK_RSHIFT]
		|| keys[DIK_LCONTROL] || keys[DIK_RCONTROL]
		|| keys[DIK_LALT] || keys[DIK_RALT];
	return res;
}

OnState_ KeyBinder::GetControllerKeyState(const Binding& binding, const REGPK_Flag& controllerKeys, const REGPK_Flag& lastControllerKeys)
{
	OnState_ ret = OnState_Null;

	bool isDown = binding.DoKeysMatch(controllerKeys);
	bool wasDown = binding.DoKeysMatch(lastControllerKeys);

	if (isDown && !wasDown) ret = OnState_Press;
	if (isDown && wasDown) ret = OnState_Hold;
	if (!isDown && wasDown) ret = OnState_Release;

	return ret;
}

KeyBinder::Binding::Binding(Binding&& other)
	: m_keyboardKeys(std::move(other.m_keyboardKeys)), m_modifierKeys(std::move(other.m_modifierKeys)),
	m_pressFunc(std::move(other.m_pressFunc)), m_holdFunc(std::move(other.m_holdFunc)),
	m_releaseFunc(std::move(other.m_releaseFunc))
{}

KeyBinder::Binding::Binding(const std::string& name, const OptFunc& pressFunc, const OptFunc& holdFunc,
                              const OptFunc& releaseFunc, const
                              KBKeyList& keys, const std::optional<KBKeyList>& modifierKeys, const REGPK_Flag& controllerKeys)
	: m_name(name), m_keyboardKeys(keys), m_modifierKeys(modifierKeys), m_controllerKeys(controllerKeys), m_pressFunc(pressFunc), m_holdFunc(holdFunc),
	  m_releaseFunc(releaseFunc)
{}

KeyBinder::Binding::Binding(const std::string& name, const OptFunc& func, const OnState_& keyState,
                              const KBKeyList& keys, const std::optional<KBKeyList>& modifierKeys, const REGPK_Flag& controllerKeys)
	: m_name(name), m_keyboardKeys(keys), m_modifierKeys(modifierKeys), m_controllerKeys(controllerKeys)
{
	switch (keyState)
	{
	case OnState_Press:
		m_pressFunc = func;
		break;

	case OnState_Hold:
		m_holdFunc = func;
		break;

	case OnState_Release:
		m_releaseFunc = func;
		break;

	case OnState_Null:
	default:
		break;
	}
}

// Keyboard

bool KeyBinder::Binding::HasModifier() const
{
	return m_modifierKeys.has_value();
}

bool KeyBinder::Binding::DoKeysMatch(const DIKeyList& kbState) const
{
	bool res = true;

	!m_keyboardKeys.empty() ? res = true : res = false;

	for (auto& key : m_keyboardKeys)
	{
		res = res && kbState[key];
	}

	return res;
}

bool KeyBinder::Binding::DoKeysMatch(const KBKeyList& keyList) const
{
	DIKeyList diKeyList = { 0 };

	for (auto& key : keyList)
	{
		diKeyList[key] = true;
	}

	return DoKeysMatch(diKeyList);
}

bool KeyBinder::Binding::IsModifierSet(const DIKeyList& kbState) const
{
	bool res = true;

	!m_modifierKeys->empty() ? res = true : res = false;

	for (auto& modKey : *m_modifierKeys)
	{
		res = res && kbState[modKey];
	}

	return res;
}

// Controller

bool KeyBinder::Binding::DoKeysMatch(const REGPK_Flag& controllerState) const
{
	bool res = false;

	if(m_controllerKeys != REGPK_None && (m_controllerKeys & controllerState) == m_controllerKeys)
	{
		res = true;
	}

	return res;
}

KeyBinder::Binding& KeyBinder::Binding::operator=(Binding&& other)
{
	this->m_keyboardKeys = std::move(other.m_keyboardKeys);
	this->m_modifierKeys = std::move(other.m_modifierKeys);
	this->m_pressFunc = std::move(other.m_pressFunc);
	this->m_holdFunc = std::move(other.m_holdFunc);
	this->m_releaseFunc = std::move(other.m_releaseFunc);

	return *this;
}
