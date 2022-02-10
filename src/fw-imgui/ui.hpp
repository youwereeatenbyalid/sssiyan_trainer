#pragma once
// clang-format off
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi")

#include <d3d11.h>
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>

#include <imgui/imgui.h>

#include "KeyBinder.hpp"

#define OUTLINE_GRAY		ImVec4(0.29f, 0.29f, 0.29f, 1.00f)
#define OUTLINE_NORM		ImVec4(0.50f, 0.93f, 0.93f, 1.00f)
#define ELEMENT_BG_DARK		ImVec4(0.07f, 0.07f, 0.07f, 1.00f)
#define ELEMENT_BC_ACTV		ImVec4(0.07f, 0.22f, 0.29f, 1.00f)
#define ELEMENT_TEXT_DCT	ImVec4(0.09f, 0.60f, 0.64f, 1.00f)
#define ELEMENT_TEXT_ACT	ImVec4(0.49f, 0.91f, 0.91f, 1.00f)
#define ELEMENT_STYLE_ACT	ImVec4(0.09f, 0.25f, 0.33f, 1.00f)
#define ELEMENT_STYLE_DCT	ImVec4(0.07f, 0.07f, 0.07f, 1.00f)
#define ELEMENT_LABEL_ACT	ImVec4(0.95f, 0.95f, 0.95f, 1.00f)
#define ELEMENT_LABEL_DCT	ImVec4(0.20f, 0.20f, 0.20f, 1.00f)
#define BUTTON_BG_DARK		ImVec4(0.07f, 0.07f, 0.07f, 1.00f)
#define BUTTON_BC_ACTV		ImVec4(0.07f, 0.22f, 0.29f, 1.00f)
#define BUTTON_TEXT_DCT		ImVec4(0.09f, 0.60f, 0.64f, 1.00f)
#define BUTTON_TEXT_ACT		ImVec4(0.49f, 0.91f, 0.91f, 1.00f)

	// Indices of the buttons in the DMC5 controlls button texure
enum BtnIndex_
{
	// XBox controller
	BtnIndex_Xbox_A = 0,
	BtnIndex_Xbox_B = 1,
	BtnIndex_Xbox_X = 2,
	BtnIndex_Xbox_Y = 3,
	BtnIndex_Xbox_DpadNoInput = 4,
	BtnIndex_Xbox_EmptySpaceAfterDpad = 5,
	BtnIndex_Xbox_DPadUp = 6,
	BtnIndex_Xbox_DPadDown = 7,
	BtnIndex_Xbox_DPadLeft = 8,
	BtnIndex_Xbox_DPadRight = 9,
	BtnIndex_Xbox_DpadLeftAndRight = 10,
	BtnIndex_Xbox_DpadUpAndDown = 11,
	BtnIndex_Xbox_LeftStickNoInput = 12,
	BtnIndex_Xbox_RightStickNoInput = 13,
	BtnIndex_Xbox_LBumper = 14,
	BtnIndex_Xbox_RBumper = 15,
	BtnIndex_Xbox_Start = 16,
	BtnIndex_Xbox_Select = 17,
	BtnIndex_Xbox_LeftStickClick = 18,
	BtnIndex_Xbox_RightStickClick = 19,
	BtnIndex_Xbox_LTrigger = 20,
	BtnIndex_Xbox_RTrigger = 21,
	BtnIndex_Xbox_EmptySpaceAfterTriggers = 22,
	BtnIndex_Xbox_DPad = 23,
	BtnIndex_Xbox_LeftStickLeftAndRight = 24,
	BtnIndex_Xbox_LeftStickUpAndDown = 25,
	BtnIndex_Xbox_LStickDown = 26,
	BtnIndex_Xbox_RStickDown = 27,
	BtnIndex_Xbox_LStickUp = 28,
	BtnIndex_Xbox_RStickUp = 29,
	BtnIndex_Xbox_RightStickLeftAndRight = 30,
	BtnIndex_Xbox_RightStickUpAndDown = 31,
	BtnIndex_Xbox_LStickLeft = 32,
	BtnIndex_Xbox_LStickRight = 33,
	BtnIndex_Xbox_RStickLeft = 34,
	BtnIndex_Xbox_RStickRight = 35,

