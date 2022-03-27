#pragma once

#include <sdk/ReClass.hpp>

namespace dmc5 {
class HIDNativeDeviceBase : public REManagedObject {
public:
    char pad_0010[272]; // 0x0010
};                      // Size: 0x0120
static_assert(sizeof(HIDNativeDeviceBase) == 0x120);

class HIDGamePadDevice : public HIDNativeDeviceBase {
public:
    uint32_t m_buttons;     // 0x120
    uint32_t m_buttonsDown; // 0x124
    uint32_t m_buttonsUp;    // 0x128
    char padd_12C[68];      // 0x12C
    float m_triggerL;       // 0x170
    float m_triggerR;       // 0x174
    char padd_178[296];     // 0x178
};                      // Size: 0x02A0
static_assert(sizeof(HIDGamePadDevice) == 0x2A0);
}

class REReference {
public:
    uint32_t classHash; // 0x00
    uint16_t noIdea1;   // 0x04
    uint16_t noIdea2;   // 0x06
    uint64_t noIdea3;   // 0x08
}; // Size: 0x10
static_assert(sizeof(REReference) == 0x10);

class SceneManager
{
public:
    char pad_0000[0x48];                // 0x00
    class SceneView* main_scene_view; // 0x48
}; // Size: 0x50
static_assert(sizeof(SceneManager) == 0x50);

class SceneView : public REGameObject
{
public:
    char pad_0060[0x18];// 0x60
}; // Size = 0x78
static_assert(sizeof(SceneView) == 0x78);

class UI_String_t {
public:
    UI_String_t(const wchar_t* english = L"", const wchar_t* italian = L"", const wchar_t* polish = L"") 
        : text1{L"AdditionalMenu_costume"}, text2 {L"AdditonalMenu_PauseKey"},
        m_English{(wchar_t*)english}, m_Italian{(wchar_t*)italian}, m_Polish{(wchar_t*)polish}
    {
        if (italian == L"") {
            m_Italian = m_Polish;
        }

        if (polish == L"") {
            m_Polish = m_English;
        }
    }

    REReference reference;	   // 0x0000
	wchar_t		*text1;		   // 0x0010
	wchar_t		*text2;		   // 0x0018
	wchar_t		*N00000055;	   // 0x0020
	wchar_t		*N00000056;	   // 0x0028
	wchar_t		*m_English;	   // 0x0030
	wchar_t		*m_French;	   // 0x0038
	wchar_t		*m_Italian;	   // 0x0040
	wchar_t		*m_German;	   // 0x0048
	wchar_t		*m_Spanish;	   // 0x0050
	void		*N0000005C;	   // 0x0058
	wchar_t		*m_Polish;	   // 0x0060
	wchar_t		*N0000005E;	   // 0x0068
	wchar_t		*N000000E0;	   // 0x0070
	wchar_t		*m_Portuguese; // 0x0078
	wchar_t		*m_Russian;	   // 0x0080
	wchar_t		*m_Japanese;   // 0x0088
	wchar_t		*N000000CE;	   // 0x0090
	wchar_t		*N000000CF;	   // 0x0098
	wchar_t		*N000000D0;	   // 0x00A0
	wchar_t		*N000000D1;	   // 0x00A8
	wchar_t		*N000000D2;	   // 0x00B0
	wchar_t		*N000000D3;	   // 0x00B8
	wchar_t		*N000000D4;	   // 0x00C0
	wchar_t		*N000000D5;	   // 0x00C8
	wchar_t		*N000000D6;	   // 0x00D0
	wchar_t		*N000000D7;	   // 0x00D8
	wchar_t		*N000000D8;	   // 0x00E0
	wchar_t		*N000000D9;	   // 0x00E8
	wchar_t		*N000000FC;	   // 0x00F0
	wchar_t		*N000000DB;	   // 0x00F8
	wchar_t		*N000000DC;	   // 0x0100
};							   // Size: 0x0108
static_assert(sizeof(UI_String_t) == 0x0108);