	// Keyboard & Mouse
	BtnIndex_Keyboard_F1 = 36,
	BtnIndex_Keyboard_F2 = 37,
	BtnIndex_Keyboard_F3 = 38,
	BtnIndex_Keyboard_F4 = 39,
	BtnIndex_Keyboard_F5 = 40,
	BtnIndex_Keyboard_F6 = 41,
	BtnIndex_Keyboard_F7 = 42,
	BtnIndex_Keyboard_F8 = 43,
	BtnIndex_Keyboard_F9 = 44,
	BtnIndex_Keyboard_F10 = 45,
	BtnIndex_Keyboard_F11 = 46,
	BtnIndex_Keyboard_F12 = 47,
	BtnIndex_Keyboard_BlankSpaceAfterFKeys1 = 48,
	BtnIndex_Keyboard_BlankSpaceAfterFKeys2 = 49,
	BtnIndex_Keyboard_BlankSpaceAfterFKeys3 = 50,
	BtnIndex_Keyboard_BlankSpaceAfterFKeys4 = 51,
	BtnIndex_Keyboard_BlankSpaceAfterFKeys5 = 52,
	BtnIndex_Mouse_NoInput = 53,
	BtnIndex_Mouse_ScrollWheelUp = 54,
	BtnIndex_Mouse_ScrollWheelDown = 55,
	BtnIndex_Mouse_ScrollWheelClick = 56,
	BtnIndex_Mouse_LeftClick = 57,
	BtnIndex_Mouse_RightClick = 58,
	BtnIndex_Keyboard_Caret = 59,
	BtnIndex_Keyboard_A = 60,
	BtnIndex_Keyboard_B = 61,
	BtnIndex_Keyboard_C = 62,
	BtnIndex_Keyboard_D = 63,
	BtnIndex_Keyboard_E = 64,
	BtnIndex_Keyboard_F = 65,
	BtnIndex_Keyboard_G = 66,
	BtnIndex_Keyboard_H = 67,
	BtnIndex_Keyboard_I = 68,
	BtnIndex_Keyboard_J = 69,
	BtnIndex_Keyboard_K = 70,
	BtnIndex_Keyboard_L = 71,
	BtnIndex_Keyboard_M = 72,
	BtnIndex_Keyboard_N = 73,
	BtnIndex_Keyboard_O = 74,
	BtnIndex_Keyboard_P = 75,
	BtnIndex_Keyboard_Q = 76,
	BtnIndex_Keyboard_R = 77,
	BtnIndex_Keyboard_S = 78,
	BtnIndex_Keyboard_T = 79,
	BtnIndex_Keyboard_U = 80,
	BtnIndex_Keyboard_V = 81,
	BtnIndex_Keyboard_W = 82,
	BtnIndex_Keyboard_X = 83,
	BtnIndex_Keyboard_Y = 84,
	BtnIndex_Keyboard_Z = 85,
	BtnIndex_Keyboard_At = 86,
	BtnIndex_Keyboard_ESC = 87,
	BtnIndex_Keyboard_Tab = 88,
	BtnIndex_Keyboard_Space = 89,
	BtnIndex_Keyboard_Backspace = 90,
	BtnIndex_Keyboard_End = 91,
	BtnIndex_Keyboard_Home = 92,
	BtnIndex_Keyboard_Colon = 93,
	BtnIndex_Keyboard_Semicolon = 94,
	BtnIndex_Keyboard_Comma = 95,
	BtnIndex_Keyboard_ArrowUp = 96,
	BtnIndex_Keyboard_ArrowLeft = 97,
	BtnIndex_Keyboard_AsteriskActive = 98,
	BtnIndex_Keyboard_ShiftLeft = 99,
	BtnIndex_Keyboard_CtrlLeft = 100,
	BtnIndex_Keyboard_AltLeft = 101,
	BtnIndex_Keyboard_Return = 102,
	BtnIndex_Keyboard_SlashForward = 103,
	BtnIndex_Keyboard_Underscore = 104,
	BtnIndex_Keyboard_Period = 105,
	BtnIndex_Keyboard_Insert = 106,
	BtnIndex_Keyboard_Delete = 107,
	BtnIndex_Keyboard_ArrowDown = 108,
	BtnIndex_Keyboard_ArrowRight = 109,
	BtnIndex_Keyboard_PlusActive = 110,
	BtnIndex_Keyboard_ShiftRight = 111,
	BtnIndex_Keyboard_CtrlRight = 112,
	BtnIndex_Keyboard_AltRight = 113,
	BtnIndex_Keyboard_ReturnActive = 114,
	BtnIndex_Keyboard_SlashForwardActive = 115,
	BtnIndex_Keyboard_UnderscoreActive = 116,
	BtnIndex_Keyboard_PeriodActive = 117,
	BtnIndex_Keyboard_PageUp = 118,
	BtnIndex_Keyboard_PageDown = 119,
	BtnIndex_Keyboard_0 = 120,
	BtnIndex_Keyboard_1 = 121,
	BtnIndex_Keyboard_2 = 122,
	BtnIndex_Keyboard_3 = 123,
	BtnIndex_Keyboard_4 = 124,
	BtnIndex_Keyboard_5 = 125,
	BtnIndex_Keyboard_6 = 126,
	BtnIndex_Keyboard_7 = 127,
	BtnIndex_Keyboard_8 = 128,
	BtnIndex_Keyboard_9 = 129,
	BtnIndex_Keyboard_SquareBracketOpen = 130,
	BtnIndex_Keyboard_SquareBracketClose = 131,
	BtnIndex_Keyboard_0Active = 132,
	BtnIndex_Keyboard_1Active = 133,
	BtnIndex_Keyboard_2Active = 134,
	BtnIndex_Keyboard_3Active = 135,
	BtnIndex_Keyboard_4Active = 136,
	BtnIndex_Keyboard_5Active = 137,
	BtnIndex_Keyboard_6Active = 138,
	BtnIndex_Keyboard_7Active = 139,
	BtnIndex_Keyboard_8Active = 140,
	BtnIndex_Keyboard_9Active = 141,
	BtnIndex_Keyboard_SlashBackYen = 142,
	BtnIndex_Keyboard_SlashBackUnderscore = 143,
	BtnIndex_Keyboard_WASD = 144,
	BtnIndex_Keyboard_ArrowUpAndDown = 145,
	BtnIndex_Mouse_Movement = 146,
	BtnIndex_Mouse_ScrollWheelUpAndDown = 147,
	BtnIndex_Mouse_Button3 = 148,
	BtnIndex_Mouse_Button1 = 149,
	BtnIndex_Mouse_Button2 = 150,
	BtnIndex_Mouse_Button4 = 151,
	BtnIndex_Mouse_Button5 = 152,
	BtnIndex_Keyboard_WASDBlank = 153,
	BtnIndex_Mouse_MovementAndLeftClick = 154,
	BtnIndex_Mouse_MovementAndRightClick = 155,
	BtnIndex_Mouse_MovementAndMiddleClick = 156,
	BtnIndex_Keyboard_BlankKey = 157,
	BtnIndex_Keyboard_ArrowLeftAndRight = 158,

	// None
	BtnIndex_None = 159
};

namespace UI {
	namespace WRL = Microsoft::WRL;

	const ImVec4 BUTTONCOLOR = { 0.501f, 0.929f, 0.929f, 1.0f };

	unsigned int stb_decompress_length(const unsigned char *input);
	unsigned int stb_decompress(unsigned char *output, const unsigned char *i, unsigned int length);
	int stb_resize(const unsigned char *input_pixels , int input_w , int input_h , int input_stride_in_bytes,
                   unsigned char *output_pixels, int output_w, int output_h, int output_stride_in_bytes,
                   int num_channels);

	void RenderRoundTopFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding);
	bool TopRoundButton(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags);

	bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", float thickness = 1.0f, ImGuiSliderFlags flags = 0);
	bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", float thickness = 1.0f, ImGuiSliderFlags flags = 0);

	BtnIndex_ REGPKeyToBtnIndex(const REGPK_Flag& key);

	void KeyCaptureWindow(struct KCWBuffers& kcwBuffers, bool closeBtn = false);
	// fitHeight: Matches the last item's height with keeping the size ratio the same (Will override scale)
	void KeyBindButton(const std::string& windowTitle, const std::string& bindName, KCWBuffers& kcwBuffers, const float& scale = 1.0f, bool fitHeight = false, ImVec4 tint_col = {1.0f, 1.0f, 1.0f, 1.0f}, bool showBtnsOnHover = true);

	bool DMC5LayoutStyleButton(std::string label, char side = 'M', const float& scale = 1.0f, ImVec2 size = ImVec2(0.0f, 0.0f));
	bool DMC5ControlsButton(BtnIndex_ index, const float& scale = 1.0f);
	
	// Copied straight from CE
	template<typename T>
	bool Combo(const char* label, const char** items, T count, T& var, T pos, bool save);

	bool TabBtn(const char* text, bool state, ImVec2 size_arg = ImVec2(0.0f, 0.0f), float rounding = 0.0f);

	enum class KeyMode_t : uint8_t
	{
		Keyboard = 0,
		Controller = 1
	};

	// Key capture window vars
	struct KCWBuffers {
		bool drawWindow{ false };
		std::string windowTitle{};
		std::string bindName{};
		// Buffers
		bool needConfirmBuffer{ false };
		int pressedCountBuffer = 1;
		std::vector<uint8_t> kbPressedBufferList;
		REGPK_Flag gpPressedBuffer;
		std::vector <REGPK_Flag> gpPressedBufferList;
		ImVec2 windowSizeBuffer{ 0.0f, 0.0f };
		ImVec2 windowSizeAddBuffer{ 0.0f, 0.0f };
		KeyMode_t keyMode;
	};

	template <typename T>
	struct Vec2
	{
		Vec2(T _x, T _y)
			: x(_x), y(_y)
		{}

		Vec2(const Vec2& other)
			: x(other.x), y(other.y)
		{}

		Vec2(const Vec2&& other)
			: x(std::move(other.x)), y(std::move(other.y))
		{}

		T x;
		T y;

		// ***
		// Depends on ImGui

		Vec2(const ImVec2& imVec)
			: x(imVec.x), y(imVec.y)
		{}

		// ***

		operator ImVec2() { return ImVec2(static_cast<float>(x), static_cast<float>(y)); }
		operator ImVec2() const { return ImVec2(static_cast<float>(x), static_cast<float>(y)); }

		Vec2 operator*(const T rhs) { return Vec2(x * rhs, y * rhs); }
		Vec2 operator/(const T rhs) { return Vec2(x / rhs, y / rhs); }
		Vec2 operator+(const Vec2& rhs) { return Vec2(x + rhs.x, y + rhs.y); }
		Vec2 operator-(const Vec2& rhs) { return Vec2(x - rhs.x, y - rhs.y); }
		Vec2 operator*(const Vec2& rhs) { return Vec2(x * rhs.x, y * rhs.y); }
		Vec2 operator/(const Vec2& rhs) { return Vec2(x / rhs.x, y / rhs.y); }
		Vec2& operator*=(const T rhs) { return { x *= rhs, y *= rhs }; }
		Vec2& operator/=(const T rhs) { return { x /= rhs, y /= rhs }; }
		Vec2& operator+=(const Vec2& rhs) { return { x += rhs.x, y += rhs.y }; }
		Vec2& operator-=(const Vec2& rhs) { return { x -= rhs.x, y -= rhs.y }; }
		Vec2& operator*=(const Vec2& rhs) { return { x *= rhs.x, y *= rhs.y }; }
		Vec2& operator/=(const Vec2& rhs) { return { x /= rhs.x, y /= rhs.y }; }
	};

	class Texture2DDX11 {
	public:
		Texture2DDX11() = default;

		Texture2DDX11(const char* filename, ID3D11Device* pd3dDevice);
		Texture2DDX11(const unsigned char* image_data, size_t image_size, ID3D11Device* pd3dDevice);
		Texture2DDX11(unsigned char* image_data, int width, int height, ID3D11Device* pd3dDevice);

		[[nodiscard]] unsigned char* GetImageData() const { return m_image_data; }
		[[nodiscard]] ID3D11ShaderResourceView* GetTexture() const { return m_srv.Get(); }
		[[nodiscard]] int GetWidth() const { return m_width; }
		[[nodiscard]] int GetHeight() const { return m_height; }
		template<typename T = float>
		[[nodiscard]] Vec2<T> GetSize(const T scale = 1.0f) const { return Vec2<T>(m_width, m_height) * scale; }
		template<typename T = float>
		[[nodiscard]] Vec2<T> GetSizeByWRatio(const T nWidth) const { return Vec2<T>(m_width, m_height) * (nWidth / m_width); }
		template<typename T = float>
		[[nodiscard]] Vec2<T> GetSizeByHRatio(const T nHeight) const { return Vec2<T>(m_width, m_height) * (nHeight / m_height); }
		[[nodiscard]] bool IsValid() const { return m_is_loaded; }

		inline operator void*() { return GetTexture(); }
		inline operator void*() const { return GetTexture(); }
		inline operator ID3D11ShaderResourceView*() { return GetTexture(); }
		inline operator ID3D11ShaderResourceView*() const { return GetTexture(); }
		inline operator bool() { return IsValid(); }
		inline operator bool() const { return IsValid(); }

	private:
		bool Commit();

	private:
		unsigned char* m_image_data{ nullptr };
		
		WRL::ComPtr<ID3D11Device> m_pd3dDevice{};
		WRL::ComPtr<ID3D11ShaderResourceView> m_srv{};
		int m_width{ 0 };
		int m_height{ 0 };

		bool m_is_loaded{ false };
	};

	class Texture2DDX12 {
	public:
		Texture2DDX12() = default;
		Texture2DDX12(const char* filename, ID3D12Device* pd3dDevice, ID3D12DescriptorHeap* pd3dSrvDescHeap, UINT descIndexInHeap);
		Texture2DDX12(const unsigned char* image_data, size_t image_size, ID3D12Device* pd3dDevice, ID3D12DescriptorHeap* pd3dSrvDescHeap, UINT descIndexInHeap);
		Texture2DDX12(unsigned char* image_data, int width, int height, ID3D12Device* pd3dDevice, ID3D12DescriptorHeap* pd3dSrvDescHeap, UINT descIndexInHeap);

		[[nodiscard]] UINT64 GetTexture() const { return m_texture_srv_gpu_handle.ptr; }
		[[nodiscard]] int GetWidth() const { return m_width; }
		[[nodiscard]] int GetHeight() const { return m_height; }
		template<typename T = float>
		[[nodiscard]] Vec2<T> GetSize(const T scale = 1.0f) const { return Vec2<T>(m_width, m_height) * scale; }
		template<typename T = float>
		[[nodiscard]] Vec2<T> GetSizeByWRatio(const T nWidth) const { return Vec2<T>(m_width, m_height) * (nWidth / m_width); }
		template<typename T = float>
		[[nodiscard]] Vec2<T> GetSizeByHRatio(const T nHeight) const { return Vec2<T>(m_width, m_height) * (nHeight / m_height); }
		[[nodiscard]] bool IsValid() const { return m_is_loaded; }

		inline operator void*() { return (void*)GetTexture(); }
		inline operator void*() const { return (void*)GetTexture(); }
		inline operator UINT64() { return GetTexture(); }
		inline operator UINT64() const { return GetTexture(); }
		inline operator bool() { return IsValid(); }
		inline operator bool() const { return IsValid(); }

	private:
		bool Commit();

	private:
		unsigned char* m_image_data{ nullptr };

		WRL::ComPtr<ID3D12Resource> m_tex_resource{};
		WRL::ComPtr<ID3D12Device> m_pd3dDevice{};
		WRL::ComPtr<ID3D12DescriptorHeap> m_pd3dSrvDescHeap{};

		UINT m_handle_increment;
		D3D12_CPU_DESCRIPTOR_HANDLE m_texture_srv_cpu_handle{};
		D3D12_GPU_DESCRIPTOR_HANDLE m_texture_srv_gpu_handle{};
		
		int m_width{ 0 };
		int m_height{ 0 };

		bool m_is_loaded{ false };
	};
}